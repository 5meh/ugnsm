#ifndef SPEEDSORTSTRATEGY_H
#define SPEEDSORTSTRATEGY_H

#include "inetworksortstrategy.h"

class SpeedSortStrategy : public INetworkSortStrategy
{
    Q_OBJECT
    Q_INTERFACES(INetworkSortStrategy)
public:
    Q_INVOKABLE explicit SpeedSortStrategy(QObject* parent = nullptr);
    void sort(QList<NetworkInfo*>& networks) override;
    int findBestNetwork(QList<NetworkInfo*>& networks) override;
};

#endif // SPEEDSORTSTRATEGY_H
