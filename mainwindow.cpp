#include "mainwindow.h"
#include <QtNetwork/QNetworkInterface>
#include <QDebug>
#include <QHBoxLayout>

#include "networkinfoview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    getAllAvailableNetworksInfo();
    setupUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI()
{
    setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(new QHBoxLayout(this));
    for(NetworkInfoView* widget: netInfoViews)
        centralWidget()->layout()->addWidget(widget);
}

void MainWindow::getAllAvailableNetworksInfo()
{
    bool ethernetConnected = false;

    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    //qInfo() << "Checking network interfaces...\n";

    for(const QNetworkInterface &interface : interfaces)
    {
        // Skip loopback and non-ethernet interfaces
        if(interface.type() != QNetworkInterface::Ethernet ||
            interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            continue;
        }

        //TODO:make grid with widgets
        netInfoViews.emplaceBack(new NetworkInfoView(this));
        netInfoViews.back()->addKeyValue(QPair<QString,QString>("Interface:", interface.humanReadableName()));
        netInfoViews.back()->addKeyValue(QPair<QString,QString>("MAC:", interface.hardwareAddress()));
        netInfoViews.back()->addKeyValue(QPair<QString,QString>("Type:", "Ethernet"));

        //TODO:mb later us eit for logs.
        //qInfo() << "Interface:" << interface.humanReadableName();
        //qInfo() << "  MAC:" << interface.hardwareAddress();
        //qInfo() << "  Type: Ethernet";

        const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
        const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

        netInfoViews.back()->addKeyValue(QPair<QString,QString>("is Up:", isUp ? "True" : "False"));
        netInfoViews.back()->addKeyValue(QPair<QString,QString>("is Running:", isRunning ? "True" : "False"));

        // qInfo() << "  Status:"
        //         << (isUp ? "Up" : "Down")
        //         << "|"
        //         << (isRunning ? "Running" : "Not Running");

        bool hasIpv4 = false;
        const QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for(const QNetworkAddressEntry &entry : entries)
        {
            if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                hasIpv4 = true;

                netInfoViews.back()->addKeyValue(QPair<QString,QString>("IPv4:", entry.ip().toString()));
                netInfoViews.back()->addKeyValue(QPair<QString,QString>("Netmask:", entry.netmask().toString()));
                netInfoViews.back()->addKeyValue(QPair<QString,QString>("Broadcast:", entry.broadcast().toString()));
                // qInfo() << "  IPv4:" << entry.ip().toString();
                // qInfo() << "  Netmask:" << entry.netmask().toString();
                // qInfo() << "  Broadcast:" << entry.broadcast().toString();
            }
        }

        if(isUp && isRunning && hasIpv4)
        {
            ethernetConnected = true;
            netInfoViews.back()->addKeyValue(QPair<QString,QString>("ethernet Connected:", ethernetConnected ? "True" : "False"));
            //qInfo() << "  --> ACTIVE ETHERNET CONNECTION DETECTED";
        }

        //qInfo() << "----------------------------------------";
    }

    qInfo() << "\nEthernet connection available:" << (ethernetConnected ? "Yes" : "No");
}
