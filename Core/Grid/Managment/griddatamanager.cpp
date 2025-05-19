// griddatamanager.cpp
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

#include <QTimer>
#include <QMessageBox>
#include <QCheckBox>

GridDataManager::GridDataManager(QObject* parent)
    :m_monitor{new NetworkMonitor{nullptr, this}},
    m_sorter{GlobalManager::componentRegistry()->create<INetworkSortStrategy>(this)},
    m_parser{GlobalManager::componentRegistry()->create<IParser>(nullptr)},
    QObject{parent}
{
    connect(m_parser.get(), &IParser::parsingCompleted,
            this, &GridDataManager::handleParsingCompleted, Qt::QueuedConnection);
    connect(m_monitor, &NetworkMonitor::statsUpdated,
            this, &GridDataManager::handleNetworkStats);

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
    Q_ASSERT(result.canConvert<QList<NetworkInfo*>>());
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
    GlobalManager::taskScheduler()->scheduleAtomic(m_refreshInProgress,
                                "data_refresh_task",
                                m_parser.get(),
                                &IParser::parse);
}

void GridDataManager::swapCellsImpl(QPoint from, QPoint to)
{
    if(from.x() < 0 || from.x() >= getRows() || from.y() < 0 || from.y() >= getCols() ||
        to.x() < 0 || to.x() >= getRows() || to.y() < 0 || to.y() >= getCols())
    {
        Logger::instance().log(Logger::Warning, "Invalid swap coordinates", "Grid");
        return;
    }

    if ((from.x() == 0 && from.y() == 0) || (to.x() == 0 && to.y() == 0))
    {
        if(m_showBestNetworkWarning)
            if (!showBestNetworkWarning())
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

    if (m_data.isEmpty())
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

void GridDataManager::initializeGridWithData(const QList<NetworkInfo*>& allInfos)
{
    Q_ASSERT(m_data.isEmpty());

    const int rows = getRows();
    const int cols = getCols();
    const int capacity = rows * cols;
    QList<NetworkInfo*> usedInfos = allInfos.mid(0, capacity);
    QList<NetworkInfo*> unusedInfos = allInfos.mid(capacity);

    GlobalManager::taskScheduler()->scheduleMainThread(
        QString("model_creation"),
        [this, rows, cols, usedInfos]()//TODO:change capture values
        {
            for(size_t x = 0; x < rows; x++)
            {
                for(size_t y = 0; y < cols; y++)
                {
                    const int linearIndex = x * cols + y;
                    NetworkInfo* info;
                    if(linearIndex < usedInfos.size())
                        info = usedInfos[linearIndex];
                    else
                        continue;
                    //NetworkInfo* info = (linearIndex < usedInfos.size()) ? usedInfos[linearIndex] : {continue;};
                    delete m_data[x][y];
                    m_data[x][y] = new NetworkInfoModel(info, this);
                    m_macIndex[info->getMac()] = QPoint(x,y);
                    cellChanged(QPoint(x,y), m_data[x][y]);
                }
            }
        },
        QThread::HighPriority
        );

    qDeleteAll(unusedInfos);
    unusedInfos.clear();
}

void GridDataManager::updateGridWithData(const QList<NetworkInfo*>& allInfos)
{
    Q_ASSERT(!m_data.isEmpty());

    const int rows = getRows();
    const int cols = getCols();
    const int capacity = rows * cols;

    QList<NetworkInfo*> sortableList = allInfos;
    m_sorter->sort(sortableList);
    QList<NetworkInfo*> usedInfos = sortableList.mid(0, capacity);

    QHash<QString, NetworkInfo*> newMacs;
    for(NetworkInfo* info : usedInfos)
        newMacs.insert(info->getMac(), info);

    QVector<QPair<QPoint, NetworkInfo*>> updates;

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
        NetworkInfo* info = it.value();

        if(m_macIndex.contains(mac))
            updates.append({m_macIndex[mac], info});
        else
        {
            for(int r = 0; r < rows; ++r)
            {
                for(int c = 0; c < cols; ++c)
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

    const int bestIndex = m_sorter->findBestNetwork(sortableList);
    if(bestIndex >= 0 && bestIndex < sortableList.size())
    {
        NetworkInfo* newBest = sortableList[bestIndex];
        if(newBest && m_macIndex.contains(newBest->getMac()))
        {
            QPoint bestPos = m_macIndex[newBest->getMac()];
            if(bestPos != QPoint(0, 0))
            {
                bool showWarning = GlobalManager::settingsManager()->getShowBestNetworkWarning();
                if(showWarning)
                {
                    GlobalManager::taskScheduler()->scheduleMainThread("bestNetworkCheck", [=]() {
                        QMessageBox::StandardButton reply = QMessageBox::question(
                            nullptr,
                            tr("Network Change"),
                            tr("New best network detected. Move to primary position?"),
                            QMessageBox::Yes | QMessageBox::No
                            );
                        if(reply == QMessageBox::Yes)
                            swapCellsImpl(bestPos, QPoint(0,0));
                    });
                }
                else
                    swapCellsImpl(bestPos, QPoint(0,0));
            }
        }
    }

    GlobalManager::taskScheduler()->scheduleMainThread("gridUpdate", [=]() {

        for(const QPair<QPoint, NetworkInfo*>& pair : updates)
        {
            if(!pair.second)
            {
                delete m_data[pair.first.x()][pair.first.y()];
                m_data[pair.first.x()][pair.first.y()] = nullptr;
                emit cellChanged(pair.first, nullptr);
            }
        }

        for(const QPair<QPoint, NetworkInfo*>& pair : updates)
        {
            if(pair.second)
            {
                QPoint pos = pair.first;
                NetworkInfo* info = pair.second;

                if(m_data[pos.x()][pos.y()])
                    m_data[pos.x()][pos.y()]->updateFromNetworkInfo(info);
                else
                    m_data[pos.x()][pos.y()] = new NetworkInfoModel(info, this);

                emit cellChanged(pos, m_data[pos.x()][pos.y()]);
            }
        }

        qDeleteAll(allInfos);
    });
}

bool GridDataManager::showBestNetworkWarning()
{
    QMessageBox msgBox;
    msgBox.setText("You are trying to swap the best network.");
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setCheckBox(new QCheckBox("Do not show this message again"));

    int ret = msgBox.exec();
    if (msgBox.checkBox()->isChecked())
        m_showBestNetworkWarning = false;

    return ret == QMessageBox::Yes;
}
