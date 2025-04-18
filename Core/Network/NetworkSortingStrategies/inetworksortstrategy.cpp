#include "inetworksortstrategy.h"

#include "speedsortstrategy.h"
#include "stabilitysortstrategy.h"

INetworkSortStrategy::INetworkSortStrategy(QObject *parent)
    : QObject{parent}
{}

INetworkSortStrategy *INetworkSortStrategy::create(SortStrategyType type, QObject *parent)
{
    switch(type)
    {
    case SortStrategyType::Speed:
        return new SpeedSortStrategy(parent);
    case SortStrategyType::Stability:
        return new StabilitySortStrategy(parent);
    default:
        return new SpeedSortStrategy(parent);
    }
}
