#ifndef NETWORKDASHBOARDMANAGER_H
#define NETWORKDASHBOARDMANAGER_H

#include <QObject>
#include <QHash>
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

public slots:
    void addNetworkInfo(NetworkInfo* info);
    void handleWidgetDropped(const QString& sourceMac, const QString& targetMac);

signals:
    void gridLayoutChanged();
    void widgetPositionsUpdated(const QHash<QString, QPair<int, int>>& positions);

private:
    struct DashboardItem
    {
        NetworkInfoViewModel* viewModel;
        NetworkInfoViewWidget* widget;
        QPair<int, int> gridPosition;
    };

    QGridLayout* m_gridLayout;
    NetworkMonitor* m_networkMonitor;
    QHash<QString, DashboardItem> m_items;
    QHash<QPair<int, int>, QString> m_positionMap;

    void setupWidgetConnections(NetworkInfoViewWidget* widget);
    QPair<int, int> findAvailableGridPosition() const;
    void updateGridLayout();
};

#endif // NETWORKDASHBOARDMANAGER_H
