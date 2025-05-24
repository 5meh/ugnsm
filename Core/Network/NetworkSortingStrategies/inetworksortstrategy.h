#ifndef INETWORKSORTSTRATEGY_H
#define INETWORKSORTSTRATEGY_H

#include <QObject>
#include <QtPlugin>

#include "../Network/Information/networkinfo.h"

class INetworkSortStrategy : public QObject
{
    Q_OBJECT
public:
    explicit INetworkSortStrategy(QObject *parent = nullptr);
    virtual ~INetworkSortStrategy() = default;
    virtual void sort(QList<NetworkInfoPtr>& networks) = 0;
    virtual int findBestNetwork(const QList<NetworkInfoPtr>& networks) = 0;
};

#define INetworkSortStrategy_iid "com.ugnsm.INetworkSortStrategy"
Q_DECLARE_INTERFACE(INetworkSortStrategy, INetworkSortStrategy_iid)


#endif // INETWORKSORTSTRATEGY_H
