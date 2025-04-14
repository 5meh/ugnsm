#include "networkethernetparser.h"

#include "../Core/Network/NetworkSortingStrategies/speedsortstrategy.h"

NetworkEthernetParser::NetworkEthernetParser(INetworkSortStrategy* sorter, QObject* parent)
    : IParser(parent),
    m_sorter(new SpeedSortStrategy(this))
{

}

void NetworkEthernetParser::parse()
{
    QList<NetworkInfo*> results;
    QStringList warnings;

    const QList<QNetworkInterface>  interfaces = QNetworkInterface::allInterfaces();
    for(const QNetworkInterface& interface : interfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            parseInterface(interface, results);
        }
    }

    QVariant result = QVariant::fromValue(results);
    if(validate(result, warnings))
    {
        if(m_sorter)
            m_sorter->sort(results);
        emit parsingCompleted(result);
    }
    else
    {
        qWarning() << "Validation failed:" << warnings;
        emit parsingFailed(warnings.join("; "));
        qDeleteAll(results); // Cleanup invalid data
    }
}

bool NetworkEthernetParser::validate(QVariant& result, QStringList& warnings)
{
    QList<NetworkInfo*> networks = result.value<QList<NetworkInfo*>>();
    bool valid = true;

    for(NetworkInfo* info : networks)
    {
        if(info->getMac().isEmpty())
        {
            warnings << "Invalid MAC for " + info->getName();
            valid = false;
        }
        if(info->getIpv4().isEmpty())
        {
            warnings << "Missing IPv4 for " + info->getName();
            valid = false;
        }
    }

    return valid && !networks.isEmpty();
}
