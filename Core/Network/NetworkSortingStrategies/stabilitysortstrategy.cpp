#include "stabilitysortstrategy.h"

#include "Network/Information/networkinfo.h"

StabilitySortStrategy::StabilitySortStrategy(QObject* parent)
    : INetworkSortStrategy{parent}
{

}

void StabilitySortStrategy::sort(QList<NetworkInfoPtr>& networks)
{
    std::sort(networks.begin(), networks.end(),
              [](NetworkInfoPtr a, NetworkInfoPtr b)
              {
                  return a.data()->getTimestamp() > b.data()->getTimestamp();//TODO:mb change rework timesettings in NetworkInfo
              });
}

int StabilitySortStrategy::findBestNetwork(const QList<NetworkInfoPtr>& networks)
{
    return -1;
}
