#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkInterface>

QT_FORWARD_DECLARE_CLASS(NetworkInfoViewWidget);
QT_FORWARD_DECLARE_CLASS(NetworkInfoView);
QT_FORWARD_DECLARE_CLASS(QGridLayout);
class NetworkInfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void setupUI();
    void getAllAvailableNetworksInfo();

    void addInfoViewWidget(NetworkInfo *info);
    void updateInfoViewWidgee(NetworkInfo *info);
    void removeInfoViewWidge(const QString &mac);

    void arrangeGrid();
    void addAllNetworkInfoViewWidgets();

    QGridLayout *m_grid;
    NetworkInfoView* m_viewInfo;
    QHash<QString, NetworkInfoViewWidget*> m_netInfoViewWidgets;
};
#endif // MAINWINDOW_H
