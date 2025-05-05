#include "networkmonitor.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "../../../UI/Components/Grid/GridCellWidgets/networkinfoviewwidget.h"
#include "../TaskSystem/taskscheduler.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#elif defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#elif defined(Q_OS_LINUX)
// Linux implementation
#endif

NetworkMonitor::NetworkMonitor(TaskScheduler* scheduler, QObject* parent)
    :m_scheduler(scheduler),
    QObject(parent)
{
}

void NetworkMonitor::startMonitoring(int intervalMs)
{
    m_interval = intervalMs;
    m_running.storeRelease(1);
}

void NetworkMonitor::stopMonitoring()
{
    //m_timer.stop();
}

void NetworkMonitor::refreshStats()
{
    QHash<QString, InterfaceStats> currentStats;
    if(getInterfaceStats(currentStats))
    {
        calculateSpeeds(currentStats);
    }
}

void NetworkMonitor::monitoringLoop()
{

}

bool NetworkMonitor::getInterfaceStats(QHash<QString, InterfaceStats>& currentStats)
{
    if(!readRawInterfaceStats(currentStats))
    {
        return false;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for(auto& stats : currentStats)
    {
        stats.lastUpdate = now;
    }
    return true;
}

void NetworkMonitor::calculateSpeeds(const QHash<QString, InterfaceStats>& currentStats)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    for(auto it = currentStats.constBegin(); it != currentStats.constEnd(); ++it)
    {
        const QString& interface = it.key();
        const InterfaceStats& current = it.value();

        if(m_previousStats.contains(interface))
        {
            const InterfaceStats& previous = m_previousStats[interface];
            qint64 timeDelta = now - previous.lastUpdate;

            if(timeDelta > 0)
            {
                quint64 rxSpeed = (current.rxBytes - previous.rxBytes) * 1000 / timeDelta;
                quint64 txSpeed = (current.txBytes - previous.txBytes) * 1000 / timeDelta;

                emit statsUpdated(interface, rxSpeed, txSpeed);
            }
        }
    }

    m_previousStats = currentStats;
}

// Platform-specific implementations
bool NetworkMonitor::readRawInterfaceStats(QHash<QString, InterfaceStats>& stats)
{
#ifdef Q_OS_WIN
    PMIB_IF_TABLE2 ifTable;
    if(GetIfTable2Ex(MibIfTableNormal, &ifTable) != NO_ERROR)
        return false;

    for(ULONG i = 0; i < ifTable->NumEntries; i++)
    {
        MIB_IF_ROW2* ifRow = &ifTable->Table[i];
        QString name = QString::fromWCharArray(ifRow->Description);

        stats[name].rxBytes = ifRow->InOctets;
        stats[name].txBytes = ifRow->OutOctets;
    }

    FreeMibTable(ifTable);
    return true;

#elif defined(Q_OS_MAC)
    int mib[] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0};
    size_t len;
    if(sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
        return false;

    QByteArray buf(len, 0);
    if(sysctl(mib, 6, buf.data(), &len, NULL, 0) < 0)
        return false;

    char* ptr = buf.data();
    char* end = ptr + len;

    while(ptr < end)
    {
        struct if_msghdr* ifm = (struct if_msghdr*)ptr;
        if(ifm->ifm_type == RTM_IFINFO2)
        {
            struct if_msghdr2* if2m = (struct if_msghdr2*)ifm;
            QString name = QString::fromUtf8(if2m->ifm_data.ifi_name);

            stats[name].rxBytes = if2m->ifm_data.ifi_ibytes;
            stats[name].txBytes = if2m->ifm_data.ifi_obytes;
        }
        ptr += ifm->ifm_msglen;
    }
    return true;

#elif defined(Q_OS_LINUX)
    QFile file("/proc/net/dev");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    in.readLine(); // Skip header
    in.readLine();

    while(!in.atEnd())
    {
        QString line = in.readLine().simplified();
        QStringList parts = line.split(' ');

        if(parts.size() < 10) continue;

        QString interface = parts[0].replace(":", "");
        quint64 rxBytes = parts[1].toULongLong();
        quint64 txBytes = parts[9].toULongLong();

        stats[interface].rxBytes = rxBytes;
        stats[interface].txBytes = txBytes;
    }
    return true;

#else
    return false;
#endif
}
