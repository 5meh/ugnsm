// griddatamanager.cpp
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
    m_monitor{new NetworkMonitor{nullptr, this}},
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

void GridDataManager::swapCells(QPoint from, QPoint to)
{
    m_scheduler->scheduleMainThread(
        QString("grid_swap"),
        [this, from, to]()
        {
            swapCellsImpl(from, to);
        },
        QThread::HighPriority
        );
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

void GridDataManager::handleNetworkStats(QString mac, quint64 rxSpeed, quint64 txSpeed)
{

    m_scheduler->schedule(QString("stats_update"),
                          this,
                          &GridDataManager::handleNetworkStatsImpl,
                          QThread::LowPriority,
                          std::move(mac),
                          rxSpeed,
                          txSpeed);
}

void GridDataManager::refreshData()
{
    m_parser->parse();
}

void GridDataManager::swapCellsImpl(QPoint from, QPoint to)
{
    if(from.x() < 0 || from.x() >= getRows() || from.y() < 0 || from.y() >= getCols() ||
        to.x() < 0 || to.x() >= getRows() || to.y() < 0 || to.y() >= getCols())
    {
        Logger::instance().log(Logger::Warning, "Invalid swap coordinates", "Grid");
        return;
    }

    std::swap(m_data[from.x()][from.y()], m_data[to.x()][to.y()]);
    updateMacMap();

    emit cellChanged(from, m_data[from.x()][from.y()]);
    emit cellChanged(to, m_data[to.x()][to.y()]);
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

    QHash<QString, QPoint> oldIndex = m_macIndex;
    m_macIndex.clear();

    QVector<QPair<QPoint, NetworkInfo*>> cellsToCreate;

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            const int linearIndex = r * cols + c;
            NetworkInfo* info = (linearIndex < usedInfos.size()) ? usedInfos[linearIndex] : nullptr;

            if (info)
            {
                const QString mac = info->getMac();
                if (!oldIndex.contains(mac))
                {
                    cellsToCreate.append(qMakePair(QPoint(r, c), info));
                }
            }
        }
    }

    // Schedule QObject creation in main thread
    if (!cellsToCreate.isEmpty())
    {
        m_scheduler->scheduleMainThread(
            QString("model_creation"),
            [this, cellsToCreate]()
            {
                for (const auto& pair : cellsToCreate)
                {
                    QPoint pos = pair.first;
                    NetworkInfo* info = pair.second;

                    delete m_data[pos.x()][pos.y()];
                    m_data[pos.x()][pos.y()] = new NetworkInfoModel(info, this);
                    m_macIndex[info->getMac()] = pos;
                    emit cellChanged(pos, m_data[pos.x()][pos.y()]);
                }
            },
            QThread::HighPriority
            );
    }

    // Cleanup in worker thread
    qDeleteAll(unusedInfos);
    unusedInfos.clear();
}

void GridDataManager::handleNetworkStatsImpl(QString mac, quint64 rxSpeed, quint64 txSpeed)
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
