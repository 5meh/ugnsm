#ifndef NETWORKINFOMODEL_H
#define NETWORKINFOMODEL_H

#include <QObject>

class NetworkInfo;

class NetworkInfoModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString mac READ getMac NOTIFY macChanged)
    Q_PROPERTY(QString ipAddress READ getIpAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(QString netmask READ getNetmask NOTIFY netmaskChanged)
    Q_PROPERTY(QString downloadSpeed READ getDownloadSpeed WRITE setDownloadSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString uploadSpeed READ getUploadSpeed WRITE setUploadSpeed NOTIFY speedChanged)
    Q_PROPERTY(quint64 totalSpeed READ getTotalSpeed NOTIFY speedChanged)

public:
    explicit NetworkInfoModel(NetworkInfo* model, QObject* parent = nullptr);

    QList<QPair<QString, QString>> getAllKeyValuesAsList() const;
    QString getName() const;
    QString getMac() const;
    QString getIpAddress() const;
    QString getNetmask() const;
    QString getDownloadSpeed() const;
    QString getUploadSpeed() const;
    QString getTotalSpeed() const;
    QDateTime getTimestamp() const;
    qint64 getLastUpdateTime() const;

    //void updateFromModel();
public slots:
    void updateSpeeds(quint64 rx, quint64 tx);

signals:
    void nameChanged(const QString& name);
    void macChanged(const QString& mac);
    void ipAddressChanged(const QString& ip);
    void netmaskChanged(const QString& netmask);
    void speedChanged(qint64, qint64);
//    void speedChanged(quint64 download, quint64 upload);

private:
    QString formatSpeed(quint64 bytes) const;
    void connectModelSignals();

    NetworkInfo* m_model;
    //QString m_uploadSpeed;
};

#endif // NETWORKINFOMODEL_H
