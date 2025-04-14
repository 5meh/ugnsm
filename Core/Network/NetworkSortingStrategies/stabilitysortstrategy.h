#ifndef STABILITYSORTSTRATEGY_H
#define STABILITYSORTSTRATEGY_H

#include "inetworksortstrategy.h"

class StabilitySortStrategy : public INetworkSortStrategy
{
public:
    explicit StabilitySortStrategy(QObject *parent = nullptr);
    void sort(QList<NetworkInfo *> &networks) override;
};

#endif // STABILITYSORTSTRATEGY_H
