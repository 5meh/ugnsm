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
////////////////////////////////////////////////
signals:
    void widgetsSwapped(QWidget* source, QWidget* target);

//protected:
    //bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupUI();

    void clearGrid();
    QWidget* createPlaceholder();
    void updateGridDisplay();
    QPair<int, int> gridPosition(QWidget* widget) const;
    //////////////////
    // void initializeGrid();
    // void updateGridDisplay();

    // void handleWidgetsSwap(QWidget* source, QWidget* target);
    // void handleDropOnPlaceholder(QWidget* source, int row, int col);
    // void addInfoViewWidget(NetworkInfoViewModel* info);

    // QWidget* createPlaceholderWidget();
    // void arrangeGrid();
     void addAllNetworkInfoViewWidgets();

    // static constexpr int GRID_SIZE = 3;
    // QVector<QVector<NetworkInfoViewWidget*>> m_gridSlots;

    // QGridLayout* m_grid;
    // NetworkInfoView* m_viewInfo;

    // const QSize m_widgetSize = QSize(300, 180);//TODO:replace with widget size
    // const int m_gridSpacing = 15;
    // const int m_gridMargins = 10;

    // QHash<QString, NetworkInfoViewWidget*> m_netInfoViewWidgets;
/////////////////////////////////
    //QPair<int, int> gridPosition(QWidget* widget) const;

    NetworkDashboard* m_dashboard;
    QGridLayout* m_gridLayout;
    QHash<QString, NetworkInfoViewWidget*> m_widgets;
    QWidget* m_draggedWidget = nullptr;

    static constexpr int GRID_SIZE = 3;
    const QSize WIDGET_SIZE = QSize(300, 180);
};

#endif // MAINWINDOW_H
