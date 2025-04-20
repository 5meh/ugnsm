#include "griddatamanager.h"

#include "../../Network/Information/networkinfo.h"
#include "../../Network/Information/networkinfomodel.h"
#include "../Utilities/Parser/networkethernetparser.h"
#include "../Network/NetworkSortingStrategies/speedsortstrategy.h"
#include "../Network/Monitoring/networkmonitor.h"
#include "../../componentsystem.h"

#include <QTimer>

GridDataManager::GridDataManager(ComponentSystem& system, QObject* parent)
    : m_monitor{new NetworkMonitor{this}},//TODO: mb use "old" syntaxis
    m_sorter{system.create<INetworkSortStrategy>()},
    m_parser{system.create<IParser>()},
    QObject{parent}
{
    connect(m_parser, &IParser::parsingCompleted,
            this, &GridDataManager::handleParsingCompleted);
    connect(m_monitor, &NetworkMonitor::statsUpdated,
            this, &GridDataManager::handleNetworkStats);

    QTimer* refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &GridDataManager::refreshData);
    refreshTimer->start(5000);
    m_monitor->startMonitoring(1000);

    // Initial data load
    QMetaObject::invokeMethod(this, "refreshData", Qt::QueuedConnection);
}

GridDataManager::~GridDataManager()
{
    clearGrid();
}

NetworkInfoModel* GridDataManager::cellData(int row, int col) const
{
    if(row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size())
        return m_data[row][col];
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
    emit gridDimensionsChanged();
    refreshData();
}

void GridDataManager::swapCells(int fromRow, int fromCol, int toRow, int toCol)
{
    if(fromRow < 0 || fromRow >= getRows() || fromCol < 0 || fromCol >= getCols() ||
        toRow < 0 || toRow >= getRows() || toCol < 0 || toCol >= getCols())
        return;

    qSwap(m_data[fromRow][fromCol], m_data[toRow][toCol]);
    emit modelChanged();
}

void GridDataManager::handleParsingCompleted(const QVariant& result)
{
    QList<NetworkInfo*> newData = result.value<QList<NetworkInfo*>>();
    m_sorter->sort(newData);

    clearGrid();
    const int maxItems = qMin(newData.size(), 9);
    const int numRows = getRows();
    const int numCols = getCols();

    if(numRows == 0 || numCols == 0) return;

    int itemIndex = 0;
    for(int row = 0; row < numRows && itemIndex < maxItems; ++row)
    {
        m_data[row].resize(numCols);
        for(int col = 0; col < numCols && itemIndex < maxItems; ++col)
        {
            NetworkInfo* info = newData[itemIndex];
            m_data[row][col] = new NetworkInfoModel(info, this);
            ++itemIndex;
        }
    }

    for(int i = maxItems; i < newData.size(); ++i)
        delete newData[i];

    updateMacMap();
    emit modelChanged();
}

void GridDataManager::handleNetworkStats(const QString& mac, quint64 rxSpeed, quint64 txSpeed)
{
    if(m_macMap.contains(mac))
    {
        NetworkInfoModel* model = m_macMap[mac];
        model->updateSpeeds(rxSpeed, txSpeed);
    }
}

void GridDataManager::refreshData()
{
    if(m_parser)
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
    m_macMap.clear();
}

void GridDataManager::updateMacMap()
{
    m_macMap.clear();
    for(const auto& row : m_data)
    {
        for(NetworkInfoModel* model : row)
        {
            if(model  && !model->getMac().isEmpty())
                m_macMap.insert(model->getMac(), model);
        }
    }
}
