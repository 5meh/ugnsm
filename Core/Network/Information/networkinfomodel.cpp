#include "networkinfomodel.h"

#include "globalmanager.h"

#include <QReadLocker>

NetworkInfoModel::NetworkInfoModel(NetworkInfoPtr model, QObject *parent)
    : QObject(parent),
    m_model(model)
{
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(50);  // Max 50ms delay for batched updates
    connectModelSignals();
}

QList<QPair<QString, QString>> NetworkInfoModel::getAllKeyValuesAsList() const
{
    QReadLocker lock(&m_rw_lock);
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
    m_model->setRxSpeed(rx);
    m_model->setTxSpeed(tx);
    m_model->setLastUpdateTime(QDateTime::currentMSecsSinceEpoch());
    markPropertyChanged("downloadSpeed");
    markPropertyChanged("uploadSpeed");
    markPropertyChanged("totalSpeed");
    markPropertyChanged("lastUpdate");
    flushPropertyChanges();
}

QString NetworkInfoModel::formatTimestamp() const
{
    return QDateTime::fromMSecsSinceEpoch(m_model->getLastUpdateTime())
    .toString("hh:mm:ss.zzz");
}

QString NetworkInfoModel::formatSpeed(quint64 bytesPerSec) const
{
    SettingsManager* settings = GlobalManager::settingsManager();
    QString unitSetting = settings->getDataUnits();
    int precision = settings->getDecimalPrecision();

    double value = 0;
    QString unit;

    if (unitSetting == "KB")
    {
        value = bytesPerSec / 1024.0;
        unit = "KB";
    }
    else if (unitSetting == "MB")
    {
        value = bytesPerSec / (1024.0 * 1024.0);
        unit = "MB";
    }
    else if (unitSetting == "GB")
    {
        value = bytesPerSec / (1024.0 * 1024.0 * 1024.0);
        unit = "GB";
    }
    else
    {
        value = static_cast<double>(bytesPerSec);
        unit = "B";
    }

    // Handle very small values
    if (value < 1 && value > 0)
    {
        // Switch to lower unit
        if (unit == "GB" && value > 0.001)
        {
            value *= 1024;
            unit = "MB";
        }
        else if (unit == "MB" && value > 0.001)
        {
            value *= 1024;
            unit = "KB";
        }
        else if (unit == "KB" && value > 0.1)
        {
            value *= 1024;
            unit = "B";
        }
    }

    // Determine precision dynamically for small values
    int actualPrecision = precision;
    if (value < 1.0 && value > 0.0)
    {
        actualPrecision = qMin(4, precision + 2);
    }
    else if (value < 10.0)
    {
        actualPrecision = qMin(3, precision + 1);
    }

    return QString::number(value, 'f', actualPrecision) + " " + unit;
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
    connect(&m_updateTimer, &QTimer::timeout,
            this, &NetworkInfoModel::flushPropertyChanges);
}

void NetworkInfoModel::markPropertyChanged(const QString& property)
{
    if(!m_changedProperties.contains(property))
    {
        m_changedProperties.append(property);

        // Trigger immediate update for speed-related properties
        if (property == "downloadSpeed" ||
            property == "uploadSpeed" ||
            property == "totalSpeed")
        {
            flushPropertyChanges();
        }
        // Batch other properties when threshold reached or timer expires
        else if (m_changedProperties.size() >= UPDATE_THRESHOLD)
            flushPropertyChanges();
        else
            m_updateTimer.start();
    }
}

void NetworkInfoModel::flushPropertyChanges()
{
    if (!m_changedProperties.isEmpty())
    {
        const auto changed = m_changedProperties;
        m_changedProperties.clear();

        QMetaObject::invokeMethod(this, [this, changed]() {
            emit propertiesChanged(changed);
        }, Qt::QueuedConnection);
    }
    m_updateTimer.stop();
}
