#include "mainwindow.h"
#include <QtNetwork/QNetworkInterface>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    bool ethernetConnected = false;

    // Get all network interfaces
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    qInfo() << "Checking network interfaces...\n";

    for(const QNetworkInterface &interface : interfaces)
    {
        //QNetworkInterface::
        // Skip loopback and non-ethernet interfaces
        if(interface.type() != QNetworkInterface::Ethernet ||
            interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            continue;
        }

        qInfo() << "Interface:" << interface.humanReadableName();
        qInfo() << "  MAC:" << interface.hardwareAddress();
        qInfo() << "  Type: Ethernet";

        // Check interface status flags
        const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
        const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

        qInfo() << "  Status:"
                << (isUp ? "Up" : "Down")
                << "|"
                << (isRunning ? "Running" : "Not Running");

        // Check for IPv4 connectivity
        bool hasIpv4 = false;
        const QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for(const QNetworkAddressEntry &entry : entries)
        {
            if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                hasIpv4 = true;
                qInfo() << "  IPv4:" << entry.ip().toString();
                qInfo() << "  Netmask:" << entry.netmask().toString();
                qInfo() << "  Broadcast:" << entry.broadcast().toString();
            }
        }

        if(isUp && isRunning && hasIpv4)
        {
            ethernetConnected = true;
            qInfo() << "  --> ACTIVE ETHERNET CONNECTION DETECTED";
        }

        qInfo() << "----------------------------------------";
    }

    qInfo() << "\nEthernet connection available:" << (ethernetConnected ? "Yes" : "No");
}

MainWindow::~MainWindow() {}
