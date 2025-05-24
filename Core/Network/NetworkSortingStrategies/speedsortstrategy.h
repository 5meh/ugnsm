#ifndef SPEEDSORTSTRATEGY_H
#define SPEEDSORTSTRATEGY_H

#include "inetworksortstrategy.h"

class SpeedSortStrategy : public INetworkSortStrategy
{
    Q_OBJECT
    Q_INTERFACES(INetworkSortStrategy)
public:
    Q_INVOKABLE explicit SpeedSortStrategy(QObject* parent = nullptr);
    void sort(QList<NetworkInfoPtr>& networks) override;
    int findBestNetwork(const QList<NetworkInfoPtr>& networks) override;
};

#endif // SPEEDSORTSTRATEGY_H
