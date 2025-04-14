#include "stabilitysortstrategy.h"

#include "Network/Information/networkinfo.h"

StabilitySortStrategy::StabilitySortStrategy(QObject *parent)
    : INetworkSortStrategy{parent}
{

}

void StabilitySortStrategy::sort(QList<NetworkInfo*>& networks)
{
    std::sort(networks.begin(), networks.end(),
              [](NetworkInfo* a, NetworkInfo* b)
              {
                  return a->getTimestamp() > b->getTimestamp();//TODO:mb change rework timesettings in NetworkInfo
              });
}
