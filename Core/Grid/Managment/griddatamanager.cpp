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
    :m_monitor{new NetworkMonitor{this}},
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

    int interval = GlobalManager::settingsManager()->getUpdateInterval();

    GlobalManager::taskScheduler()->scheduleRepeating("data_refresh",
                                                      interval,
                                                      this,
                                                      &GridDataManager::refreshData,
                                                      QThread::NormalPriority);

    Logger::instance().log(Logger::Info, "GridDataManager initialized", "Grid");
}

GridDataManager::~GridDataManager()
{
    clearGrid();
}

QSharedPointer<NetworkInfoModel> GridDataManager::cellData(QPoint indx) const
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
    Logger::instance().log(Logger::Info,
                           QString("Initializing grid to %1x%2").arg(rows).arg(cols),
                           "Grid");
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
    GlobalManager::taskScheduler()->executeMainThread(
        QString("grid_swap"),
        [this, from, to]()
        {
            swapCellsImpl(from, to);
        },
        QThread::HighPriority
        );
}

void GridDataManager::setUpdatesPaused(bool paused)
{
    m_updatesPaused = paused;

    // Apply queued updates when unpausing
    if (!paused && !m_queuedUpdates.isEmpty())
    {
        // Only process the latest update
        QList<NetworkInfoPtr> latest = m_queuedUpdates.last();
        m_queuedUpdates.clear();
        updateGridWithData(latest);
    }
}

void GridDataManager::handleParsingCompleted(const QVariant& result)
{
    QVariant resultCopy = result;

    GlobalManager::taskScheduler()->executeMainThread(QString("data_processing"),
                                                      [this, resultCopy = std::move(resultCopy)]() {
                                                          // now we’re safely on the GUI thread:
                                                          handleParsingCompletedImpl(std::move(resultCopy));
                                                      });
}

void GridDataManager::handleNetworkStats(QString mac, quint64 rxSpeed, quint64 txSpeed)
{
    GlobalManager::taskScheduler()->executeMainThread(
        "stats_update",
        this,
        &GridDataManager::handleNetworkStatsImpl,
        Qt::QueuedConnection,
        std::move(mac),
        rxSpeed,
        txSpeed
        );
}

void GridDataManager::refreshData()
{
    GlobalManager::taskScheduler()->scheduleAtomic(m_refreshInProgress,
                                                   "data_refresh_task",
                                                   m_parser.get(),
                                                   &IParser::parse);
}

void GridDataManager::triggerRefresh()
{
    GlobalManager::taskScheduler()->scheduleAtomic(
        m_refreshInProgress,
        "data_refresh_manual",
        m_parser.get(),
        &IParser::parse
        );
}

void GridDataManager::swapCellsImpl(QPoint from, QPoint to)
{
    Logger::instance().log(Logger::Debug,
                           QString("Swapping cells: (%1,%2) -> (%3,%4)").arg(from.x()).arg(from.y()).arg(to.x()).arg(to.y()),
                           "Grid");

    if(from.x() < 0 || from.x() >= getRows() || from.y() < 0 || from.y() >= getCols() ||
        to.x() < 0 || to.x() >= getRows() || to.y() < 0 || to.y() >= getCols())
    {
        Logger::instance().log(Logger::Warning, "Invalid swap coordinates", "Grid");
        return;
    }

    if(to == QPoint{0,0})
    {
        const QString dialogId = "BestNetworkMove";

        auto result = GlobalManager::messageBoxManager()->showDialog(
            dialogId,
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
        QSharedPointer<NetworkInfoModel> model = m_data[m_macIndex[mac].x()][m_macIndex[mac].y()];
        model->updateSpeeds(rxSpeed, txSpeed);
    }
}

void GridDataManager::clearGrid()//TODO:mb rework
{
    for (auto& row : m_data)
    {
        for (auto& item : row)
            item.clear();
        row.clear();
    }
    m_data.clear();
    m_macIndex.clear();
}

void GridDataManager::updateMacMap()
{
    m_macIndex.clear();
    m_interfaceToMac.clear();
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

    GlobalManager::taskScheduler()->executeMainThread(
        QString("model_creation"),
        [this, usedInfos]()
        {
            QSet<QString> macsToTrack;
            for(int x = 0; x < getRows(); x++)
            {
                for(int y = 0; y < getCols(); y++)
                {
                    const int linearIndex = x * getCols() + y;
                    if(linearIndex >= usedInfos.size())
                        continue;

                    auto networkInfo = usedInfos[linearIndex];
                    QString mac = networkInfo->getMac();
                    m_data[x][y] = QSharedPointer<NetworkInfoModel>::create(networkInfo, this);
                    m_macIndex[mac] = QPoint(x,y);
                    macsToTrack.insert(mac);
                    ++m_validDataCount;
                    //emit cellChanged(QPoint(x,y), m_data[x][y]);
                }
            }

            // QSet<QString> interfaces;

            // for (int r = 0; r < m_data.size(); ++r)
            // {
            //     for (int c = 0; c < m_data[r].size(); ++c)
            //     {
            //         QSharedPointer<NetworkInfoModel> model = m_data[r][c];
            //         if (!model.isNull())
            //             interfaces.insert(model->getName());
            //     }
            // }

            m_monitor->initializeStats(macsToTrack);
            m_monitor->startMonitoring(1000);

            for(int x = 0; x < getRows(); x++)
            {
                for(int y = 0; y < getCols(); y++)
                {
                    if(!m_data[x][y].isNull())
                    {
                        qDebug() << "Emitter thread:" << QThread::currentThread();
                        emit cellChanged(QPoint(x,y), m_data[x][y]);
                    }
                }
            }
        },
        QThread::HighPriority
        );

    unusedInfos.clear();//TODO:mb rework
}

void GridDataManager::showBestNetworkChangedMessage(NetworkInfoPtr newBest)//TODO: why we need this method?
{
    GlobalManager::taskScheduler()->executeMainThread("bestNetworkChanged", [this, newBest]() {
        GlobalManager::messageBoxManager()->showDialog(
            "NewBestNetwork",
            "New Best Network Detected",
            QString("A new best network '%1' has been detected with speed %2 Mbps")
                .arg(newBest->getMac())
                .arg(newBest->getTotalSpeed()),
            "",
            QMessageBox::Ok
            );
    });
}

void GridDataManager::applyUpdates(const QVector<QPair<QPoint, NetworkInfoPtr>>& updates)
{
    GlobalManager::taskScheduler()->executeMainThread("gridUpdate", [this, updates]() {
        // Process removals first
        for(const auto& pair : updates)
        {
            if(!pair.second)
            {
                QPoint pos = pair.first;
                if(m_data[pos.x()][pos.y()])
                {
                    m_data[pos.x()][pos.y()].clear();
                    --m_validDataCount;
                    emit cellChanged(pos, nullptr);
                }
            }
        }
        for(const auto& pair : updates)
        {
            if(pair.second)
            {
                QPoint pos = pair.first;
                NetworkInfoPtr info = pair.second;

                if(m_data[pos.x()][pos.y()])
                    m_data[pos.x()][pos.y()]->updateFromNetworkInfo(info);
                else
                {
                    m_data[pos.x()][pos.y()] = QSharedPointer<NetworkInfoModel>::create(info, this);
                    ++m_validDataCount;
                }
                emit cellChanged(pos, m_data[pos.x()][pos.y()]);
            }
        }

        // Rebuild MAC index
        m_macIndex.clear();
        for(int r = 0; r < getRows(); ++r)
        {
            for(int c = 0; c < getCols(); ++c)
            {
                if(m_data[r][c] && !m_data[r][c]->getMac().isEmpty())
                    m_macIndex.insert(m_data[r][c]->getMac(), QPoint(r, c));
            }
        }
    });
}

void GridDataManager::fullGridUpdate(const QList<NetworkInfoPtr>& allInfos)
{
    QVector<QPair<QPoint, NetworkInfoPtr>> updates;
    const int capacity = getCapacity();

    for(int r = 0; r < getRows(); ++r)
    {
        for(int c = 0; c < getCols(); ++c)
        {
            if(m_data[r][c])
                updates.append({QPoint(r, c), nullptr});
        }
    }

    for(int i = 0; i < qMin(capacity, allInfos.size()); ++i)
    {
        const int r = i / getCols();
        const int c = i % getCols();
        updates.append({QPoint(r, c), allInfos[i]});
    }

    applyUpdates(updates);

    if(!allInfos.isEmpty() && m_data[0][0])
    {
        const QString currentBest = m_data[0][0]->getMac();
        if(currentBest != allInfos[0]->getMac())
            showBestNetworkChangedMessage(allInfos[0]);
    }
}

void GridDataManager::incrementalUpdate(const QList<NetworkInfoPtr>& allInfos)
{
    QVector<QPair<QPoint, NetworkInfoPtr>> updates;
    QHash<QString, NetworkInfoPtr> newMacs;

    for(NetworkInfoPtr info : allInfos)
        newMacs.insert(info->getMac(), info);

    // Identify removed networks
    for(auto it = m_macIndex.begin(); it != m_macIndex.end(); ++it)
    {
        if(!newMacs.contains(it.key()))
            updates.append({it.value(), nullptr});
    }

    // Identify changed or new networks
    QList<QPoint> emptySlots;
    for(int r = 0; r < getRows(); ++r)
    {
        for(int c = 0; c < getCols(); ++c)
        {
            if(!m_data[r][c])
                emptySlots.append(QPoint(r, c));
        }
    }

    for(auto it = newMacs.begin(); it != newMacs.end(); ++it)
    {
        const QString& mac = it.key();
        NetworkInfoPtr info = it.value();

        if(m_macIndex.contains(mac))
            // Update existing
            updates.append({m_macIndex[mac], info});
        else if(!emptySlots.isEmpty())
            // Add to first empty slot
            updates.append({emptySlots.takeFirst(), info});
    }

    applyUpdates(updates);
}

void GridDataManager::keepBestUpdate(const QList<NetworkInfoPtr>& allInfos)
{
    QVector<QPair<QPoint, NetworkInfoPtr>> updates;
    NetworkInfoPtr currentBest = m_data[0][0] ? m_data[0][0]->getNetworkInfo() : nullptr;

    bool bestExists = false;
    for(NetworkInfoPtr info : allInfos)
    {
        if(currentBest && info->getMac() == currentBest->getMac())
        {
            bestExists = true;
            break;
        }
    }

    if(!bestExists)
    {
        fullGridUpdate(allInfos);
        return;
    }

    updates.append({QPoint(0, 0), currentBest});

    QList<NetworkInfoPtr> otherInfos;
    for(NetworkInfoPtr info : allInfos)
    {
        if(!currentBest || info->getMac() != currentBest->getMac())
            otherInfos.append(info);
    }

    m_sorter->sort(otherInfos);
    int index = 0;
    for(int r = 0; r < getRows(); ++r)
    {
        for(int c = 0; c < getCols(); ++c)
        {
            if(r == 0 && c == 0)
                continue; // Skip best position

            if(index < otherInfos.size())
                updates.append({QPoint(r, c), otherInfos[index++]});
            else if(m_data[r][c])
                updates.append({QPoint(r, c), nullptr});
        }
    }

    applyUpdates(updates);
}

void GridDataManager::updateGridWithData(const QList<NetworkInfoPtr>& allInfos)
{
    if (m_updatesPaused)
    {
        m_queuedUpdates.append(allInfos);
        return;
    }
    const QString strategy = GlobalManager::settingsManager()->getGridUpdateStrategy();

    if(strategy == "FullUpdate")
        fullGridUpdate(allInfos);
    else if(strategy == "IncrementalUpdate")
        incrementalUpdate(allInfos);
    else if(strategy == "KeepBestUpdate")
        keepBestUpdate(allInfos);

    updateTrackedMacs();
}

void GridDataManager::updateTrackedMacs()
{
    QSet<QString> macs;

    for (int r = 0; r < m_data.size(); ++r)
    {
        for (int c = 0; c < m_data[r].size(); ++c)
        {
            QSharedPointer<NetworkInfoModel> model = m_data[r][c];
            if (model)
            {
                macs.insert(model->getMac());
            }
        }
    }

    m_monitor->updateTrackedMacs(macs);
}
