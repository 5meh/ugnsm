#ifndef SPEEDSORTSTRATEGY_H
#define SPEEDSORTSTRATEGY_H

#include "inetworksortstrategy.h"

class SpeedSortStrategy : public INetworkSortStrategy
{
public:
    explicit SpeedSortStrategy(QObject* parent = nullptr);
    void sort(QList<NetworkInfo*>& networks) override;
};

#endif // SPEEDSORTSTRATEGY_H
