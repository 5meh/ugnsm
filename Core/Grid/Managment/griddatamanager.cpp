#include "griddatamanager.h"

#include "../Utilities/Logger/logger.h"
#include "../TaskSystem/taskscheduler.h"
#include "../../Network/Information/networkinfo.h"
#include "../../Network/Information/networkinfomodel.h"
#include "../Utilities/Parser/iparser.h"
#include "../NetworkSortingStrategies/inetworksortstrategy.h"
#include "../Utilities/Parser/networkethernetparser.h"

#include "../NetworkSortingStrategies/speedsortstrategy.h"
#include "../Monitoring/networkmonitor.h"
#include "../../componentregistry.h"
#include "../TaskSystem/taskscheduler.h"

#include <QTimer>

GridDataManager::GridDataManager(TaskScheduler* scheduler, QObject* parent)
    : m_scheduler(scheduler),
    m_monitor{new NetworkMonitor{nullptr, this}},//TODO: mb use "old" syntaxis
    m_sorter{ComponentRegistry::create<INetworkSortStrategy>()},
    m_parser{ComponentRegistry::create<IParser>(nullptr)},
    QObject{parent}
{
    connect(m_parser.get(), &IParser::parsingCompleted,
            this, &GridDataManager::handleParsingCompleted, Qt::QueuedConnection);
    connect(m_monitor, &NetworkMonitor::statsUpdated,
            this, &GridDataManager::handleNetworkStats);

    m_scheduler->scheduleRepeating("data_refresh", 2000, this,
                                   &GridDataManager::refreshData,
                                   QThread::NormalPriority);

    Logger::instance().log(Logger::Info, "GridDataManager initialized", "Grid");
}

GridDataManager::~GridDataManager()
{
    clearGrid();
}

NetworkInfoModel* GridDataManager::cellData(QPoint indx) const
{
    if (indx.x() >= 0 && indx.x() < m_data.size() &&
        indx.y() >= 0 && indx.y() < m_data[indx.x()].size())
    {
        return m_data[indx.x()][indx.y()];
    }
    return nullptr;
}

int GridDataManager::getRows() const
{
    return m_data.size();
}

int GridDataManager::getCols() const
{
    return m_data.isEmpty() ? 0 : m_data[0].size();
}

void GridDataManager::initializeGrid(int rows, int cols)
{
    clearGrid();
    m_data.resize(rows);
    for(auto& row : m_data)
    {
        row.resize(cols);
    }

    refreshData();
    emit gridDimensionsChanged();
}

void GridDataManager::swapCells(const QPoint& from, const QPoint& to)
{
    m_scheduler->schedule(QString("grid_swap"),
                          this,
                          &GridDataManager::swapCellsImpl,
                          QThread::HighPriority,
                          from,
                          to);
}

void GridDataManager::handleParsingCompleted(const QVariant& result)
{
    Q_ASSERT(result.canConvert<QList<NetworkInfo*>>());
    QVariant resultCopy = result;
    m_scheduler->scheduleAtomic(m_refreshInProgress,
                                QString("data_processing"),
                                this,
                                &GridDataManager::handleParsingCompletedImpl,
                                QThread::NormalPriority,
                                std::move(resultCopy));
}

void GridDataManager::handleNetworkStats(const QString& mac, const quint64& rxSpeed, const quint64& txSpeed)
{
    m_scheduler->schedule(QString("stats_update"),
                          this,
                          &GridDataManager::handleNetworkStatsImpl,
                          QThread::LowPriority,
                          std::forward<const QString&>(mac),
                          rxSpeed,
                          txSpeed);
}

void GridDataManager::refreshData()
{
    m_parser->parse();
}

void GridDataManager::swapCellsImpl(const QPoint& from, const QPoint& to)
{
    if(from.x() < 0 || from.x() >= getRows() || from.y() < 0 || from.y() >= getCols() ||
        to.x() < 0 || to.x() >= getRows() || to.y() < 0 || to.y() >= getCols())
    {
        Logger::instance().log(Logger::Warning, "Invalid swap coordinates", "Grid");
        return;
    }

    std::swap(m_data[from.x()][from.y()], m_data[to.x()][to.y()]);
    updateMacMap();

    emit cellChanged(from);
    emit cellChanged(to);
}

void GridDataManager::handleParsingCompletedImpl(QVariant result)
{
    QList<NetworkInfo*> allInfos = result.value<QList<NetworkInfo*>>();
    m_sorter->sort(allInfos);

    const int rows = getRows();
    const int cols = getCols();
    const int capacity = rows * cols;

    QList<NetworkInfo*> usedInfos = allInfos.mid(0, capacity);
    QList<NetworkInfo*> unusedInfos = allInfos.mid(capacity);

    qDeleteAll(unusedInfos);
    unusedInfos.clear();

    QHash<QString, QPoint> oldIndex = m_macIndex;
    m_macIndex.clear();

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            const int linearIndex = r * cols + c;
            NetworkInfo* info = (linearIndex < usedInfos.size()) ? usedInfos[linearIndex] : nullptr;

            if (info)
            {
                const QString mac = info->getMac();

                if (oldIndex.contains(mac))
                {
                    QPoint oldPos = oldIndex[mac];
                    if (oldPos != QPoint(r, c))
                    {
                        std::swap(m_data[r][c], m_data[oldPos.x()][oldPos.y()]);
                        emit cellChanged(oldPos);
                    }
                    m_data[r][c]->updateFromNetworkInfo(info);
                }
                else
                {
                    delete m_data[r][c];
                    auto* model = new NetworkInfoModel(info, this);
                    //model->moveToThread(this->thread());
                    m_data[r][c] = model;
                    m_macIndex[info->getMac()] = QPoint(r, c);
                    emit cellChanged(QPoint(r, c));

                    // m_scheduler->scheduleMainThread("model_creation",
                    //                                 [this, r, c, info]()
                    //                                 {
                    //                                     delete m_data[r][c];
                    //                                     m_data[r][c] = new NetworkInfoModel(info, nullptr);
                    //                                     m_macIndex[info->getMac()] = QPoint(r, c);
                    //                                     emit cellChanged(QPoint(r, c));
                    //                                 },
                    //                                 QThread::HighPriority
                    //                                 );
                }
                m_macIndex[mac] = QPoint(r, c);
            }
            else if (m_data[r][c])
            {
                // Schedule cleanup in main thread
                m_scheduler->scheduleMainThread(
                    "model_cleanup",
                    [=] {
                        delete m_data[r][c];
                        m_data[r][c] = nullptr;
                        emit cellChanged({r,c});
                    }
                    );
            }
        }
    }

    // Cleanup remaining old entries
    for (auto it = oldIndex.constBegin(); it != oldIndex.constEnd(); ++it)
    {
        const QPoint pos = it.value();
        if (!m_macIndex.contains(it.key()) && m_data[pos.x()][pos.y()])
        {
            m_scheduler->scheduleMainThread("legacy_cleanup",
                                            [this, pos]() {
                                                delete m_data[pos.x()][pos.y()];
                                                m_data[pos.x()][pos.y()] = nullptr;
                                                emit cellChanged(pos);
                                            },
                                            QThread::NormalPriority
                                            );
        }
    }

    //qDeleteAll(usedInfos);

    // // Clear temporary lists after scheduling all tasks
    // QMetaObject::invokeMethod(this, [usedInfos]() {
    //     qDeleteAll(usedInfos);
    // }, Qt::QueuedConnection);
}

void GridDataManager::handleNetworkStatsImpl(const QString& mac, const quint64& rxSpeed, const quint64& txSpeed)
{
    if(m_macIndex.contains(mac))
    {
        NetworkInfoModel* model = m_data[m_macIndex[mac].x()][m_macIndex[mac].y()];
        model->updateSpeeds(rxSpeed, txSpeed);
    }
}

void GridDataManager::clearGrid()
{
    for(auto& row : m_data)
    {
        qDeleteAll(row);
        row.clear();
    }
    m_data.clear();
    m_macIndex.clear();
}

void GridDataManager::updateMacMap()
{
    m_macIndex.clear();
    for (int r = 0; r < m_data.size(); ++r)
    {
        for (int c = 0; c < m_data[r].size(); ++c)
        {
            if (m_data[r][c] && !m_data[r][c]->getMac().isEmpty())
            {
                m_macIndex.insert(m_data[r][c]->getMac(), QPoint(r, c));
            }
        }
    }
}
