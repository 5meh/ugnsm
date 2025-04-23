#include "networkethernetparser.h"

#include "../Core/Network/NetworkSortingStrategies/speedsortstrategy.h"
#include "../Core/Network/Information/networkinfo.h"

NetworkEthernetParser::NetworkEthernetParser(QObject* parent)
    : IParser(parent)
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
        emit parsingCompleted(result);
    }
    else
    {
        qWarning() << "Validation failed:" << warnings;
        emit parsingFailed(warnings.join("; "));
        qDeleteAll(results); // Cleanup invalid data
    }
}

void NetworkEthernetParser::parseInterface(const QNetworkInterface& interface, QList<NetworkInfo*>& results)
{
    NetworkInfo* info = new NetworkInfo(this);
    info->setMac(interface.hardwareAddress());
    const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
    const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

    info->setIsUp(isUp);
    info->setIsRunning(isRunning);

    bool hasIpv4 = false;
    const QList<QNetworkAddressEntry> entries = interface.addressEntries();


    for(const QNetworkAddressEntry &entry : entries)
    {

        if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            hasIpv4 = true;

            info->setIpv4(entry.ip().toString());
            //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("IPv4:", entry.ip().toString()));
            info->setIpv4(entry.netmask().toString());
            //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("Netmask:", entry.netmask().toString()));
            info->setIpv4(entry.broadcast().toString());
            //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("Broadcast:", entry.broadcast().toString()));

            // qInfo() << "  IPv4:" << entry.ip().toString();
            // qInfo() << "  Netmask:" << entry.netmask().toString();
            // qInfo() << "  Broadcast:" << entry.broadcast().toString();
        }
    }

    // if(isUp && isRunning && hasIpv4)//TODO:move to NetworkInfoModel
    // {
    //     ethernetConnected = true;
    //     //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("ethernet Connected:", ethernetConnected ? "True" : "False"));
    //     //qInfo() << "  --> ACTIVE ETHERNET CONNECTION DETECTED";
    // }

    //qInfo() << "----------------------------------------";
    results.append(info);
}

bool NetworkEthernetParser::validate(QVariant& result, QStringList& warnings)
{
    QList<NetworkInfo*> networks = result.value<QList<NetworkInfo*>>();
    QList<NetworkInfo*> validNetworks;
    bool hasValid = false;

    for(NetworkInfo* info : networks)
    {
        bool isValid = true;
        if(info->getMac().isEmpty())
        {
            warnings << "Invalid MAC for " + info->getName();
            isValid = false;
        }
        if(info->getIpv4().isEmpty())
        {
            warnings << "Missing IPv4 for " + info->getName();
            isValid = false;
        }

        if(isValid)
        {
            validNetworks.append(info);
            hasValid = true;
        }
        else
        {
            delete info;
        }
    }

    result.setValue(validNetworks);

    return hasValid;
}
