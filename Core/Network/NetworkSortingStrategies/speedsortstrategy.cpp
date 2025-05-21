#include "speedsortstrategy.h"

#include "Network/Information/networkinfo.h"

SpeedSortStrategy::SpeedSortStrategy(QObject* parent)
    : INetworkSortStrategy{parent}
{

}

void SpeedSortStrategy::sort(QList<NetworkInfoPtr>& networks)
{
    std::sort(networks.begin(), networks.end(),
              [](NetworkInfoPtr a, NetworkInfoPtr b)
              {
                  return a.data()->getTotalSpeed() > b.data()->getTotalSpeed();
              });
}

int SpeedSortStrategy::findBestNetwork(QList<NetworkInfoPtr>& networks)
{
    if (networks.isEmpty())
        return -1;

    int bestIndex = 0;
    for (int i = 1; i < networks.size(); ++i)
    {
        if (networks[i].data()->getTotalSpeed() > networks[bestIndex].data()->getTotalSpeed())
        {
            bestIndex = i;
        }
    }

    return bestIndex;
}
