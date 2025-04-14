#include "speedsortstrategy.h"

#include "Network/Information/networkinfo.h"

SpeedSortStrategy::SpeedSortStrategy(QObject* parent)
    : INetworkSortStrategy{parent}
{

}

void SpeedSortStrategy::sort(QList<NetworkInfo*>& networks)
{
    std::sort(networks.begin(), networks.end(),
              [](NetworkInfo* a, NetworkInfo* b)
              {
                  return a->getTotalSpeed() > b->getTotalSpeed();
              });
}
