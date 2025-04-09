#ifndef NETWORKDASHBOARDMANAGER_H
#define NETWORKDASHBOARDMANAGER_H

#include <QObject>
#include <QHash>
#include <QPoint>
#include <QPair>
#include <QSize>

QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

class NetworkInfo;
class NetworkInfoModel;
class NetworkInfoViewWidget;
class NetworkMonitor;

class NetworkDashboardManager: public QObject
{
    Q_OBJECT
public:
    explicit NetworkDashboardManager(QGridLayout* gridLayout, QObject* parent = nullptr);
    void initialize(NetworkMonitor* networkMonitor);
    void createInitialGrid();

    enum SortingCriteria
    {
        TotalSpeed,
        UploadSpeed,
        DownloadSpeed,
        Latency
    };
    Q_ENUM(SortingCriteria)

    void setSortingCriteria(SortingCriteria criteria);

    //QHash<QString, QPoint> getCurrentPositions() const;//TODO: mb remove
public slots:
    void addNetworkInfo(NetworkInfo* info);
    void handleWidgetDropped(const QString& sourceMac, const QString& targetMac);
    void updateNetworkRankings();
    void handleStatsUpdate(const QString& interface, quint64 rx, quint64 tx);
    void handleDragDrop(const QString& draggedMac, const QPoint& newPos);

signals:
    void gridLayoutChanged();
    void initializationComplete();
    void widgetPositionsUpdated(const QHash<QString, QPoint>& positions);

private slots:
    void handleStatsUpdated(const QString& interface, quint64 rx, quint64 tx);

private:
    struct DashboardItem
    {
        NetworkInfoModel* viewModel;
        NetworkInfoViewWidget* widget;
        QPoint gridPosition;
    };

    quint64 calculateNetworkScore(const NetworkInfo* info) const;
    QWidget* createPlaceholder() const;
    void setupWidgetConnections(QWidget* widget);
    QPoint findAvailableGridPosition() const;
    void updateGridLayout();
    void positionBestNetwork();
    void updatePlaceholders();

    QString m_draggedMac;
    QGridLayout* m_gridLayout;
    NetworkMonitor* m_networkMonitor;
    QHash<QString, DashboardItem> m_items;
    QHash<QPoint, QString> m_positionMap;
    QProgressBar* m_loadingBar;
    bool m_initialized = false;
    static constexpr int GRID_SIZE = 9;
    static constexpr QSize WIDGET_SIZE = QSize(120, 80);
    int m_loadedCount = 0;
    SortingCriteria m_sortCriteria = TotalSpeed;
    //void setupWidgetConnections(NetworkInfoViewWidget* widget);
    //QPoint findAvailableGridPosition() const;
    //void updateGridLayout();
};

#endif // NETWORKDASHBOARDMANAGER_H
