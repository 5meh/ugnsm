#include "griddatamanager.h"

#include "../Utilities/Logger/logger.h"
#include "../../Network/Information/networkinfo.h"
#include "../../Network/Information/networkinfomodel.h"
#include "../Utilities/Parser/iparser.h"
#include "../NetworkSortingStrategies/inetworksortstrategy.h"
#include "../Utilities/Parser/networkethernetparser.h"
#include "../NetworkSortingStrategies/speedsortstrategy.h"
#include "../Monitoring/networkmonitor.h"
#include "../../globalmanager.h"
#include "../Utilities/MessageBoxManager/messageboxmanager.h"

#include <QTimer>
#include <QMessageBox>
#include <QCheckBox>

GridDataManager::GridDataManager(QObject* parent)
    :m_monitor{new NetworkMonitor{nullptr, this}},
    m_sorter{GlobalManager::componentRegistry()->create<INetworkSortStrategy>(this)},
    m_parser{GlobalManager::componentRegistry()->create<IParser>(nullptr)},
    m_validDataCount(0),
    QObject{parent}
{
    connect(m_parser.get(), &IParser::parsingCompleted,
            this, &GridDataManager::handleParsingCompleted, Qt::QueuedConnection);
    connect(m_monitor, &NetworkMonitor::statsUpdated,
            this, &GridDataManager::handleNetworkStats);
    GlobalManager::messageBoxManager()->addBlockingRelationship("BestNetworkMove","SwapWarning");

    GlobalManager::taskScheduler()->scheduleRepeating("data_refresh", 5000, this,
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

int GridDataManager::getCapacity() const
{
    return getRows() * getCols();
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
    GlobalManager::taskScheduler()->scheduleMainThread(
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
    QVariant resultCopy = result;

    GlobalManager::taskScheduler()->scheduleAtomic(m_refreshInProgress,
                                                   QString("data_processing"),
                                                   this,
                                                   &GridDataManager::handleParsingCompletedImpl,
                                                   QThread::NormalPriority,
                                                   std::move(resultCopy));
}

void GridDataManager::handleNetworkStats(QString mac, quint64 rxSpeed, quint64 txSpeed)
{
    GlobalManager::taskScheduler()->schedule(QString("stats_update"),
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

    if(to == QPoint{0,0})
    {
        const QString dialogId = "BestNetworkMove";
        auto result = GlobalManager::messageBoxManager()->showDialog(dialogId,
                                                                     "Network Change",
                                                                     "New best network detected. Move to primary position?",
                                                                     "Do not show this message again"
                                                                     );
        if(result == QMessageBox::No)
            return;
    }

    std::swap(m_data[from.x()][from.y()], m_data[to.x()][to.y()]);
    updateMacMap();

    emit cellChanged(from, m_data[from.x()][from.y()]);
    emit cellChanged(to, m_data[to.x()][to.y()]);
}

void GridDataManager::handleParsingCompletedImpl(QVariant result)
{
    QList<NetworkInfo*> parsedInfoList = result.value<QList<NetworkInfo*>>();

    QList<NetworkInfoPtr> allInfos;
    allInfos.reserve(parsedInfoList.size());
    std::transform(parsedInfoList.begin(), parsedInfoList.end(),
                   std::back_inserter(allInfos),
                   [](NetworkInfo* ptr){ return NetworkInfoPtr(ptr); });

    m_sorter->sort(allInfos);

    if (m_validDataCount == 0)
        initializeGridWithData(allInfos);
    else
        updateGridWithData(allInfos);
}

void GridDataManager::handleNetworkStatsImpl(QString mac, quint64 rxSpeed, quint64 txSpeed)
{
    if(m_macIndex.contains(mac))
    {
        NetworkInfoModel* model = m_data[m_macIndex[mac].x()][m_macIndex[mac].y()];
        model->updateSpeeds(rxSpeed, txSpeed);
    }
}

void GridDataManager::clearGrid()//TODO:mb rework
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

void GridDataManager::initializeGridWithData(const QList<NetworkInfoPtr>& allInfos)
{
    QList<NetworkInfoPtr> usedInfos = allInfos.mid(0, getCapacity());
    QList<NetworkInfoPtr> unusedInfos = allInfos.mid(getCapacity());

    GlobalManager::taskScheduler()->scheduleMainThread(
        QString("model_creation"),
        [this, usedInfos]()
        {
            for(size_t x = 0; x < getRows(); x++)
            {
                for(size_t y = 0; y < getCols(); y++)
                {
                    const int linearIndex = x * getCols() + y;
                    if(linearIndex > usedInfos.size() - 1)
                        continue;
                    m_data[x][y] = new NetworkInfoModel(usedInfos[linearIndex], this);
                    m_macIndex[usedInfos[linearIndex]->getMac()] = QPoint(x,y);
                    ++m_validDataCount;
                    cellChanged(QPoint(x,y), m_data[x][y]);
                }
            }
        },
        QThread::HighPriority
        );

    unusedInfos.clear();//TODO:mb rework
}

void GridDataManager::updateGridWithData(const QList<NetworkInfoPtr>& allInfos)
{
    QList<NetworkInfoPtr> usedInfos = allInfos.mid(0, getCapacity());

    QHash<QString, NetworkInfoPtr> newMacs;
    for(NetworkInfoPtr info : usedInfos)
        newMacs.insert(info->getMac(), info);

    QVector<QPair<QPoint, NetworkInfoPtr>> updates;

    for(auto it = m_macIndex.begin(); it != m_macIndex.end();)
    {
        if(!newMacs.contains(it.key()))
        {
            updates.append({it.value(), nullptr});
            it = m_macIndex.erase(it);
        }
        else
            ++it;
    }

    for(auto it = newMacs.begin(); it != newMacs.end(); ++it)
    {
        const QString& mac = it.key();
        NetworkInfoPtr info = it.value();

        if(m_macIndex.contains(mac))
            updates.append({m_macIndex[mac], info});
        else
        {
            for(int r = 0; r < getRows(); ++r)
            {
                for(int c = 0; c < getCols(); ++c)
                {
                    QPoint pos(r, c);
                    if(!m_macIndex.values().contains(pos))
                    {
                        updates.append({pos, info});
                        m_macIndex.insert(mac, pos);
                        break;
                    }
                }
            }
        }
    }

    const int bestIndex = m_sorter->findBestNetwork(allInfos);
    if(bestIndex >= 0 && bestIndex < allInfos.size())
    {
        NetworkInfoPtr newBest = allInfos[bestIndex];
        if(newBest && m_macIndex.contains(newBest->getMac()))
        {
            QPoint bestPos = m_macIndex[newBest->getMac()];
            swapCells(bestPos, QPoint(0, 0));
        }

        GlobalManager::taskScheduler()->scheduleMainThread("gridUpdate", [=]() {

            for(const QPair<QPoint, NetworkInfoPtr>& pair : updates)
            {
                if(!pair.second)
                {
                    delete m_data[pair.first.x()][pair.first.y()];
                    m_data[pair.first.x()][pair.first.y()] = nullptr;
                    --m_validDataCount;
                    emit cellChanged(pair.first, nullptr);
                }
            }

            for(const QPair<QPoint, NetworkInfoPtr>& pair : updates)
            {
                if(pair.second)
                {
                    QPoint pos = pair.first;
                    NetworkInfoPtr info = pair.second;

                    if(m_data[pos.x()][pos.y()])
                        m_data[pos.x()][pos.y()]->updateFromNetworkInfo(info);
                    else
                    {
                        m_data[pos.x()][pos.y()] = new NetworkInfoModel(info, this);
                        ++m_validDataCount;
                    }

                    emit cellChanged(pos, m_data[pos.x()][pos.y()]);
                }
            }
        });
    }
}
