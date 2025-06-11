#include "networkinfomodel.h"

#include "globalmanager.h"

NetworkInfoModel::NetworkInfoModel(NetworkInfoPtr model, QObject *parent)
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

void NetworkInfoModel::updateFromNetworkInfo(NetworkInfoPtr newInfo)
{
    if(!m_model || !newInfo)
        return;

    m_model->blockSignals(true);
    m_model->updateFrom(newInfo.get());
    m_model->blockSignals(false);

    const QStringList allProperties =
        {
            "name",
            "mac",
            "ipAddress",
            "netmask",
            "status",
            "downloadSpeed",
            "uploadSpeed",
            "totalSpeed",
            "lastUpdate"
        };

    //emit propertyChanged(allProperties);

    //delete newInfo;//TODO:mb remove and use std::move
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
    QString unitSetting = GlobalManager::settingsManager()->getDataUnits();
    int precision = GlobalManager::settingsManager()->getDecimalPrecision();
    double value = bytes;
    QString unit;

    if (unitSetting == "Bytes")
        unit = "B";
    else if (unitSetting == "KB")
    {
        unit = "KB";
        value /= 1024.0;
    }
    else if (unitSetting == "MB")
    {
        unit = "MB";
        value /= (1024.0 * 1024.0);
    }
    else if (unitSetting == "GB")
    {
        unit = "GB";
        value /= (1024.0 * 1024.0 * 1024.0);
    }
    else
    {
        // Fallback to automatic scaling for unknown units
        const QStringList units = {"B", "KB", "MB", "GB"};
        int unitIndex = 0;
        double speed = bytes;

        while (speed >= 1024 && unitIndex < units.size() - 1)
        {
            speed /= 1024;
            unitIndex++;
        }
        // Use settings precision for non-byte units
        int displayPrecision = (unitIndex > 0) ? precision : 0;
        return QString("%1 %2").arg(speed, 0, 'f', displayPrecision).arg(units[unitIndex]);
    }

    return QString("%1 %2").arg(value, 0, 'f', precision).arg(unit);
}

void NetworkInfoModel::connectModelSignals()
{
    auto connectProperty = [this](const QString& property, auto signal)
    {
        connect(m_model.data(), signal, this, [this, property]()
                {
                    markPropertyChanged(property);
                });
    };

    connectProperty("name", &NetworkInfo::nameChanged);
    connectProperty("mac", &NetworkInfo::macChanged);
    connectProperty("ipAddress", &NetworkInfo::ipv4Changed);
    connectProperty("netmask", &NetworkInfo::netmaskChanged);
    connectProperty("status", &NetworkInfo::isUpChanged);

    connect(m_model.data(), &NetworkInfo::rxSpeedChanged, this, [this]()
            {
                markPropertyChanged("downloadSpeed");
                markPropertyChanged("totalSpeed");
            });

    connect(m_model.data(), &NetworkInfo::txSpeedChanged, this, [this]()
            {
                markPropertyChanged("uploadSpeed");
                markPropertyChanged("totalSpeed");
            });

    connect(m_model.data(), &NetworkInfo::lastUpdateTimeChanged, this, [this]()
            {
                markPropertyChanged("lastUpdate");
            });
}

void NetworkInfoModel::markPropertyChanged(const QString& property)
{
    if(!m_changedProperties.contains(property))
        m_changedProperties.append(property);
        //emit propertyChanged(property);
}
