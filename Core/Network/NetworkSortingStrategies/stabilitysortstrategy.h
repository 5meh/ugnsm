#ifndef STABILITYSORTSTRATEGY_H
#define STABILITYSORTSTRATEGY_H

#include "inetworksortstrategy.h"

class StabilitySortStrategy : public INetworkSortStrategy
{
    Q_OBJECT
    Q_INTERFACES(INetworkSortStrategy)
public:
    Q_INVOKABLE explicit StabilitySortStrategy(QObject *parent = nullptr);
    void sort(QList<NetworkInfo *> &networks) override;
};

#endif // STABILITYSORTSTRATEGY_H
