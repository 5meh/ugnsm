#ifndef NETWORKDASHBOARDMANAGER_H
#define NETWORKDASHBOARDMANAGER_H

#include <QObject>
#include <QHash>
#include <QPoint>
#include <QPair>

class NetworkInfo;
class NetworkInfoViewModel;
class NetworkInfoViewWidget;
class NetworkMonitor;
class QGridLayout;

class NetworkDashboardManager: public QObject
{
    Q_OBJECT
public:
    explicit NetworkDashboardManager(QGridLayout* gridLayout, QObject* parent = nullptr);
    void initialize(NetworkMonitor* networkMonitor);

    QHash<QString, QPoint> getCurrentPositions() const;
public slots:
    void addNetworkInfo(NetworkInfo* info);
    void handleWidgetDropped(const QString& sourceMac, const QString& targetMac);

signals:
    void gridLayoutChanged();
    void widgetPositionsUpdated(const QHash<QString, QPoint>& positions);

private:
    struct DashboardItem
    {
        NetworkInfoViewModel* viewModel;
        NetworkInfoViewWidget* widget;
        QPoint gridPosition;
    };

    QGridLayout* m_gridLayout;
    NetworkMonitor* m_networkMonitor;
    QHash<QString, DashboardItem> m_items;
    QHash<QPoint, QString> m_positionMap;

    void setupWidgetConnections(NetworkInfoViewWidget* widget);
    QPoint findAvailableGridPosition() const;
    void updateGridLayout();
};

#endif // NETWORKDASHBOARDMANAGER_H
