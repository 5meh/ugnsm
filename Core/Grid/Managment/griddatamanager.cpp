#include "griddatamanager.h"

#include "../../Network/Information/networkinfo.h"
#include "../../Network/Information/networkinfomodel.h"

#include <QTimer>

GridDataManager::GridDataManager(QObject* parent)
    : QObject{parent}
{
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GridDataManager::refreshGrid);
    timer->start(5000);
}

NetworkInfoModel* GridDataManager::cellData(int row, int col) const
{
    if(row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size())
        return m_data[row][col];
    return nullptr;
}

void GridDataManager::initializeData(int rows, int cols)
{
    for(auto& row : m_data)
        qDeleteAll(row);
    m_data.clear();

    QList<QNetworkInterface> interfaces = getSortedInterfaces();

    m_data.resize(rows);
    for(int i=0; i<rows; ++i)
    {
        m_data[i].resize(cols);
        for(int j=0; j<cols; ++j)
        {
            int idx = i*cols + j;
            NetworkInfo* info = idx < interfaces.size() ?
                                    new NetworkInfo(interfaces[idx], this) :
                                    new NetworkInfo(/* dummy */);
            m_data[i][j] = new NetworkInfoModel(info, this);
        }
    }
    emit modelChanged();
}

QList<QNetworkInterface> GridDataManager::getSortedInterfaces() const
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    QList<QNetworkInterface> ethernet;
    for(const QNetworkInterface& iface : interfaces)
    {
        if(iface.type() == QNetworkInterface::Ethernet &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            ethernet.append(iface);//TODO:rework initialization
        }
    }

    // std::sort(ethernet.begin(), ethernet.end(),
    //           [](const QNetworkInterface& a, const QNetworkInterface& b)
    //           {
    //               return a.speed() > b.speed();
    //           });

    return ethernet.mid(0, 9); // Max 9 best interfaces
}

void GridDataManager::refreshGrid()
{
    initializeData(m_data.size(), m_data.isEmpty() ? 0 : m_data[0].size());
}
