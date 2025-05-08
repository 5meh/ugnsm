#include "networkinfo.h"

NetworkInfo::NetworkInfo(QObject* parent) : QObject(parent),
    m_isUp(false),
    m_isRunning(false),
    m_lastRxBytes(0),
    m_lastTxBytes(0),
    m_rxSpeed(0),
    m_txSpeed(0),
    m_lastUpdateTime(0) {}

NetworkInfo::NetworkInfo(const QString& mac, QObject* parent) : QObject(parent),
    m_mac(mac),
    m_isUp(false),
    m_isRunning(false),
    m_lastRxBytes(0),
    m_lastTxBytes(0),
    m_rxSpeed(0),
    m_txSpeed(0),
    m_lastUpdateTime(0) {}

NetworkInfo::NetworkInfo(const QString& name, const QString& mac, bool isUp, bool isRunning,
                         const QDateTime& timestamp, QObject* parent) : QObject(parent),
    m_name(name),
    m_mac(mac),
    m_isUp(isUp),
    m_isRunning(isRunning),
    m_timestamp(timestamp),
    m_lastRxBytes(0),
    m_lastTxBytes(0),
    m_rxSpeed(0),
    m_txSpeed(0),
    m_lastUpdateTime(0)
{
}

NetworkInfo::NetworkInfo(const NetworkInfo& obj, QObject* parent)
    : QObject(parent),
    m_name(obj.m_name),
    m_mac(obj.m_mac),
    m_ipv4(obj.m_ipv4),
    m_netmask(obj.m_netmask),
    m_broadcast(obj.m_broadcast),
    m_isUp(obj.m_isUp),
    m_isRunning(obj.m_isRunning),
    m_timestamp(obj.m_timestamp),
    m_lastRxBytes(obj.m_lastRxBytes),
    m_lastTxBytes(obj.m_lastTxBytes),
    m_rxSpeed(obj.m_rxSpeed),
    m_txSpeed(obj.m_txSpeed),
    m_lastUpdateTime(obj.m_lastUpdateTime)
{
    disconnect(this, 0, 0, 0);
}

bool NetworkInfo::operator==(const NetworkInfo& other) const
{
    return m_mac == other.m_mac;
}

bool NetworkInfo::operator!=(const NetworkInfo& other) const
{
    return !(*this == other);
}

void NetworkInfo::setName(const QString &name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged(m_name);
    }
}

void NetworkInfo::setMac(const QString &mac)
{
    if (m_mac != mac)
    {
        m_mac = mac;
        emit macChanged(m_mac);
    }
}

void NetworkInfo::setIsUp(bool isUp)
{
    if (m_isUp != isUp)
    {
        m_isUp = isUp;
        emit isUpChanged(m_isUp);
    }
}

void NetworkInfo::setTimestamp(const QDateTime &timestamp)
{
    if (m_timestamp != timestamp)
    {
        m_timestamp = timestamp;
        emit timestampChanged(m_timestamp);
    }
}

void NetworkInfo::setIpv4(const QString& newIpv4)
{
    if (m_ipv4 != newIpv4)
    {
        m_ipv4 = newIpv4;
        emit ipv4Changed();
    }
}

void NetworkInfo::setNetmask(const QString& newNetmask)
{
    if (m_netmask != newNetmask)
    {
        m_netmask = newNetmask;
        emit netmaskChanged();
    }
}

void NetworkInfo::setBroadcast(const QString& newBroadcast)
{
    if (m_broadcast != newBroadcast)
    {
        m_broadcast = newBroadcast;
        emit broadcastChanged();
    }
}

void NetworkInfo::setIsRunning(bool newIsRunning)
{
    if (m_isRunning != newIsRunning) {
        m_isRunning = newIsRunning;
        emit isRunningChanged();
    }
}

void NetworkInfo::setLastRxBytes(quint64 newLastRxBytes)
{
    if (m_lastRxBytes != newLastRxBytes)
    {
        m_lastRxBytes = newLastRxBytes;
        emit lastRxBytesChanged();
    }
}

void NetworkInfo::setLastTxBytes(quint64 newLastTxBytes)
{
    if (m_lastTxBytes != newLastTxBytes)
    {
        m_lastTxBytes = newLastTxBytes;
        emit lastTxBytesChanged();
    }
}

void NetworkInfo::setRxSpeed(qint64 newRxSpeed)
{
    if (m_rxSpeed != newRxSpeed)
    {
        m_rxSpeed = newRxSpeed;
        emit rxSpeedChanged();
        emit speedChanged();
    }
}

void NetworkInfo::setTxSpeed(qint64 newTxSpeed)
{
    if (m_txSpeed != newTxSpeed)
    {
        m_txSpeed = newTxSpeed;
        emit txSpeedChanged();
        emit speedChanged();
    }
}

void NetworkInfo::setLastUpdateTime(qint64 newLastUpdateTime)
{
    if (m_lastUpdateTime != newLastUpdateTime)
    {
        m_lastUpdateTime = newLastUpdateTime;
        emit lastUpdateTimeChanged();
    }
}

void NetworkInfo::resetIpv4()
{
    setIpv4("N/A");
}

void NetworkInfo::resetNetmask()
{
    setNetmask("N/A");
}

void NetworkInfo::resetBroadcast()
{
    setBroadcast("N/A");
}

void NetworkInfo::resetIsRunning()
{
    setIsRunning(false);
}

void NetworkInfo::resetLastRxBytes()
{
    setLastRxBytes(0);
}

void NetworkInfo::resetLastTxBytes()
{
    setLastTxBytes(0);
}

void NetworkInfo::resetRxSpeed()
{
    setRxSpeed(0);
}

void NetworkInfo::resetTxSpeed()
{
    setTxSpeed(0);
}

void NetworkInfo::resetLastUpdateTime()
{
    setLastUpdateTime(0);
}

void NetworkInfo::updateFrom(const NetworkInfo* other)
{
    if(!other)
        return;

    // Only update if values actually change
    setName(other->getName());
    setMac(other->getMac());
    setIsUp(other->getIsUp());
    setTimestamp(other->getTimestamp());
    setIpv4(other->getIpv4());
    setNetmask(other->getNetmask());
    setBroadcast(other->getBroadcast());
    setIsRunning(other->isRunning());
    setLastRxBytes(other->getLastRxBytes());
    setLastTxBytes(other->getLastTxBytes());
    setRxSpeed(other->getRxSpeed());
    setTxSpeed(other->getTxSpeed());
    setLastUpdateTime(other->getLastUpdateTime());
}
