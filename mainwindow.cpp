#include "mainwindow.h"
#include <QtNetwork/QNetworkInterface>
#include <QDebug>
#include <QHBoxLayout>

#include "networkinfoviewwidget.h"
#include "networkinfoview.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    getAllAvailableNetworksInfo();
    setupUI();
    setGeometry(200,200,1000,800);
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    m_grid = new QGridLayout(central);
    central->setLayout(m_grid);
    setCentralWidget(central);
    addAllNetworkInfoViewWidgets();
}

void MainWindow::getAllAvailableNetworksInfo()
{
    m_viewInfo = new NetworkInfoView(this);
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for(const QNetworkInterface &interface : interfaces)
    {
        if(interface.type() != QNetworkInterface::Ethernet ||
            interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            continue;
        }


        m_viewInfo->createOrUpdateInfo(interface, interface.hardwareAddress());


        //TODO:mb later us eit for logs.
        //qInfo() << "Interface:" << interface.humanReadableName();
        //qInfo() << "  MAC:" << interface.hardwareAddress();
        //qInfo() << "  Type: Ethernet";

        //     const bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
        //     const bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

        //     //netInfoViews.back()->addKeyValue(QPair<QString,QString>("is Up:", isUp ? "True" : "False"));
        //     //netInfoViews.back()->addKeyValue(QPair<QString,QString>("is Running:", isRunning ? "True" : "False"));

        //     bool hasIpv4 = false;
        //     const QList<QNetworkAddressEntry> entries = interface.addressEntries();
        //     for(const QNetworkAddressEntry &entry : entries)
        //     {
        //         if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
        //         {
        //             hasIpv4 = true;

        //             //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("IPv4:", entry.ip().toString()));
        //             //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("Netmask:", entry.netmask().toString()));
        //             //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("Broadcast:", entry.broadcast().toString()));
        //             // qInfo() << "  IPv4:" << entry.ip().toString();
        //             // qInfo() << "  Netmask:" << entry.netmask().toString();
        //             // qInfo() << "  Broadcast:" << entry.broadcast().toString();
        //         }
        //     }

        //     if(isUp && isRunning && hasIpv4)
        //     {
        //         ethernetConnected = true;
        //         //netInfoViewWidgets.back()->addKeyValue(QPair<QString,QString>("ethernet Connected:", ethernetConnected ? "True" : "False"));
        //         //qInfo() << "  --> ACTIVE ETHERNET CONNECTION DETECTED";
        //     }
        //     //netInfoViewWidgets.emplaceBack(new NetworkInfoViewWidget(this));
        //     //qInfo() << "----------------------------------------";
    }

    //qInfo() << "\nEthernet connection available:" << (ethernetConnected ? "Yes" : "No");
}

void MainWindow::addInfoViewWidget(NetworkInfo* info)
{
    if(!m_netInfoViewWidgets.contains(info->getMac()))
    {
        NetworkInfoViewWidget* netInfoWdgt = new NetworkInfoViewWidget(info, this);
        m_netInfoViewWidgets.insert(info->getMac(), netInfoWdgt);
        m_grid->addWidget(m_netInfoViewWidgets.value(info->getMac()));
        arrangeGrid();
    }
}

void MainWindow::updateInfoViewWidgee(NetworkInfo *info)
{

}

void MainWindow::removeInfoViewWidge(const QString &mac)
{

}

void MainWindow::arrangeGrid()
{
    QLayoutItem* item;
    while ((item = m_grid->takeAt(0)) != nullptr)
    {
        delete item; // Delete layout item, not the widget
    }

    // Get sorted list of network interfaces
    QList<NetworkInfoViewWidget*> widgets = m_netInfoViewWidgets.values();

    // Sort widgets by MAC address (or other preferred property)
    // std::sort(widgets.begin(), widgets.end(), [](NetworkInfoViewWidget* a, NetworkInfoViewWidget* b) {
    //     return a->m() < b->macAddress(); // Use actual getter method
    // });


    const int minColumnWidth = 300; //TODO: calculate properly
    const int columnCount = qMax(1, width() / minColumnWidth);

    int row = 0, col = 0;
    for (NetworkInfoViewWidget* widget : widgets)
    {
        m_grid->addWidget(widget, row, col);
        if (++col >= columnCount)
        {
            col = 0;
            row++;
        }
    }

    // if (col != 0)
    // {
    //     m_grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum),
    //                     row, col, 1, columnCount - col);
    // }
}

void MainWindow::addAllNetworkInfoViewWidgets()
{
    if(!m_viewInfo) return;

    const auto &infos = m_viewInfo->getNetworkInfos();

    for(NetworkInfo* netInfo : infos)
    {
        addInfoViewWidget(netInfo);
    }
}
