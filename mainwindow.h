#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkInterface>

QT_FORWARD_DECLARE_CLASS(NetworkInfoView);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void setupUI();
    void getAllAvailableNetworksInfo();
    QList<NetworkInfoView*> netInfoViews;
};
#endif // MAINWINDOW_H
