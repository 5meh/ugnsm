#include "networkmonitor.h"
#include "../globalmanager.h"
#include "../TaskSystem/taskscheduler.h"
#include "../../../Utilities/Logger/logger.h"

#include <QNetworkInterface>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include <cmath>

#ifdef Q_OS_WIN
#  include <winsock2.h>
#  include <iphlpapi.h>
#  pragma comment(lib, "iphlpapi.lib")
#elif defined(Q_OS_MAC)
#  include <sys/sysctl.h>
#  include <net/if.h>
#  include <net/if_dl.h>
#endif

NetworkMonitor::NetworkMonitor(QObject* parent)
    : QObject(parent),
    m_interval(1000),
    m_running(0),
    m_initialized(false)
{
    Logger::instance().log(Logger::Debug, "NetworkMonitor instance created", "NetworkMonitor");
}

QString NetworkMonitor::normalizeMac(const QString& raw)
{
    QString formatted = raw.toLower().remove(':');
    Logger::instance().log(Logger::Trace,
                           QString("Normalized MAC: %1 -> %2").arg(raw).arg(formatted),
                           "NetworkMonitor");
    return formatted;
}

void NetworkMonitor::startMonitoring(int intervalMs)
{
    if (m_running.testAndSetOrdered(0,1))
    {
        m_interval = intervalMs;
        GlobalManager::taskScheduler()->cancelRepeating("network_monitoring");
        GlobalManager::taskScheduler()->scheduleRepeating("network_monitoring",
                                                          m_interval, this, &NetworkMonitor::refreshStats);
        Logger::instance().log(Logger::Info,
                               QString("Network monitoring started (interval %1 ms)").arg(m_interval),
                               "NetworkMonitor");
    }
}

void NetworkMonitor::stopMonitoring()
{
    if (m_running.testAndSetOrdered(1,0))
    {
        GlobalManager::taskScheduler()->cancelRepeating("network_monitoring");
        Logger::instance().log(Logger::Info,
                               "Network monitoring stopped",
                               "NetworkMonitor");
    }
}

void NetworkMonitor::initializeStats(const QSet<QString>& macs)
{
    QMutexLocker locker(&m_interfaceMutex);
    Logger::instance().log(Logger::Info,
                           QString("Initializing stats for %1 MACs").arg(macs.size()),
                           "NetworkMonitor");

    m_trackedMacs.clear();
    for (auto raw : macs)
    {
        QString normalized = normalizeMac(raw);
        m_trackedMacs.insert(normalized);
    }

    m_previousStats.clear();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (auto mac : m_trackedMacs)
    {
        InterfaceStats s;
        s.lastUpdate = now;
        s.initialized = false;
        m_previousStats.insert(mac, s);
        Logger::instance().log(Logger::Debug,
                               QString("Tracking MAC: %1").arg(mac),
                               "NetworkMonitor");
    }
    m_initialized = true;
}

void NetworkMonitor::updateTrackedMacs(const QSet<QString>& macs)
{
    QMutexLocker locker(&m_interfaceMutex);
    QSet<QString> newSet;
    for (QString raw : macs)
        newSet.insert(normalizeMac(raw));

    // Log changes
    QSet<QString> added = newSet - m_trackedMacs;
    QSet<QString> removed = m_trackedMacs - newSet;

    if (!added.isEmpty())
        Logger::instance().log(Logger::Info,
                               QString("Adding %1 new MACs: %2").arg(added.size()).arg(added.values().join(", ")),
                               "NetworkMonitor");
    if (!removed.isEmpty())
        Logger::instance().log(Logger::Info,
                               QString("Removing %1 MACs: %2").arg(removed.size()).arg(removed.values().join(", ")),
                               "NetworkMonitor");

    // add any new
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (QString mac : newSet)
    {
        if (!m_previousStats.contains(mac))
        {
            m_previousStats[mac] = InterfaceStats{0,0,now};
            Logger::instance().log(Logger::Debug,
                                   QString("Added new MAC: %1").arg(mac),
                                   "NetworkMonitor");
        }
    }
    // remove any old
    for (auto it = m_previousStats.begin(); it != m_previousStats.end(); )
    {
        if (!newSet.contains(it.key()))
        {
            Logger::instance().log(Logger::Debug,
                                   QString("Removing MAC: %1").arg(it.key()),
                                   "NetworkMonitor");
            it = m_previousStats.erase(it);
        }
        else
            ++it;
    }
    m_trackedMacs = std::move(newSet);
}

void NetworkMonitor::refreshStats()
{
    if (!m_running.loadAcquire())
    {
        Logger::instance().log(Logger::Debug, "Skipping refresh - monitoring not running", "NetworkMonitor");
        return;
    }

    if (!m_initialized)
    {
        Logger::instance().log(Logger::Debug, "Skipping refresh - not initialized", "NetworkMonitor");
        return;
    }

    if (m_trackedMacs.isEmpty())
    {
        Logger::instance().log(Logger::Debug, "Skipping refresh - no MACs to track", "NetworkMonitor");
        return;
    }

    QMutexLocker locker(&m_interfaceMutex);
    QHash<QString, InterfaceStats> rawStats;
    if (!readRawInterfaceStats(rawStats))
    {
        Logger::instance().log(Logger::Warning, "Failed to read raw interface stats", "NetworkMonitor");
        return;
    }

    Logger::instance().log(Logger::Debug,
                           QString("Read stats for %1 interfaces").arg(rawStats.size()),
                           "NetworkMonitor");

    // Log all raw stats for debugging
    for (auto it = rawStats.constBegin(); it != rawStats.constEnd(); ++it)
    {
        Logger::instance().log(Logger::Trace,
                               QString("Raw Interface: %1 RX: %2 TX: %3")
                                   .arg(it.key())
                                   .arg(it.value().rxBytes)
                                   .arg(it.value().txBytes),
                               "NetworkMonitor");
    }

    // Create direct mapping from interface names to MACs
    QHash<QString, QString> interfaceToMac;
    QHash<QString, QString> macToInterface;

    // First, get MAC addresses from system interfaces
    const QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& interface : allInterfaces)
    {
        QString ifaceName = interface.name();
        QString mac = normalizeMac(interface.hardwareAddress());

        if (!mac.isEmpty() && rawStats.contains(ifaceName))
        {
            interfaceToMac[ifaceName] = mac;
            macToInterface[mac] = ifaceName;
            Logger::instance().log(Logger::Debug,
                                   QString("Mapped interface %1 to MAC %2")
                                       .arg(ifaceName).arg(mac),
                                   "NetworkMonitor");
        }
    }

    // Build per-MAC current readings
    QHash<QString, InterfaceStats> current;
    for (auto it = rawStats.constBegin(); it != rawStats.constEnd(); ++it)
    {
        QString ifaceName = it.key();
        if (interfaceToMac.contains(ifaceName))
        {
            QString mac = interfaceToMac[ifaceName];
            current[mac] = it.value();

            Logger::instance().log(Logger::Debug,
                                   QString("Assigned stats for %1 to MAC %2: RX=%3 TX=%4")
                                       .arg(ifaceName).arg(mac)
                                       .arg(it.value().rxBytes).arg(it.value().txBytes),
                                   "NetworkMonitor");
        }
        else
        {
            Logger::instance().log(Logger::Debug,
                                   QString("No mapping for interface: %1").arg(ifaceName),
                                   "NetworkMonitor");
        }
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    int updatedCount = 0;
    const quint64 MAX_COUNTER = std::numeric_limits<quint64>::max();

    // Compute delta for each tracked MAC
    for (QString mac : m_trackedMacs)
    {
        if (!current.contains(mac))
        {
            Logger::instance().log(Logger::Debug,
                                   QString("No current stats for MAC: %1").arg(mac),
                                   "NetworkMonitor");
            continue;
        }

        if (!m_previousStats.contains(mac))
        {
            Logger::instance().log(Logger::Debug,
                                   QString("Initializing new MAC: %1").arg(mac),
                                   "NetworkMonitor");
            // Initialize with current values
            m_previousStats[mac] = current[mac];
            m_previousStats[mac].lastUpdate = now;
            m_previousStats[mac].initialized = false;
            continue;
        }

        auto& prev = m_previousStats[mac];
        auto& cur  = current[mac];
        qint64 dt = now - prev.lastUpdate;

        // Always update previous stats with new byte counts
        quint64 prevRx = prev.rxBytes;
        quint64 prevTx = prev.txBytes;
        prev.rxBytes = cur.rxBytes;
        prev.txBytes = cur.txBytes;
        prev.lastUpdate = now;

        // Skip invalid time intervals
        if (dt <= 0 || dt > 5000)
        {
            Logger::instance().log(Logger::Debug,
                                   QString("Skipping MAC %1 due to invalid dt: %2ms").arg(mac).arg(dt),
                                   "NetworkMonitor");
            continue;
        }

        // Skip calculation until we have two readings
        if (!prev.initialized)
        {
            Logger::instance().log(Logger::Debug,
                                   QString("Initial reading for MAC %1 - marking as initialized").arg(mac),
                                   "NetworkMonitor");
            prev.initialized = true;
            continue;
        }

        // Handle counter wrap-around
        quint64 rx_diff = (cur.rxBytes >= prevRx)
                              ? (cur.rxBytes - prevRx)
                              : (MAX_COUNTER - prevRx + cur.rxBytes);

        quint64 tx_diff = (cur.txBytes >= prevTx)
                              ? (cur.txBytes - prevTx)
                              : (MAX_COUNTER - prevTx + cur.txBytes);

        // Calculate speeds in BITS per second (not bytes!)
        quint64 rx_bps = (rx_diff * 8 * 1000) / dt;
        quint64 tx_bps = (tx_diff * 8 * 1000) / dt;

        Logger::instance().log(Logger::Debug,
                               QString("MAC: %1 DT: %2ms RX: %3 bps TX: %4 bps (Diff: RX %5 TX %6)")
                                   .arg(mac)
                                   .arg(dt)
                                   .arg(rx_bps)
                                   .arg(tx_bps)
                                   .arg(rx_diff)
                                   .arg(tx_diff),
                               "NetworkMonitor");

        emit statsUpdated(mac, rx_bps, tx_bps);
        updatedCount++;
    }

    Logger::instance().log(Logger::Info,
                           QString("Updated %1/%2 interfaces").arg(updatedCount).arg(m_trackedMacs.size()),
                           "NetworkMonitor");
}

bool NetworkMonitor::readRawInterfaceStats(QHash<QString, InterfaceStats>& stats)
{
    static QMutex readLock;
    QMutexLocker locker(&readLock);

    Logger::instance().log(Logger::Debug, "Reading raw interface stats", "NetworkMonitor");

#if defined(Q_OS_WIN)
    PMIB_IF_TABLE2 table = nullptr;
    DWORD result = GetIfTable2Ex(MibIfTableNormal, &table);

    if (result != NO_ERROR)
    {
        Logger::instance().log(Logger::Error,
                               QString("GetIfTable2Ex failed with error: %1").arg(result),
                               "NetworkMonitor");
        return false;
    }

    for (ULONG i = 0; i < table->NumEntries; ++i)
    {
        MIB_IF_ROW2* row = &table->Table[i];
        QString name = QString::fromWCharArray(row->Description);

        stats[name].rxBytes = row->InOctets;
        stats[name].txBytes = row->OutOctets;

        Logger::instance().log(Logger::Trace,
                               QString("Interface: %1 RX: %2 TX: %3")
                                   .arg(name)
                                   .arg(row->InOctets)
                                   .arg(row->OutOctets),
                               "NetworkMonitor");
    }

    FreeMibTable(table);
    return true;

#elif defined(Q_OS_MAC)
    int mib[] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0};
    size_t len = 0;

    if (sysctl(mib, 6, nullptr, &len, nullptr, 0) < 0)
    {
        Logger::instance().log(Logger::Error,
                               "sysctl failed to get buffer size", "NetworkMonitor");
        return false;
    }

    QByteArray buf(static_cast<int>(len), 0);
    if (sysctl(mib, 6, buf.data(), &len, nullptr, 0) < 0)
    {
        Logger::instance().log(Logger::Error,
                               "sysctl failed to read interface data", "NetworkMonitor");
        return false;
    }

    const char* ptr = buf.constData();
    const char* end = ptr + len;

    while (ptr < end)
    {
        const if_msghdr* ifm = reinterpret_cast<const if_msghdr*>(ptr);

        if (ifm->ifm_type == RTM_IFINFO2)
        {
            const if_msghdr2* if2 = reinterpret_cast<const if_msghdr2*>(ptr);
            const sockaddr_dl* sdl = reinterpret_cast<const sockaddr_dl*>(ifm + 1);

            if (sdl->sdl_family == AF_LINK)
            {
                QString name = QString::fromLatin1(sdl->sdl_data, sdl->sdl_nlen);
                stats[name].rxBytes = if2->ifm_data.ifi_ibytes;
                stats[name].txBytes = if2->ifm_data.ifi_obytes;

                Logger::instance().log(Logger::Trace,
                                       QString("Interface: %1 RX: %2 TX: %3")
                                           .arg(name)
                                           .arg(if2->ifm_data.ifi_ibytes)
                                           .arg(if2->ifm_data.ifi_obytes),
                                       "NetworkMonitor");
            }
        }
        ptr += ifm->ifm_msglen;
    }
    return true;

#elif defined(Q_OS_LINUX)
    QFile f("/proc/net/dev");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::instance().log(Logger::Error,
                               "Failed to open /proc/net/dev", "NetworkMonitor");
        return false;
    }

    QTextStream in(&f);
    in.readLine(); // Skip header
    in.readLine(); // Skip divider

    int validCount = 0;
    int skippedCount = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // Find the interface name (ends with colon)
        int colonPos = line.indexOf(':');
        if (colonPos < 0)
        {
            skippedCount++;
            continue;
        }

        QString iface = line.left(colonPos).trimmed();
        QString data = line.mid(colonPos + 1).trimmed();

        // Split data columns
        auto parts = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (parts.size() < 16)
        {
            Logger::instance().log(Logger::Warning,
                                   QString("Skipping interface %1 with only %2 columns")
                                       .arg(iface)
                                       .arg(parts.size()),
                                   "NetworkMonitor");
            skippedCount++;
            continue;
        }

        bool rxOk, txOk;
        quint64 rxBytes = parts[0].toULongLong(&rxOk);
        quint64 txBytes = parts[8].toULongLong(&txOk);

        if (!rxOk || !txOk)
        {
            Logger::instance().log(Logger::Warning,
                                   QString("Invalid numbers for interface %1: RX=%2 TX=%3")
                                       .arg(iface)
                                       .arg(parts[0])
                                       .arg(parts[8]),
                                   "NetworkMonitor");
            skippedCount++;
            continue;
        }

        stats[iface].rxBytes = rxBytes;
        stats[iface].txBytes = txBytes;
        validCount++;

        Logger::instance().log(Logger::Trace,
                               QString("Interface: %1 RX: %2 TX: %3")
                                   .arg(iface)
                                   .arg(rxBytes)
                                   .arg(txBytes),
                               "NetworkMonitor");
    }

    Logger::instance().log(Logger::Debug,
                           QString("Parsed %1 interfaces, skipped %2").arg(validCount).arg(skippedCount),
                           "NetworkMonitor");
    return true;

#else
    Logger::instance().log(Logger::Warning, "Unsupported platform", "NetworkMonitor");
    return false;
#endif
}
