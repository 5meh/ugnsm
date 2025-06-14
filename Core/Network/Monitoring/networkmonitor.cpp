#include "networkmonitor.h"

#include "../globalmanager.h"

#include <QFile>
#include <QTextStream>
#include <QNetworkInterface>
#include <QDateTime>
#include "../../../UI/Components/Grid/GridCellWidgets/networkinfoviewwidget.h"
#include "../TaskSystem/taskscheduler.h"
#include "../../../Utilities/Logger/logger.h"

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

NetworkMonitor::NetworkMonitor(QObject* parent)
    : QObject(parent)
{
}

void NetworkMonitor::startMonitoring(int intervalMs)
{
    m_interval = intervalMs;
    GlobalManager::taskScheduler()->scheduleRepeating("network_monitoring", m_interval, this,
                                                      &NetworkMonitor::refreshStats,
                                                      QThread::NormalPriority);
    Logger::instance().log(Logger::Info,
                           QString("Network monitoring started (interval %1ms)").arg(m_interval), "Network");
}

void NetworkMonitor::stopMonitoring()
{
    m_running = 0;
}

void NetworkMonitor::initializeStats(const QSet<QString> &macs)
{
    m_trackedMacs = macs;
    m_previousStats.clear();

    // Get initial stats for all interfaces
    QHash<QString, InterfaceStats> currentStats;
    if (readRawInterfaceStats(currentStats))
    {
        qint64 now = QDateTime::currentMSecsSinceEpoch();

        // Initialize all tracked MACs with zero stats
        for (const QString& mac : m_trackedMacs)
        {
            InterfaceStats stats;
            stats.lastUpdate = now;
            m_previousStats[mac] = stats;
        }
        m_initialized = true;
    }
}

void NetworkMonitor::updateTrackedMacs(const QSet<QString> &macs)
{
    for (const QString& mac : macs)
    {
        if (!m_trackedMacs.contains(mac))
        {
            // Initialize with zero stats
            InterfaceStats stats;
            stats.lastUpdate = QDateTime::currentMSecsSinceEpoch();
            m_previousStats[mac] = stats;
        }
    }

    // Remove untracked MACs
    for (auto it = m_previousStats.begin(); it != m_previousStats.end(); )
    {
        if (!macs.contains(it.key()))
            it = m_previousStats.erase(it);
        else
            ++it;
    }

    m_trackedMacs = macs;
}

void NetworkMonitor::refreshStats()
{
    if (!m_running.loadAcquire() || !m_initialized || m_trackedMacs.isEmpty())
    {
        Logger::instance().log(Logger::Debug, "Skipping refresh - monitor not running", "Network");
        return;
    }

    QHash<QString, InterfaceStats> interfaceStats;
    if (!readRawInterfaceStats(interfaceStats))
        return;

    // Convert to MAC-based stats using Qt's interface list
    QHash<QString, InterfaceStats> macStats;
    for (const QNetworkInterface& interface: QNetworkInterface::allInterfaces())
    {
        QString mac = interface.hardwareAddress();
        QString name = interface.name();

        if (interfaceStats.contains(name) && m_trackedMacs.contains(mac))
        {
            macStats[mac] = interfaceStats[name];
        }
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Calculate speeds for each tracked MAC
    for (const QString& mac : m_trackedMacs)
    {
        if (macStats.contains(mac) && m_previousStats.contains(mac))
        {
            const InterfaceStats& current = macStats[mac];
            const InterfaceStats& previous = m_previousStats[mac];
            qint64 timeDelta = now - previous.lastUpdate;

            if (timeDelta > 0)
            {
                quint64 rxSpeed = (current.rxBytes - previous.rxBytes) * 1000 / timeDelta;
                quint64 txSpeed = (current.txBytes - previous.txBytes) * 1000 / timeDelta;

                // Update previous stats with current data
                InterfaceStats updated = current;
                updated.lastUpdate = now;
                m_previousStats[mac] = updated;

                emit statsUpdated(mac, rxSpeed, txSpeed);
            }
        }
    }

    Logger::instance().log(Logger::Debug,
                           QString("Updated stats for %1 interfaces").arg(macStats.size()),
                           "Network");
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
    static QMutex readMutex;  // Add static mutex for thread safety
    QMutexLocker locker(&readMutex);
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
