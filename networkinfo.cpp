#include "networkinfo.h"

NetworkInfo::NetworkInfo(QObject *parent)
    : QObject{parent}
{}

NetworkInfo::NetworkInfo(const QString &name, const QString &mac, int speed, bool isUp, const QDateTime &timestamp, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_mac(mac)
    , m_speed(speed)
    , m_isUp(isUp)
    , m_timestamp(timestamp)
{

}

NetworkInfo::NetworkInfo(const NetworkInfo &obj)
    :QObject(obj.parent()),
    m_name(obj.m_name),
    m_mac(obj.m_mac),
    m_ipv4(obj.m_ipv4),
    m_netmask(obj.m_netmask),
    m_broadcast(obj.m_broadcast),
    m_speed(obj.m_speed),
    m_isUp(obj.m_isUp),
    m_isRunning(obj.m_isRunning),
    m_timestamp(obj.m_timestamp)
{

}

bool NetworkInfo::operator==(const NetworkInfo &other) const
{
    return m_name == other.m_name &&
           m_mac == other.m_mac &&
           m_speed == other.m_speed &&
           m_isUp == other.m_isUp &&
           m_timestamp == other.m_timestamp;
}

bool NetworkInfo::operator!=(const NetworkInfo &other) const
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

void NetworkInfo::setSpeed(int speed)
{
    if (m_speed != speed)
    {
        m_speed = speed;
        emit speedChanged(m_speed);
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

QString NetworkInfo::ipv4() const
{
    return m_ipv4;
}

void NetworkInfo::setIpv4(const QString &newIpv4)
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

QString NetworkInfo::netmask() const
{
    return m_netmask;
}

void NetworkInfo::setNetmask(const QString &newNetmask)
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

QString NetworkInfo::broadcast() const
{
    return m_broadcast;
}

void NetworkInfo::setBroadcast(const QString &newBroadcast)
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

QList<QPair<QString, QString> > NetworkInfo::getAllKeyValuesAsList()
{
    QList<QPair<QString, QString>> list;
    list.append({"Interface:", m_name});
    list.append({"MAC:", m_mac});
    list.append({"IPV4:", m_ipv4});
    list.append({"Broadcast:", m_broadcast});
    list.append(std::make_pair(QString("Is Up:"), QString(m_isUp ? "True" : "False")));
    list.append(std::make_pair(QString("Is Running:"), QString(m_isRunning ? "True" : "False")));
    list.append({"Date/time:", m_timestamp.toString()});
    return list;
}
