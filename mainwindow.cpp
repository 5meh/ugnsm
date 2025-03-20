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

    connect(this, &MainWindow::widgetsSwapped, this, &MainWindow::handleWidgetsSwap);
}

MainWindow::~MainWindow()
{
    qDeleteAll(m_netInfoViewWidgets);
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    m_grid = new QGridLayout(central);
    central->setLayout(m_grid);
    setCentralWidget(central);
    m_grid->setSpacing(15);
    m_grid->setContentsMargins(10, 10, 10, 10);
    addAllNetworkInfoViewWidgets();
}

void MainWindow::getAllAvailableNetworksInfo()
{
    m_viewInfo = new NetworkInfoView(this);
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();


    for(const auto &interface : interfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            m_viewInfo->createOrUpdateInfo(interface, interface.hardwareAddress());
        }
    }

    //TODO:mb later us eit for logs.
    //qInfo() << "Interface:" << interface.humanReadableName();
    //qInfo() << "  MAC:" << interface.hardwareAddress();
    //qInfo() << "  Type: Ethernet";
}

void MainWindow::handleWidgetsSwap(QWidget *source, QWidget *target)
{
    NetworkInfoViewWidget* srcWidget = qobject_cast<NetworkInfoViewWidget*>(source);
    NetworkInfoViewWidget* tgtWidget = qobject_cast<NetworkInfoViewWidget*>(target);

    if(!srcWidget || !tgtWidget) return;

    const int srcIdx = m_visualOrder.indexOf(srcWidget);
    const int tgtIdx = m_visualOrder.indexOf(tgtWidget);

    if(srcIdx != -1 && tgtIdx != -1)
    {
        m_visualOrder.swapItemsAt(srcIdx, tgtIdx);
        arrangeGrid();
    }
}

void MainWindow::addInfoViewWidget(NetworkInfo* info)
{
    if(!m_netInfoViewWidgets.contains(info->getMac()))
    {
        NetworkInfoViewWidget* netInfoWdgt = new NetworkInfoViewWidget(info, this);
        m_netInfoViewWidgets.insert(info->getMac(), netInfoWdgt);
        m_visualOrder.append(netInfoWdgt);

        connect(netInfoWdgt, &NetworkInfoViewWidget::swapRequested,
                this, &MainWindow::handleWidgetsSwap);

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
    while((item = m_grid->takeAt(0))) delete item;

    const int minColumnWidth = 300;
    const int columnCount = qMax(1, width() / minColumnWidth);
    int row = 0, col = 0;

    for(NetworkInfoViewWidget* widget : m_visualOrder)
    {
        m_grid->addWidget(widget, row, col);
        widget->setMinimumSize(280, 150);
        widget->show();

        if(++col >= columnCount)
        {
            col = 0;
            row++;
        }
    }

    if(col != 0)
    {
        m_grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum),
                        row, col, 1, columnCount - col);
    }
}

void MainWindow::addAllNetworkInfoViewWidgets()
{
    if(!m_viewInfo) return;

    for(NetworkInfo* netInfo : m_viewInfo->getNetworkInfos())
    {
        addInfoViewWidget(netInfo);
    }
}
