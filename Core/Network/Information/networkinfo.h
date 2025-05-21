#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>
#include <QDateTime>
#include <QSharedPointer>

class NetworkInfo : public QObject
{
    Q_OBJECT
public:
    explicit NetworkInfo(QObject* parent = nullptr);
    NetworkInfo(const QString& mac, QObject* parent = nullptr);
    NetworkInfo(const QString& name, const QString& mac, bool isUp, bool isRunning, const QDateTime& timestamp, QObject* parent = nullptr);
    NetworkInfo(const NetworkInfo& obj, QObject* parent = nullptr);
    ~NetworkInfo() = default;

    bool operator==(const NetworkInfo& other) const;
    bool operator!=(const NetworkInfo& other) const;

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString mac READ getMac WRITE setMac NOTIFY macChanged)
    Q_PROPERTY(bool isUp READ getIsUp WRITE setIsUp NOTIFY isUpChanged)
    Q_PROPERTY(QDateTime timestamp READ getTimestamp WRITE setTimestamp NOTIFY timestampChanged)
    Q_PROPERTY(QString ipv4 READ getIpv4 WRITE setIpv4 RESET resetIpv4 NOTIFY ipv4Changed FINAL)
    Q_PROPERTY(QString netmask READ getNetmask WRITE setNetmask RESET resetNetmask NOTIFY netmaskChanged FINAL)
    Q_PROPERTY(QString broadcast READ getBroadcast WRITE setBroadcast RESET resetBroadcast NOTIFY broadcastChanged FINAL)
    Q_PROPERTY(bool isRunning READ isRunning WRITE setIsRunning RESET resetIsRunning NOTIFY isRunningChanged FINAL)
    Q_PROPERTY(quint64 lastRxBytes READ getLastRxBytes WRITE setLastRxBytes RESET resetLastRxBytes NOTIFY lastRxBytesChanged FINAL)
    Q_PROPERTY(quint64 lastTxBytes READ getLastTxBytes WRITE setLastTxBytes RESET resetLastTxBytes NOTIFY lastTxBytesChanged FINAL)
    Q_PROPERTY(qint64 lastUpdateTime READ getLastUpdateTime WRITE setLastUpdateTime RESET resetLastUpdateTime NOTIFY lastUpdateTimeChanged FINAL)
    Q_PROPERTY(qint64 rxSpeed READ getRxSpeed WRITE setRxSpeed RESET resetRxSpeed NOTIFY rxSpeedChanged FINAL)
    Q_PROPERTY(qint64 txSpeed READ getTxSpeed WRITE setTxSpeed RESET resetTxSpeed NOTIFY txSpeedChanged FINAL)
    Q_PROPERTY(quint64 totalSpeed READ getTotalSpeed NOTIFY speedChanged)

    QString getName() const { return m_name; }
    QString getMac() const { return m_mac; }
    bool getIsUp() const { return m_isUp; }
    QDateTime getTimestamp() const { return m_timestamp; }
    QString getIpv4() const { return m_ipv4; }
    QString getNetmask() const { return m_netmask; }
    QString getBroadcast() const { return m_broadcast; }
    bool isRunning() const { return m_isRunning; }
    quint64 getLastRxBytes() const { return m_lastRxBytes; }
    quint64 getLastTxBytes() const { return m_lastTxBytes; }
    qint64 getRxSpeed() const { return m_rxSpeed; }
    qint64 getTxSpeed() const { return m_txSpeed; }
    quint64 getTotalSpeed() const { return static_cast<quint64>(m_rxSpeed + m_txSpeed); }
    qint64 getLastUpdateTime() const { return m_lastUpdateTime; }

    void setName(const QString &name);
    void setMac(const QString &mac);
    void setIsUp(bool isUp);
    void setTimestamp(const QDateTime &timestamp);
    void setIpv4(const QString& newIpv4);
    void setNetmask(const QString& newNetmask);
    void setBroadcast(const QString& newBroadcast);
    void setIsRunning(bool newIsRunning);
    void setLastRxBytes(quint64 newLastRxBytes);
    void setLastTxBytes(quint64 newLastTxBytes);
    void setRxSpeed(qint64 newRxSpeed);
    void setTxSpeed(qint64 newTxSpeed);
    void setLastUpdateTime(qint64 newLastUpdateTime);

    void resetIpv4();
    void resetNetmask();
    void resetBroadcast();
    void resetIsRunning();
    void resetLastRxBytes();
    void resetLastTxBytes();
    void resetRxSpeed();
    void resetTxSpeed();
    void resetLastUpdateTime();

    void updateFrom(const NetworkInfo* other);

signals:
    void nameChanged(const QString&);
    void macChanged(const QString&);
    void isUpChanged(bool);
    void timestampChanged(const QDateTime&);
    void ipv4Changed();
    void netmaskChanged();
    void broadcastChanged();
    void isRunningChanged();
    void lastRxBytesChanged();
    void lastTxBytesChanged();
    void lastUpdateTimeChanged();
    void rxSpeedChanged();
    void txSpeedChanged();
    void speedChanged();

private:
    QString m_name;
    QString m_mac;
    QString m_ipv4;
    QString m_netmask;
    QString m_broadcast;
    bool m_isUp;
    bool m_isRunning;
    QDateTime m_timestamp;
    quint64 m_lastRxBytes;
    quint64 m_lastTxBytes;
    qint64 m_rxSpeed;
    qint64 m_txSpeed;
    qint64 m_lastUpdateTime;
};

using NetworkInfoPtr = QSharedPointer<NetworkInfo>;

Q_DECLARE_METATYPE(NetworkInfo*)
Q_DECLARE_METATYPE(QList<NetworkInfo*>)
Q_DECLARE_METATYPE(NetworkInfoPtr)
Q_DECLARE_METATYPE(QList<NetworkInfoPtr>)

#endif // NETWORKINFO_H
