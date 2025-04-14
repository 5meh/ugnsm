#ifndef INETWORKSORTSTRATEGY_H
#define INETWORKSORTSTRATEGY_H

#include <QObject>

class NetworkInfo;

class INetworkSortStrategy : public QObject
{
    Q_OBJECT
public:
    explicit INetworkSortStrategy(QObject *parent = nullptr);
    virtual ~INetworkSortStrategy() = default;
    virtual void sort(QList<NetworkInfo*>& networks) = 0;
};

#endif // INETWORKSORTSTRATEGY_H
