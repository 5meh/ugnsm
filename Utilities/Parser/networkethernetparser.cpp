#include "networkethernetparser.h"
#include "networkinfo.h"

#include <QDateTime>

NetworkEthernetParser::NetworkEthernetParser()
{

}

void NetworkEthernetParser::parse()
{
    QList<NetworkInfo*> results;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for(const QNetworkInterface& interface: interfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            parseInterface(interface, results);
        }
    }

    if(!results.isEmpty())
    {
        emit parsingCompleted(QVariant::fromValue(results));
    }
    else
    {
        emit parsingFailed("No active Ethernet interfaces found");
    }
}

void NetworkEthernetParser::parseInterface(const QNetworkInterface& interface, QList<NetworkInfo*>& results)
{
    NetworkInfo* info = new NetworkInfo(
        interface.humanReadableName(),
        interface.hardwareAddress(),
        interface.flags().testFlag(QNetworkInterface::IsUp),
        interface.flags().testFlag(QNetworkInterface::IsRunning),
        QDateTime::currentDateTime(),
        this
        );

    info->setIpv4(getIPv4Address(interface));
    info->setNetmask(getNetmask(interface));
    info->setBroadcast(getBroadcast(interface));

    results.append(info);
}

QString NetworkEthernetParser::getIPv4Address(const QNetworkInterface& interface) const
{
    for(const QNetworkAddressEntry& entry: interface.addressEntries())
    {
        if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            return entry.ip().toString();
        }
    }
    return QString();

}

QString NetworkEthernetParser::getNetmask(const QNetworkInterface& interface) const
{
    for(const QNetworkAddressEntry& entry: interface.addressEntries())
    {
        if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            return entry.netmask().toString();
        }
    }
    return QString();
}

QString NetworkEthernetParser::getBroadcast(const QNetworkInterface& interface) const
{
    for(const QNetworkAddressEntry& entry: interface.addressEntries())
    {
        if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            return entry.broadcast().toString();
        }
    }
    return QString();
}

bool NetworkEthernetParser::validate(QVariant& result, QStringList& warnings)
{
    auto networks = result.value<QList<NetworkInfo*>>();
    bool isValid = true;

    for(NetworkInfo* info: networks)
    {
        if(info->getMac().isEmpty())
        {
            warnings << "Interface with empty MAC found: " + info->getName();
            isValid = false;
        }
        if(info->getIpv4().isEmpty())
        {
            warnings << "No IPv4 for interface: " + info->getName();
            isValid = false;
        }
    }

    return isValid;
}
