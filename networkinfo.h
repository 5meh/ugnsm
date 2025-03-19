#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>
#include <QDateTime>

class NetworkInfo : public QObject
{
    Q_OBJECT
public:
    explicit NetworkInfo(QObject *parent = nullptr);
    NetworkInfo(const QString &name,
                         const QString &mac,
                         int speed,
                         bool isUp,
                         const QDateTime &timestamp,
                         QObject *parent = nullptr);
    NetworkInfo(const NetworkInfo& obj);
    bool operator==(const NetworkInfo &other) const;
    bool operator!=(const NetworkInfo &other) const;

    QString getName() const { return m_name; }
    QString getMac() const { return m_mac; }
    int getSpeed() const { return m_speed; }
    bool getIsUp() const { return m_isUp; }
    QDateTime getTimestamp() const { return m_timestamp; }

    void setName(const QString &name);
    void setMac(const QString &mac);
    void setSpeed(int speed);
    void setIsUp(bool isUp);
    void setTimestamp(const QDateTime &timestamp);
    QString ipv4() const;
    void setIpv4(const QString &newIpv4);
    void resetIpv4();

    QString netmask() const;
    void setNetmask(const QString &newNetmask);
    void resetNetmask();

    QString broadcast() const;
    void setBroadcast(const QString &newBroadcast);
    void resetBroadcast();

    bool isRunning() const;
    void setIsRunning(bool newIsRunning);
    void resetIsRunning();

    QList<QPair<QString, QString>> getAllKeyValuesAsList();

signals:
    void nameChanged(const QString &);
    void macChanged(const QString &);
    void speedChanged(int);
    void isUpChanged(bool);
    void timestampChanged(const QDateTime &);
    void ipv4Changed();
    void netmaskChanged();
    void broadcastChanged();
    void isRunningChanged();

private:
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString mac READ getMac WRITE setMac NOTIFY macChanged)
    Q_PROPERTY(int speed READ getSpeed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(bool isUp READ getIsUp WRITE setIsUp NOTIFY isUpChanged)
    Q_PROPERTY(QDateTime timestamp READ getTimestamp WRITE setTimestamp NOTIFY timestampChanged)
    Q_PROPERTY(QString ipv4 READ ipv4 WRITE setIpv4 RESET resetIpv4 NOTIFY ipv4Changed FINAL)

    QString m_name;
    QString m_mac;
    QString m_ipv4;
    QString m_netmask;
    QString m_broadcast;
    int m_speed;
    bool m_isUp;
    bool m_isRunning;
    QDateTime m_timestamp;

    Q_PROPERTY(QString netmask READ netmask WRITE setNetmask RESET resetNetmask NOTIFY netmaskChanged FINAL)
    Q_PROPERTY(QString broadcast READ broadcast WRITE setBroadcast RESET resetBroadcast NOTIFY broadcastChanged FINAL)
    Q_PROPERTY(bool isRunning READ isRunning WRITE setIsRunning RESET resetIsRunning NOTIFY isRunningChanged FINAL)
};

#endif // NETWORKINFO_H
