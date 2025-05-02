#include "griddatamanager.h"

#include "../Utilities/Logger/logger.h"
#include "../../Network/Information/networkinfo.h"
#include "../../Network/Information/networkinfomodel.h"
#include "../Utilities/Parser/iparser.h"
#include "../NetworkSortingStrategies/inetworksortstrategy.h"
#include "../Utilities/Parser/networkethernetparser.h"

#include "../NetworkSortingStrategies/speedsortstrategy.h"
#include "../Monitoring/networkmonitor.h"
#include "../../componentregistry.h"

#include <QTimer>

GridDataManager::GridDataManager(QObject* parent)
    : m_monitor{new NetworkMonitor{this}},//TODO: mb use "old" syntaxis
    m_sorter{ComponentRegistry::create<INetworkSortStrategy>()},
    m_parser{ComponentRegistry::create<IParser>()},
    QObject{parent}
{
    connect(m_parser.get(), &IParser::parsingCompleted,
            this, &GridDataManager::handleParsingCompleted);
    connect(m_monitor, &NetworkMonitor::statsUpdated,
            this, &GridDataManager::handleNetworkStats);

    QTimer* refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &GridDataManager::refreshData);
    refreshTimer->start(5000);
    m_monitor->startMonitoring(1000);

    // Initial data load
    //QMetaObject::invokeMethod(this, "refreshData", Qt::QueuedConnection);
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
    if(from.x() < 0 || from.x() >= getRows() || from.y() < 0 || from.y() >= getCols() ||
        to.x() < 0 || to.x() >= getRows() || to.y() < 0 || to.y() >= getCols())
        return;

    if(from == to) return;

    std::swap(m_data[from.x()][from.y()], m_data[to.x()][to.y()]);

    updateMacMap();

    emit cellChanged(from);
    emit cellChanged(to);
}

void GridDataManager::handleParsingCompleted(const QVariant& result)
{
    // Get sorted network info list and calculate grid parameters
    QList<NetworkInfo*> allInfos = result.value<QList<NetworkInfo*>>();
    m_sorter->sort(allInfos);

    const int rows = getRows();
    const int cols = getCols();
    const int capacity = rows * cols;

    // Split into used and unused NetworkInfo objects
    QList<NetworkInfo*> usedInfos = allInfos.mid(0, capacity);
    QList<NetworkInfo*> unusedInfos = allInfos.mid(capacity);

    // Immediately delete unused NetworkInfo objects
    qDeleteAll(unusedInfos);
    unusedInfos.clear();

    QHash<QString, QPoint> oldIndex = m_macIndex;
    m_macIndex.clear();

    // Process grid cells using explicit row/column iteration
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int linearIndex = r * cols + c;
            NetworkInfo* info = (linearIndex < usedInfos.size()) ? usedInfos[linearIndex] : nullptr;

            // Process valid network information
            if (info) {
                const QString mac = info->getMac();

                if (oldIndex.contains(mac)) {
                    // Update existing model position
                    QPoint oldPos = oldIndex[mac];
                    if (oldPos != QPoint(r, c)) {
                        std::swap(m_data[r][c], m_data[oldPos.x()][oldPos.y()]);
                        emit cellChanged(oldPos);
                    }
                    m_data[r][c]->updateFromNetworkInfo(info);
                } else {
                    // Create new model and transfer ownership
                    delete m_data[r][c];
                    m_data[r][c] = new NetworkInfoModel(info, this);
                }

                // Update MAC index
                m_macIndex[mac] = QPoint(r, c);
                emit cellChanged(QPoint(r, c));
            } else {
                // Clear cell if no corresponding info
                if (m_data[r][c]) {
                    delete m_data[r][c];
                    m_data[r][c] = nullptr;
                    emit cellChanged(QPoint(r, c));
                }
            }
        }
    }

    // Cleanup remaining old entries that weren't updated
    for (auto it = oldIndex.constBegin(); it != oldIndex.constEnd(); ++it) {
        const QPoint pos = it.value();
        if (!m_macIndex.contains(it.key()) && m_data[pos.x()][pos.y()]) {
            delete m_data[pos.x()][pos.y()];
            m_data[pos.x()][pos.y()] = nullptr;
            emit cellChanged(pos);
        }
    }

    // Clear temporary lists without deleting managed pointers
    usedInfos.clear();
}

void GridDataManager::handleNetworkStats(const QString& mac, quint64 rxSpeed, quint64 txSpeed)
{
    if(m_macIndex.contains(mac))
    {
        NetworkInfoModel* model = m_data[m_macIndex[mac].x()][m_macIndex[mac].y()];
        model->updateSpeeds(rxSpeed, txSpeed);
    }
}

void GridDataManager::refreshData()
{
    //if(m_parser)
    m_parser->parse();
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
