#include "networkinfo.h"

NetworkInfo::NetworkInfo(QObject* parent)
    : QObject{parent}
{}

NetworkInfo::NetworkInfo(const QString& mac, QObject* parent)
    :m_mac(mac),
    QObject(parent)
{

}

NetworkInfo::NetworkInfo(const QString& name, const QString& mac, bool isUp, bool isRuning, const QDateTime& timestamp, QObject* parent)
    : QObject(parent),
    m_name(name),
    m_mac(mac),
    m_isUp(isUp),
    m_isRunning(isRuning),
    m_timestamp(timestamp)
{

}

NetworkInfo::NetworkInfo(const NetworkInfo& obj)
    :QObject(obj.parent()),
    m_name(obj.m_name),
    m_mac(obj.m_mac),
    m_ipv4(obj.m_ipv4),
    m_netmask(obj.m_netmask),
    m_broadcast(obj.m_broadcast),
    m_isUp(obj.m_isUp),
    m_isRunning(obj.m_isRunning),
    m_timestamp(obj.m_timestamp)
{

}

bool NetworkInfo::operator==(const NetworkInfo& other) const
{
    return m_mac == other.m_mac;
}

bool NetworkInfo::operator!=(const NetworkInfo& other) const
{
    return !(*this == other);
}

void NetworkInfo::setName(const QString& name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged(m_name);
    }
}

void NetworkInfo::setMac(const QString& mac)
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

void NetworkInfo::setTimestamp(const QDateTime& timestamp)
{
    if (m_timestamp != timestamp)
    {
        m_timestamp = timestamp;
        emit timestampChanged(m_timestamp);
    }
}

QString NetworkInfo::getIpv4() const
{
    return m_ipv4;
}

void NetworkInfo::setIpv4(const QString& newIpv4)
{
    if (m_ipv4 == newIpv4)
        return;
    m_ipv4 = newIpv4;
    emit ipv4Changed();
}

void NetworkInfo::resetIpv4()
{
    setIpv4({}); // TODO: Adapt to use your actual default value
}

QString NetworkInfo::getNetmask() const
{
    return m_netmask;
}

void NetworkInfo::setNetmask(const QString& newNetmask)
{
    if (m_netmask == newNetmask)
        return;
    m_netmask = newNetmask;
    emit netmaskChanged();
}

void NetworkInfo::resetNetmask()
{
    setNetmask({}); // TODO: Adapt to use your actual default value
}

QString NetworkInfo::getBroadcast() const
{
    return m_broadcast;
}

void NetworkInfo::setBroadcast(const QString& newBroadcast)
{
    if (m_broadcast == newBroadcast)
        return;
    m_broadcast = newBroadcast;
    emit broadcastChanged();
}

void NetworkInfo::resetBroadcast()
{
    setBroadcast({}); // TODO: Adapt to use your actual default value
}

bool NetworkInfo::isRunning() const
{
    return m_isRunning;
}

void NetworkInfo::setIsRunning(bool newIsRunning)
{
    if (m_isRunning == newIsRunning)
        return;
    m_isRunning = newIsRunning;
    emit isRunningChanged();
}

void NetworkInfo::resetIsRunning()
{
    setIsRunning({}); // TODO: Adapt to use your actual default value
}

QList<QPair<QString, QString> > NetworkInfo::getAllKeyValuesAsList() const
{
    QList<QPair<QString, QString>> list;
    list.append(qMakePair("Interface:", m_name));
    list.append(qMakePair("MAC:", m_mac));
    list.append(qMakePair("IPV4:", m_ipv4));
    list.append(qMakePair("Broadcast:", m_broadcast));
    list.append(qMakePair("Is Up:", QString(m_isUp ? "True" : "False")));
    list.append(qMakePair("Is Running:", QString(m_isRunning ? "True" : "False")));
    list.append(qMakePair("Date/time:", m_timestamp.toString()));
    list.append(qMakePair("Last Rx bytes:", QString::number(m_lastRxBytes)));
    list.append(qMakePair("Last Tx bytes:", QString::number(m_lastTxBytes)));
    list.append(qMakePair("Rx speed:", QString::number(m_rxSpeed)));
    list.append(qMakePair("Tx speed:", QString::number(m_txSpeed)));

    return list;
}

quint64 NetworkInfo::getLastRxBytes() const
{
    return m_lastRxBytes;
}

void NetworkInfo::setLastRxBytes(quint64 newLastRxBytes)
{
    if(m_lastRxBytes == newLastRxBytes)
        return;
    m_lastRxBytes = newLastRxBytes;
    emit lastRxBytesChanged();
}

void NetworkInfo::resetLastRxBytes()
{
    setLastRxBytes({}); // TODO: Adapt to use your actual default value
}

quint64 NetworkInfo::getLastTxBytes() const
{
    return m_lastTxBytes;
}

void NetworkInfo::setLastTxBytes(quint64 newLastTxBytes)
{
    if(m_lastTxBytes == newLastTxBytes)
        return;
    m_lastTxBytes = newLastTxBytes;
    emit lastTxBytesChanged();
}

void NetworkInfo::resetLastTxBytes()
{
    setLastTxBytes({}); // TODO: Adapt to use your actual default value
}

qint64 NetworkInfo::getLastUpdateTime() const
{
    return m_lastUpdateTime;
}

void NetworkInfo::setLastUpdateTime(qint64 newLastUpdateTime)
{
    if(m_lastUpdateTime == newLastUpdateTime)
        return;
    m_lastUpdateTime = newLastUpdateTime;
    emit lastUpdateTimeChanged();
}

void NetworkInfo::resetLastUpdateTime()
{
    setLastUpdateTime({}); // TODO: Adapt to use your actual default value
}

qint64 NetworkInfo::getRxSpeed() const
{
    return m_rxSpeed;
}

void NetworkInfo::setRxSpeed(qint64 newRxSpeed)
{
    if(m_rxSpeed == newRxSpeed)
        return;
    m_rxSpeed = newRxSpeed;
    emit rxSpeedChanged();
}

void NetworkInfo::resetRxSpeed()
{
    setRxSpeed({}); // TODO: Adapt to use your actual default value
}

qint64 NetworkInfo::getTxSpeed() const
{
    return m_txSpeed;
}

void NetworkInfo::setTxSpeed(qint64 newTxSpeed)
{
    if(m_txSpeed == newTxSpeed)
        return;
    m_txSpeed = newTxSpeed;
    emit txSpeedChanged();
}

void NetworkInfo::resetTxSpeed()
{
    setTxSpeed({}); // TODO: Adapt to use your actual default value
}
