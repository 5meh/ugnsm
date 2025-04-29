#include "griddatamanager.h"

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

    qSwap(m_data[from.x()][from.y()], m_data[to.x()][to.y()]);

    updateMacMap();

    emit cellChanged(from);
    emit cellChanged(to);
}

void GridDataManager::handleParsingCompleted(const QVariant& result)
{
    QList<NetworkInfo*> incoming = result.value<QList<NetworkInfo*>>();
    m_sorter->sort(incoming);

    const int rows = getRows();
    const int cols = getCols();
    const int capacity = rows * cols;
    incoming = incoming.mid(0,capacity);

    QHash<QString, QPoint> oldIndex = m_macIndex;

    for (int idx = 0; idx < capacity; ++idx)
    {
        int r = idx / cols;
        int c = idx % cols;

        if (idx < incoming.size())
        {
            NetworkInfo* info = incoming.at(idx);
            QString mac = info->getMac();

            if (auto it = oldIndex.find(mac); it != oldIndex.end())
            {
                QPoint oldPos = *it;
                NetworkInfoModel* model = m_data[oldPos.x()][oldPos.y()];

                if (oldPos != QPoint(r,c))
                {
                    std::swap(m_data[r][c], m_data[oldPos.x()][oldPos.y()]);
                }

                m_data[r][c]->updateFromNetworkInfo(info);

                oldIndex.remove(mac);
            }
            else
            {
                delete m_data[r][c];
                m_data[r][c] = new NetworkInfoModel(info, this);
            }
        }
        else
        {
            delete m_data[r][c];
            m_data[r][c] = nullptr;
        }

        emit cellChanged(QPoint(r, c));
    }

    for (auto it = oldIndex.constBegin(); it != oldIndex.constEnd(); ++it)
    {
        QPoint p = it.value();
        delete m_data[p.x()][p.y()];
        m_data[p.x()][p.y()] = nullptr;
        emit cellChanged(p);
    }

    for (int i = capacity; i < result.value<QList<NetworkInfo*>>().size(); ++i)
    {
        delete incoming[i];
    }

    m_macIndex.clear();
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (auto *model = m_data[r][c])
            {
                const QString m = model->getMac();
                if (!m.isEmpty())
                    m_macIndex.insert(m, QPoint(r,c));
            }
        }
    }
    //emit modelChanged();//TODO: mb remove
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
