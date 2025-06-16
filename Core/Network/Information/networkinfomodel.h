#ifndef NETWORKINFOMODEL_H
#define NETWORKINFOMODEL_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QReadWriteLock>
#include "networkinfo.h"

class NetworkInfoModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString mac READ getMac NOTIFY macChanged)
    Q_PROPERTY(QString ipAddress READ getIpAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(QString netmask READ getNetmask NOTIFY netmaskChanged)
    Q_PROPERTY(QString downloadSpeed READ getDownloadSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString uploadSpeed READ getUploadSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString totalSpeed READ getTotalSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(QString lastUpdate READ getLastUpdate NOTIFY timestampChanged)

public:
    explicit NetworkInfoModel(NetworkInfoPtr model, QObject* parent = nullptr);

    const QHash<QString, QString>& propertyMap() const { return m_propertyMap; }//TODO:mb remove
    QList<QPair<QString, QString>> getAllKeyValuesAsList() const;
    QPair<QString, QString> getKeyValue(const QString& key) const;
    QStringList changedProperties() const;
    void clearChangedProperties();
    void updateFromNetworkInfo(NetworkInfoPtr newInfo);

    NetworkInfoPtr getNetworkInfo() const { return m_model; }

    QString getName() const;
    QString getMac() const;
    QString getIpAddress() const;
    QString getNetmask() const;
    QString getDownloadSpeed() const;
    QString getUploadSpeed() const;
    QString getTotalSpeed() const;
    QString getStatus() const;
    QString getLastUpdate() const;

public slots:
    void updateSpeeds(quint64 rx, quint64 tx);

signals:
    void propertyChanged(const QString& propertyName);
    void propertiesChanged(const QStringList& propertiesName);

    void nameChanged(const QString& name);
    void macChanged(const QString& mac);
    void ipAddressChanged(const QString& ip);
    void netmaskChanged(const QString& netmask);
    void speedChanged();
    void statusChanged();
    void timestampChanged();

private:
    void connectModelSignals();
    void markPropertyChanged(const QString& property);
    void onUnitsChanged();
    void flushPropertyChanges();
    QString formatTimestamp() const;
    QString formatSpeed(quint64 bytesPerSec) const;

    QTimer m_updateTimer;  // Add batched update timer
    const int UPDATE_THRESHOLD = 3;  // Batch when 3+ properties change
    NetworkInfoPtr m_model;
    QStringList m_changedProperties;
    const QHash<QString, QString> m_propertyMap =
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
    mutable QReadWriteLock m_rw_lock;
    mutable QMutex m_mutex;
};

#endif // NETWORKINFOMODEL_H
