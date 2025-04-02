#include "networkinfoviewmodel.h"

#include "networkinfo.h"

NetworkInfoViewModel::NetworkInfoViewModel(NetworkInfo *model, QObject *parent)
    :QObject(parent), m_model(model)
{
    connectModelSignals();
}

QList<QPair<QString, QString> > NetworkInfoViewModel::getAllKeyValuesAsList() const
{
    QList<QPair<QString, QString>> list;
    list.append(qMakePair("Interface:", getName()));
    list.append(qMakePair("MAC:", getMac()));
    list.append(qMakePair("IPV4:", getIpAddress()));
    list.append(qMakePair("Netmask:", getNetmask()));
    list.append(qMakePair("Is Up:", QString(m_model->getIsUp() ? "True" : "False")));
    list.append(qMakePair("Is Running:", QString(true ? "True" : "False")));
    list.append(qMakePair("Date/time:", m_model->getTimestamp().toString()));
    list.append(qMakePair("Last Rx bytes:", QString::number(m_model->getLastRxBytes())));
    list.append(qMakePair("Last Tx bytes:", QString::number(m_model->getLastTxBytes())));
    list.append(qMakePair("Rx speed:", QString::number(m_model->getRxSpeed())));
    list.append(qMakePair("Tx speed:", QString::number(m_model->getTxSpeed())));

    return list;
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

QString NetworkInfoViewModel::getDownloadSpeed() const
{
    return formatSpeed(m_model->getRxSpeed());
}

QString NetworkInfoViewModel::getUploadSpeed() const
{
    return formatSpeed(m_model->getTxSpeed());
}

QDateTime NetworkInfoViewModel::getTimestamp() const
{
    return m_model->getTimestamp();
}

qint64 NetworkInfoViewModel::getLastUpdateTime() const
{
    return m_model->getLastUpdateTime();
}

void NetworkInfoViewModel::updateFromModel()
{
    //emit nameChanged();
    //emit ipAddressChanged();
}

void NetworkInfoViewModel::updateSpeeds(quint64 rx, quint64 tx)
{
    m_model->setRxSpeed(rx);
    m_model->setTxSpeed(tx);
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
