#include "networkinfomodel.h"
#include "networkinfo.h"

NetworkInfoModel::NetworkInfoModel(NetworkInfo *model, QObject *parent)
    : QObject(parent),
    m_model(model)
{
    m_propertyMap =
        {
            {"name", "Interface"},
            {"mac", "MAC Address"},
            {"ipAddress", "IP Address"},
            {"netmask", "Netmask"},
            {"status", "Status"},
            {"downloadSpeed", "Download Speed"},
            {"uploadSpeed", "Upload Speed"},
            {"totalSpeed", "Total Speed"},
            {"lastUpdate", "Last Update"}
        };

    connectModelSignals();
}

QList<QPair<QString, QString>> NetworkInfoModel::getAllKeyValuesAsList() const
{
    return
        {
            {m_propertyMap["name"], getName()},
            {m_propertyMap["mac"], getMac()},
            {m_propertyMap["ipAddress"], getIpAddress()},
            {m_propertyMap["netmask"], getNetmask()},
            {m_propertyMap["status"], getStatus()},
            {m_propertyMap["downloadSpeed"], getDownloadSpeed()},
            {m_propertyMap["uploadSpeed"], getUploadSpeed()},
            {m_propertyMap["totalSpeed"], getTotalSpeed()},
            {m_propertyMap["lastUpdate"], getLastUpdate()}
        };
}

QPair<QString, QString> NetworkInfoModel::getKeyValue(const QString &key) const
{
    if(key == m_propertyMap["name"])
        return {key, getName()};
    if(key == m_propertyMap["mac"])
        return {key, getMac()};
    if(key == m_propertyMap["ipAddress"])
        return {key, getIpAddress()};
    if(key == m_propertyMap["netmask"])
        return {key, getNetmask()};
    if(key == m_propertyMap["status"])
        return {key, getStatus()};
    if(key == m_propertyMap["downloadSpeed"])
        return {key, getDownloadSpeed()};
    if(key == m_propertyMap["uploadSpeed"])
        return {key, getUploadSpeed()};
    if(key == m_propertyMap["totalSpeed"])
        return {key, getTotalSpeed()};
    if(key == m_propertyMap["lastUpdate"])
        return {key, getLastUpdate()};
    return {QString(), QString()};
}

QStringList NetworkInfoModel::changedProperties() const
{
    return m_changedProperties;
}

void NetworkInfoModel::clearChangedProperties()
{
    m_changedProperties.clear();
}

void NetworkInfoModel::updateFromNetworkInfo(NetworkInfo* newInfo)
{
    if(!m_model || !newInfo)
        return;

    m_model->updateFrom(newInfo);

    delete newInfo;//TODO:mb remove and use std::move
}

QString NetworkInfoModel::getName() const
{
    return m_model->getName();
}

QString NetworkInfoModel::getMac() const
{
    return m_model->getMac();
}

QString NetworkInfoModel::getIpAddress() const
{
    return m_model->getIpv4();
}

QString NetworkInfoModel::getNetmask() const
{
    return m_model->getNetmask();
}

QString NetworkInfoModel::getDownloadSpeed() const
{
    return QString("%1/s").arg(formatSpeed(m_model->getRxSpeed()));
}

QString NetworkInfoModel::getUploadSpeed() const
{
    return QString("%1/s").arg(formatSpeed(m_model->getTxSpeed()));
}

QString NetworkInfoModel::getTotalSpeed() const
{
    return QString("%1/s").arg(formatSpeed(m_model->getTotalSpeed()));
}

QString NetworkInfoModel::getStatus() const
{
    return m_model->getIsUp() ? "Connected" : "Disconnected";
}

QString NetworkInfoModel::getLastUpdate() const
{
    return formatTimestamp();
}

void NetworkInfoModel::updateSpeeds(quint64 rx, quint64 tx)
{
    m_model->setRxSpeed(static_cast<qint64>(rx));
    m_model->setTxSpeed(static_cast<qint64>(tx));
    m_model->setLastUpdateTime(QDateTime::currentMSecsSinceEpoch());
    markPropertyChanged("downloadSpeed");
    markPropertyChanged("uploadSpeed");
    markPropertyChanged("totalSpeed");
    markPropertyChanged("lastUpdate");
}

QString NetworkInfoModel::formatTimestamp() const
{
    return QDateTime::fromMSecsSinceEpoch(m_model->getLastUpdateTime())
    .toString("hh:mm:ss.zzz");
}

QString NetworkInfoModel::formatSpeed(quint64 bytes) const
{
    const QStringList units = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double speed = bytes;

    while (speed >= 1024 && unitIndex < units.size() - 1)
    {
        speed /= 1024;
        unitIndex++;
    }
    return QString("%1 %2").arg(speed, 0, 'f', unitIndex > 0 ? 2 : 0).arg(units[unitIndex]);
}

void NetworkInfoModel::connectModelSignals()
{
    auto connectProperty = [this](const QString& property, auto signal)
    {
        connect(m_model, signal, this, [this, property]()
                {
                    markPropertyChanged(property);
                });
    };

    connectProperty("name", &NetworkInfo::nameChanged);
    connectProperty("mac", &NetworkInfo::macChanged);
    connectProperty("ipAddress", &NetworkInfo::ipv4Changed);
    connectProperty("netmask", &NetworkInfo::netmaskChanged);
    connectProperty("status", &NetworkInfo::isUpChanged);

    connect(m_model, &NetworkInfo::rxSpeedChanged, this, [this]()
            {
                markPropertyChanged("downloadSpeed");
                markPropertyChanged("totalSpeed");
            });

    connect(m_model, &NetworkInfo::txSpeedChanged, this, [this]()
            {
                markPropertyChanged("uploadSpeed");
                markPropertyChanged("totalSpeed");
            });

    connect(m_model, &NetworkInfo::lastUpdateTimeChanged, this, [this]()
            {
                markPropertyChanged("lastUpdate");
            });
}

void NetworkInfoModel::markPropertyChanged(const QString &property)
{
    if(!m_changedProperties.contains(property))
    {
        m_changedProperties.append(property);
        emit propertyChanged(property);
    }
}
