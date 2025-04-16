#include "networkinfomodel.h"
#include "networkinfo.h"

NetworkInfoModel::NetworkInfoModel(NetworkInfo *model, QObject *parent)
    : QObject(parent),
    m_model(model)
{
    connectModelSignals();
}

QList<QPair<QString, QString>> NetworkInfoModel::getAllKeyValuesAsList() const
{
    QList<QPair<QString, QString>> list;
    list.append({"Interface", getName()});
    list.append({"MAC Address", getMac()});
    list.append({"IP Address", getIpAddress()});
    list.append({"Netmask", getNetmask()});
    list.append({"Status", getStatus()});
    list.append({"Download Speed", getDownloadSpeed()});
    list.append({"Upload Speed", getUploadSpeed()});
    list.append({"Total Speed", getTotalSpeed()});
    list.append({"Last Update", getLastUpdate()});
    return list;
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
    emit speedChanged();
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
    connect(m_model, &NetworkInfo::nameChanged, this, &NetworkInfoModel::nameChanged);
    connect(m_model, &NetworkInfo::macChanged, this, &NetworkInfoModel::macChanged);
    connect(m_model, &NetworkInfo::ipv4Changed, this, [this]()
            {
                emit ipAddressChanged(getIpAddress());
            });
    connect(m_model, &NetworkInfo::netmaskChanged, this, [this]()
            {
                emit netmaskChanged(getNetmask());
            });
    connect(m_model, &NetworkInfo::rxSpeedChanged, this, &NetworkInfoModel::speedChanged);
    connect(m_model, &NetworkInfo::txSpeedChanged, this, &NetworkInfoModel::speedChanged);
    connect(m_model, &NetworkInfo::lastUpdateTimeChanged, this, &NetworkInfoModel::timestampChanged);
    connect(m_model, &NetworkInfo::isUpChanged, this, &NetworkInfoModel::statusChanged);
}
