#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkInterface>
#include <QVector>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(NetworkInfoViewWidget)
QT_FORWARD_DECLARE_CLASS(NetworkInfoView)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QWidget)
class NetworkInfo;
class NetworkInfoViewModel;
class NetworkDashboard;
class NetworkMonitor;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void handleLayoutChanged();
    void handleDragInitiated(QWidget* source);
    void handleDropReceived(QWidget* target);
    void addNetworkInfo(NetworkInfo* info);
signals:
    void widgetsSwapped(QWidget* source, QWidget* target);    
private:
    void setupUI();
    void initializeNetworkDashboard();
    void clearGrid();
    QWidget* createPlaceholder();
    void updateGridDisplay();
    QPair<int, int> gridPosition(QWidget* widget) const;

    void addOrUpdateNetworkWidget(const QNetworkInterface& interface);
    void addAllNetworkInfoViewWidgets();

    NetworkDashboard* m_dashboard;
    NetworkMonitor* m_speedMonitor;
    QGridLayout* m_gridLayout;
    QHash<QString, NetworkInfoViewWidget*> m_widgets;
    QWidget* m_draggedWidget = nullptr;
    QTimer* m_refreshTimer;

    static constexpr int GRID_SIZE = 3;
    const QSize WIDGET_SIZE = QSize(300, 180);
};

#endif // MAINWINDOW_H
