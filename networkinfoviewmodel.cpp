#include "networkinfoviewmodel.h"

#include "networkinfo.h"

NetworkInfoViewModel::NetworkInfoViewModel(NetworkInfo *model, QObject *parent)
    :QObject(parent), m_model(model)
{
    connectModelSignals();
}

QList<QPair<QString, QString> > NetworkInfoViewModel::getAllKeyValuesAsList() const
{
    return m_model->getAllKeyValuesAsList();
}

QString NetworkInfoViewModel::getName() const
{
    return m_model->getName();
}

QString NetworkInfoViewModel::getMac() const
{
    return m_model->getMac();
}

QString NetworkInfoViewModel::getIpAddress() const
{
    return m_model->getIpv4();
}

QString NetworkInfoViewModel::getNetmask() const
{
    return m_model->getNetmask();
}

quint64 NetworkInfoViewModel::getDownloadSpeed() const
{
    return m_model->getRxSpeed();
}

quint64 NetworkInfoViewModel::getUploadSpeed() const
{
    return m_model->getTxSpeed();
}

QString NetworkInfoViewModel::formatSpeed(quint64 bytes) const
{
    const QStringList units = {"B/s", "KB/s", "MB/s", "GB/s"};
    int unitIndex = 0;
    double speed = bytes;

    while (speed >= 1024 && unitIndex < units.size() - 1)
    {
        speed /= 1024;
        unitIndex++;
    }

    return QString("%1 %2").arg(speed, 0, 'f', unitIndex > 0 ? 2 : 0).arg(units[unitIndex]);
}

void NetworkInfoViewModel::connectModelSignals()
{
    connect(m_model, &NetworkInfo::nameChanged, this, [this](const QString& name)
            {
                emit nameChanged(name);
            });

    connect(m_model, &NetworkInfo::macChanged, this, [this](const QString& mac)
            {
                emit macChanged(mac);
            });

    connect(m_model, &NetworkInfo::ipv4Changed, this, [this]()
            {
                emit ipAddressChanged(m_model->getIpv4());
            });

    connect(m_model, &NetworkInfo::netmaskChanged, this, [this]()
            {
                emit netmaskChanged(m_model->getNetmask());
            });

    connect(m_model, &NetworkInfo::rxSpeedChanged, this, [this]()
            {
                emit speedChanged(m_model->getRxSpeed(), m_model->getTxSpeed());
            });

    connect(m_model, &NetworkInfo::txSpeedChanged, this, [this]()
            {
                emit speedChanged(m_model->getRxSpeed(), m_model->getTxSpeed());
            });
}
