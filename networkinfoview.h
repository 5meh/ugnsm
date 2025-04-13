#ifndef NETWORKINFOVIEW_H
#define NETWORKINFOVIEW_H

#include <QObject>
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QNetworkInterface)
QT_FORWARD_DECLARE_CLASS(NetworkSpeedMonitor)
#include "Core/Network/Information/networkinfo.h"

class NetworkInfoView: public QObject
{
    Q_OBJECT
public:
    explicit NetworkInfoView(QObject* parent = nullptr);

    QHash<QString, NetworkInfo*> getNetworkInfos() const { return m_networkInfos; }
    NetworkInfo* createOrUpdateInfo(const QNetworkInterface& interface, const QString& mac);
    void checkRemovedInfo(const QHash<QString, NetworkInfo*>& current);

signals:
    void networkInfoAdded(NetworkInfo* info);
    void networkInfoUpdated(NetworkInfo* info);
    void networkInfoRemoved(const QString& mac);
    void speedUpdated(const QString& interfaceName,
                      quint64 downloadBps,
                      quint64 uploadBps);
    void errorOccurred(const QString& interfaceName,
                       const QString& message);


public slots:
    void refresh();

private slots:
    void updateSpeeds();

private:
    // Platform-specific implementations
    bool getInterfaceStats(const QString& interfaceName,
                           quint64& rxBytes,
                           quint64& txBytes);

    void getAllAvailableNetworksInfo();
    QString selectBestInterface();

    QHash<QString, NetworkInfo*> m_networkInfos;
    QTimer* netSpeedUpdateTimer;
    QTimer* m_timer;
};

#endif // NETWORKINFOVIEW_H
