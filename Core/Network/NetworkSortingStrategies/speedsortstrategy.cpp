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

int SpeedSortStrategy::findBestNetwork(QList<NetworkInfo*>& networks)
{
    if (networks.isEmpty())
        return -1;

    int bestIndex = 0;
    for (int i = 1; i < networks.size(); ++i)
    {
        if (networks[i]->getTotalSpeed() > networks[bestIndex]->getTotalSpeed())
        {
            bestIndex = i;
        }
    }

    return bestIndex;
}
