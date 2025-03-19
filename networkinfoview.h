#ifndef NETWORKINFOVIEW_H
#define NETWORKINFOVIEW_H

#include <QObject>
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QNetworkInterface)
#include "networkinfo.h"

class NetworkInfoView: public QObject
{
    Q_OBJECT
public:
    explicit NetworkInfoView(QObject* parent = nullptr);
    QList<NetworkInfo*> getNetworkInfos() const { return m_networkInfos.values(); }
    NetworkInfo* createOrUpdateInfo(const QNetworkInterface &interface, const QString &mac);
    void checkRemovedInfo(const QHash<QString, NetworkInfo*> &current);
signals:
    void networkInfoAdded(NetworkInfo* info);
    void networkInfoUpdated(NetworkInfo* info);
    void networkInfoRemoved(const QString &mac);
public slots:
    void refresh();
private:
    QHash<QString, NetworkInfo*> m_networkInfos;
    QTimer *m_timer;
};

#endif // NETWORKINFOVIEW_H
