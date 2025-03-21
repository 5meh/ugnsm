#include "networkinfoview.h"
#include "networkinfo.h"

#include <QTimer>
#include <QNetworkInterface>

NetworkInfoView::NetworkInfoView(QObject* parent)
    :QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &NetworkInfoView::refresh);
    m_timer->start(5000);
}

void NetworkInfoView::refresh()
{
    const auto qtInterfaces = QNetworkInterface::allInterfaces();
    QHash<QString, NetworkInfo*> currentInterfaces;

    for(const auto& interface : qtInterfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet)
        {
            const QString mac = interface.hardwareAddress();
            currentInterfaces[mac] = createOrUpdateInfo(interface, mac);
        }
    }

    checkRemovedInfo(currentInterfaces);
}

NetworkInfo *NetworkInfoView::createOrUpdateInfo(const QNetworkInterface &interface, const QString &mac)
{
    qDebug() << "Processing interface:" << interface.name()
             << "MAC:" << mac;
    //<< "Exists:" << (info ? "Yes" : "No");
    NetworkInfo* info = m_networkInfos.value(mac, nullptr);
    const QDateTime now = QDateTime::currentDateTime();
    const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
    const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

    // Get network address information
    QString ipv4, netmask, broadcast;
    bool hasIpv4 = false;

    const QList<QNetworkAddressEntry> entries = interface.addressEntries();
    for(const QNetworkAddressEntry &entry : entries)
    {
        if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            hasIpv4 = true;
            ipv4 = entry.ip().toString();
            netmask = entry.netmask().toString();
            broadcast = entry.broadcast().toString();
            break; // Use first IPv4 entry
        }
    }

    if(!info)
    {
        // Create new NetworkInfo
        info = new NetworkInfo(
            interface.humanReadableName(),
            mac,
            5,  // Use actual speed
            isUp,
            now,
            this
            );

        // Set additional network info
        if(hasIpv4)
        {
            info->setIpv4(ipv4);
            info->setNetmask(netmask);
            info->setBroadcast(broadcast);
        }
        info->setIsRunning(isRunning);

        m_networkInfos.insert(mac, info);
        emit networkInfoAdded(info);
    }
    else
    {
        bool changed = false;

        if(info->getName() != interface.humanReadableName())
        {
            info->setName(interface.humanReadableName());
            changed = true;
        }

        if(info->getIsUp() != isUp)
        {
            info->setIsUp(isUp);
            changed = true;
        }

        // Check and update speed
        // if(info->getSpeed() != interface.speed()) {
        //     info->setSpeed(interface.speed());
        //     changed = true;
        // }

        if(info->isRunning() != isRunning)
        {
            info->setIsRunning(isRunning);
            changed = true;
        }

        if(hasIpv4)
        {
            if(info->ipv4() != ipv4)
            {
                info->setIpv4(ipv4);
                changed = true;
            }
            if(info->netmask() != netmask)
            {
                info->setNetmask(netmask);
                changed = true;
            }
            if(info->broadcast() != broadcast)
            {
                info->setBroadcast(broadcast);
                changed = true;
            }
        }

        info->setTimestamp(now);

        if(changed)
        {
            emit networkInfoUpdated(info);
        }
    }
    return info;
}

void NetworkInfoView::checkRemovedInfo(const QHash<QString, NetworkInfo *> &current)
{
    QSet<QString> removed;
    for(auto it = m_networkInfos.begin(); it != m_networkInfos.end(); ++it)
    {
        if(!current.contains(it.key()))
        {
            removed.insert(it.key());
        }
    }

    for(const QString &mac : removed)
    {
        NetworkInfo* info = m_networkInfos.take(mac);
        emit networkInfoRemoved(mac);
        info->deleteLater();
    }
}
