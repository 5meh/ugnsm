#include "networkinfoview.h"
#include "networkinfo.h"
//#include "networkspeedmonitor.h"

#include <QTimer>
#include <QNetworkInterface>
#include <QFile>

NetworkInfoView::NetworkInfoView(QObject* parent)
    :QObject(parent),
    netSpeedUpdateTimer(new QTimer(this)),
    m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &NetworkInfoView::refresh);
    connect(netSpeedUpdateTimer, &QTimer::timeout, this, &NetworkInfoView::updateSpeeds);
    getAllAvailableNetworksInfo();
    netSpeedUpdateTimer->start(1000);
    m_timer->start(5000);    
}

void NetworkInfoView::getAllAvailableNetworksInfo()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for(const auto &interface : interfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            createOrUpdateInfo(interface, interface.hardwareAddress());
        }
    }
}

void NetworkInfoView::refresh()
{
    const QList<QNetworkInterface> qtInterfaces = QNetworkInterface::allInterfaces();
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

void NetworkInfoView::updateSpeeds()
{
    for(const QPair<QString, NetworkInfo*> pair: m_networkInfos.asKeyValueRange())
    {
        const QString& interfaceName = pair.first;
        NetworkInfo& data = *pair.second;

        quint64 currentRx = 0, currentTx = 0;
        if(!getInterfaceStats(interfaceName, currentRx, currentTx))
        {
            emit errorOccurred(interfaceName, "Failed to get interface stats");
            continue;
        }

        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

        if(data.getLastUpdateTime() == 0)
        {
            data.setLastRxBytes(currentRx);
            data.setLastTxBytes(currentTx);
            data.setLastUpdateTime(currentTime);
            continue;
        }

        qint64 timeDelta = currentTime - data.getLastUpdateTime();

        if(timeDelta > 0)
        {
            quint64 rxDelta = (currentRx >= data.getLastRxBytes())
            ? (currentRx - data.getLastRxBytes())
            : (ULONG_LONG_MAX - data.getLastRxBytes() + currentRx + 1);

            quint64 txDelta = (currentTx >= data.getLastTxBytes())
                                  ? (currentTx - data.getLastTxBytes())
                                  : (ULONG_LONG_MAX - data.getLastTxBytes() + currentTx + 1);

            quint64 rxSpeed = (rxDelta * 1000) / timeDelta;
            quint64 txSpeed = (txDelta * 1000) / timeDelta;

            double rxMbps = (rxSpeed * 8.0) / 1'000'000.0;
            double txMbps = (rxSpeed * 8.0) / 1'000'000.0;

            data.setRxSpeed(rxSpeed);
            data.setTxSpeed(txSpeed);
            //emit speedUpdated(interfaceName, rxSpeed, txSpeed);

            qInfo() << interfaceName
                    << "| RX:" << rxSpeed << "B/s (" << (rxSpeed * 8 / 1'000'000.0) << "Mbps)"
                    << "| TX:" << txSpeed << "B/s (" << (txSpeed * 8 / 1'000'000.0) << "Mbps)";
        }

        data.setLastRxBytes(currentRx);
        data.setLastTxBytes(currentTx);
        data.setLastUpdateTime(currentTime);
    }
}

QString NetworkInfoView::selectBestInterface()
{
    QString bestInterfaceMac;
    quint64 maxTotal = 0;

    for(const QPair<QString, NetworkInfo*>& pair : m_networkInfos.asKeyValueRange())
    {
        quint64 total = pair.second->getLastRxBytes() + pair.second->getLastTxBytes();
        if (total > maxTotal)
        {
            maxTotal = total;
            bestInterfaceMac = pair.first;
        }
    }
    return bestInterfaceMac;
}

NetworkInfo *NetworkInfoView::createOrUpdateInfo(const QNetworkInterface& interface, const QString& mac)
{
    NetworkInfo* info = m_networkInfos.value(mac, nullptr);
    const QDateTime now = QDateTime::currentDateTime();
    const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
    const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

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
        info = new NetworkInfo(
            interface.humanReadableName(),
            mac,
            isUp,
            now,
            this
            );

        if(hasIpv4)
        {
            info->setIpv4(ipv4);
            info->setNetmask(netmask);
            info->setBroadcast(broadcast);
        }
        info->setIsRunning(isRunning);

        quint64 rx, tx;
        if(!getInterfaceStats(interface.humanReadableName(), rx, tx))
        {
            qWarning()<<"Some error";
        }

        info->setLastRxBytes(rx);
        info->setLastTxBytes(tx);
        info->setLastUpdateTime(QDateTime::currentMSecsSinceEpoch());

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

        if(info->isRunning() != isRunning)
        {
            info->setIsRunning(isRunning);
            changed = true;
        }

        if(hasIpv4)
        {
            if(info->getIpv4() != ipv4)
            {
                info->setIpv4(ipv4);
                changed = true;
            }
            if(info->getNetmask() != netmask)
            {
                info->setNetmask(netmask);
                changed = true;
            }
            if(info->getBroadcast() != broadcast)
            {
                info->setBroadcast(broadcast);
                changed = true;
            }
        }

        quint64 rx, tx;
        if(getInterfaceStats(interface.humanReadableName(), rx, tx))
        {
            if(info->getLastRxBytes() != rx)
            {
                info->setLastRxBytes(rx);
                changed = true;
            }
            if(info->getLastTxBytes() != tx)
            {
                info->setLastTxBytes(tx);
                changed = true;
            }
        }

        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if(info->getLastUpdateTime() != currentTime)
        {
            info->setLastUpdateTime(currentTime);
            changed = true;
        }

        info->setTimestamp(now);

        if(changed)
        {
            emit networkInfoUpdated(info);
        }
    }
    return info;
}

void NetworkInfoView::checkRemovedInfo(const QHash<QString, NetworkInfo *>& current)
{
    QSet<QString> removed;
    for(auto it = m_networkInfos.begin(); it != m_networkInfos.end(); ++it)
    {
        if(!current.contains(it.key()))
        {
            removed.insert(it.key());
        }
    }

    for(const QString &mac: removed)
    {
        NetworkInfo* info = m_networkInfos.take(mac);
        emit networkInfoRemoved(mac);
        info->deleteLater();
    }
}

#ifdef Q_OS_LINUX
bool NetworkInfoView::getInterfaceStats(const QString& interfaceName,
                                        quint64& rxBytes,
                                        quint64& txBytes)
{
    QFile rxFile("/sys/class/net/" + interfaceName + "/statistics/rx_bytes");
    QFile txFile("/sys/class/net/" + interfaceName + "/statistics/tx_bytes");

    if(!rxFile.open(QIODevice::ReadOnly) || !txFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    rxBytes = rxFile.readAll().trimmed().toULongLong();
    txBytes = txFile.readAll().trimmed().toULongLong();
    return true;
}
#elif defined(Q_OS_WIN)
bool NetworkInfoView::getInterfaceStats(const QString &interfaceName,
                                        quint64 &rxBytes,
                                        quint64 &txBytes)
{
    PMIB_IF_TABLE2 pIfTable = nullptr;
    if(GetIfTable2Ex(MibIfTableNormal, &pIfTable) != NO_ERROR)
    {
        return false;
    }

    bool found = false;
    for(ULONG i = 0; i < pIfTable->NumEntries; i++)
    {
        if(QString::fromWCharArray(pIfTable->Table[i].Alias) == interfaceName)
        {
            rxBytes = pIfTable->Table[i].InOctets;
            txBytes = pIfTable->Table[i].OutOctets;
            found = true;
            break;
        }
    }

    FreeMibTable(pIfTable);
    return found;
}
#else
bool NetworkInfoView::getInterfaceStats(const QString &, quint64 &, quint64 &)
{
    return false; // Unsupported platform
}
#endif
