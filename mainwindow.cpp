#include "mainwindow.h"
#include "networkinfoviewwidget.h"
//#include "networkinfo.h"
#include "networkinfoview.h"
#include "networkinfoviewmodel.h"

#include <QtNetwork/QNetworkInterface>
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QDebug>
#include "networkdashboard.h"
#include "networkinfoviewwidget.h"
#include "networkmonitor.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_dashboard(new NetworkDashboard(this)),
    m_gridLayout(new QGridLayout()),
    m_speedMonitor(new NetworkMonitor(this))
{
    setupUI();
    connect(m_dashboard, &NetworkDashboard::layoutChanged,
            this, &MainWindow::handleLayoutChanged);

    connect(m_speedMonitor, &NetworkMonitor::statsUpdated,
            this, [this](const QString& interface, quint64 rx, quint64 tx) {
                foreach(auto* widget, m_widgets) {
                    if(widget->interfaceName() == interface) {
                        widget->viewModel()->updateSpeeds(rx, tx);
                    }
                }
            });
    m_speedMonitor->startMonitoring(1000);
}

MainWindow::~MainWindow()
{
    clearGrid();
    //////////////
    // QLayoutItem* item;
    // while((item = m_grid->takeAt(0)))
    // {
    //     delete item;
    // }

    // qDeleteAll(m_netInfoViewWidgets);
    // m_netInfoViewWidgets.clear();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::DragEnter)
    {
        QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
        if(dragEvent->mimeData()->hasFormat("application/x-networkwidget"))
        {
            dragEvent->acceptProposedAction();
            return true;
        }
    }

    if(event->type() == QEvent::Drop)
    {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        if (dropEvent->mimeData()->hasFormat("application/x-networkwidget"))
        {
            QWidget* target = qobject_cast<QWidget*>(watched);
            if (target && m_draggedWidget)
            {
                // Safely cast both widgets
                NetworkInfoViewWidget* sourceWidget = qobject_cast<NetworkInfoViewWidget*>(m_draggedWidget);
                NetworkInfoViewWidget* targetWidget = qobject_cast<NetworkInfoViewWidget*>(target);

                if (sourceWidget && targetWidget)
                {
                    auto [targetRow, targetCol] = gridPosition(targetWidget);
                    auto [sourceRow, sourceCol] = gridPosition(sourceWidget);

                    if (targetRow != -1 && sourceRow != -1)
                    {
                        m_dashboard->swapPositions(
                            sourceWidget->getMac(),
                            targetWidget->getMac()
                            );
                    }
                }
            }
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);

    ///////////////////////
    // if(event->type() == QEvent::DragEnter)
    // {
    //     QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
    //     if(dragEvent->mimeData()->hasFormat("application/x-networkwidget"))
    //     {
    //         dragEvent->acceptProposedAction();
    //         return true;
    //     }
    // }

    // if(event->type() == QEvent::Drop)
    // {
    //     QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
    //     if(dropEvent->mimeData()->hasFormat("application/x-networkwidget"))
    //     {
    //         QWidget* target = qobject_cast<QWidget*>(watched);
    //         if(target && m_draggedWidget)
    //         {
    //             auto [targetRow, targetCol] = gridPosition(target);
    //             auto [sourceRow, sourceCol] = gridPosition(m_draggedWidget);

    //             if(targetRow != -1 && sourceRow != -1)
    //             {
    //                 m_dashboard->swapPositions(
    //                     m_widgets.key(m_draggedWidget),
    //                     m_widgets.key(qobject_cast<NetworkInfoViewWidget*>(target))
    //                     );
    //             }
    //         }
    //         return true;
    //     }
    // }

    // return QMainWindow::eventFilter(watched, event);
}

void MainWindow::handleLayoutChanged()
{
    updateGridDisplay();
}

void MainWindow::handleDragInitiated(QWidget *source)
{
    if (auto* validWidget = qobject_cast<NetworkInfoViewWidget*>(source))
    {
        m_draggedWidget = validWidget;
    }
}

void MainWindow::handleDropReceived(QWidget *target)
{
    Q_UNUSED(target)
    m_draggedWidget = nullptr;
}

void MainWindow::addNetworkInfo(NetworkInfo *info)
{
    NetworkInfoViewModel* viewModel = new NetworkInfoViewModel(info, this);
    m_dashboard->addInterface(viewModel);
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(m_gridLayout);
    setCentralWidget(centralWidget);

    m_gridLayout->setSpacing(15);
    m_gridLayout->setContentsMargins(20, 20, 20, 20);

    // Initialize grid with placeholders
    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            m_gridLayout->addWidget(createPlaceholder(), row, col);
        }
    }
    /////////////////////
    // QWidget *central = new QWidget(this);
    // central->setLayout(m_grid);
    // setCentralWidget(central);

    // m_grid->setSpacing(m_gridSpacing);
    // m_grid->setContentsMargins(m_gridMargins, m_gridMargins,
    //                            m_gridMargins, m_gridMargins);

    // initializeGrid();
    // addAllNetworkInfoViewWidgets();
    // arrangeGrid();
}

void MainWindow::initializeNetworkDashboard()
{
    const auto interfaces = QNetworkInterface::allInterfaces();

    for(const auto& interface : interfaces) {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            addOrUpdateNetworkWidget(interface);
        }
    }
    updateGridDisplay();
}

void MainWindow::clearGrid()
{
    for(int i = 0; i < m_gridLayout->count(); ++i)
    {
        QWidget* widget = m_gridLayout->itemAt(i)->widget();
        if(widget && widget->property("isPlaceholder").toBool())
        {
            delete widget;
        }
    }

}

QWidget *MainWindow::createPlaceholder()
{
    QWidget* ph = new QWidget();
    ph->setFixedSize(WIDGET_SIZE);
    ph->setStyleSheet("background-color: #F0F0F0; border: 2px dashed #AAAAAA; border-radius: 8px;");
    ph->setProperty("isPlaceholder", true);
    ph->installEventFilter(this);
    return ph;
}

// void MainWindow::initializeGrid()
// {
//     for(int row = 0; row < GRID_SIZE; row++)
//     {
//         for(int col = 0; col < GRID_SIZE; col++)
//         {
//             m_grid->addWidget(createPlaceholderWidget(), row, col);
//         }
//     }
// }

// QWidget* MainWindow::createPlaceholderWidget()
// {
//     QWidget* placeholder = new QWidget(this);
//     placeholder->setFixedSize(m_widgetSize);
//     placeholder->setStyleSheet(
//         "border: 2px dashed gray;"
//         "background-color: rgba(200, 200, 200, 50);"
//         );

//     QLabel* label = new QLabel("Empty", placeholder);
//     label->setAlignment(Qt::AlignCenter);
//     label->setStyleSheet("QLabel { color: gray; font: italic 10pt; }");

//     QVBoxLayout* layout = new QVBoxLayout(placeholder);
//     layout->addWidget(label);

//     placeholder->setAcceptDrops(true);
//     placeholder->installEventFilter(this);

//     return placeholder;
// }

// bool MainWindow::eventFilter(QObject* watched, QEvent* event)
// {
//     if (event->type() == QEvent::DragEnter)
//     {
//         QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
//         if (dragEvent->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
//         {
//             dragEvent->acceptProposedAction();
//             return true;
//         }
//     }

//     if (event->type() == QEvent::Drop)
//     {
//         QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
//         if (dropEvent->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
//         {
//             QWidget* sourceWidget = qobject_cast<QWidget*>(dropEvent->mimeData()->property("widget").value<QWidget*>());

//             QWidget* target = qobject_cast<QWidget*>(watched);

//             QGridLayout* grid = m_grid;//TODO; mb no need in variable

//             int targetRow = -1, targetCol = -1;
//             bool found = false;

//             for (int i = 0; i < grid->count(); ++i)
//             {
//                 int r, c, rs, cs;
//                 grid->getItemPosition(i, &r, &c, &rs, &cs);
//                 QLayoutItem* item = grid->itemAtPosition(r, c);
//                 if (item && item->widget() == target)
//                 {
//                     targetRow = r;
//                     targetCol = c;
//                     found = true;
//                     break;
//                 }
//             }

//             // If not found, check target's parent
//             if (!found && target->parentWidget())
//             {
//                 QWidget* parent = target->parentWidget();
//                 for (int i = 0; i < grid->count(); ++i)
//                 {
//                     int r, c, rs, cs;
//                     grid->getItemPosition(i, &r, &c, &rs, &cs);
//                     QLayoutItem* item = grid->itemAtPosition(r, c);
//                     if (item && item->widget() == parent)
//                     {
//                         targetRow = r;
//                         targetCol = c;
//                         found = true;
//                         break;
//                     }
//                 }
//             }

//             if (!found)
//             {
//                 qWarning() << "Drop target position not found";
//                 return QMainWindow::eventFilter(watched, event);
//             }

//             QLayoutItem* targetItem = grid->itemAtPosition(targetRow, targetCol);
//             QWidget* targetWidget = targetItem ? targetItem->widget() : nullptr;

//             if (targetWidget)
//             {
//                 if(auto networkWidget = qobject_cast<NetworkInfoViewWidget*>(targetWidget))
//                 {
//                     handleWidgetsSwap(sourceWidget, networkWidget);
//                 }
//                 else
//                 {
//                     handleDropOnPlaceholder(sourceWidget, targetRow, targetCol);
//                 }
//             }
//             else
//             {
//                 qWarning() << "Target cell has no widget";
//             }

//             return true;
//         }
//     }

//     return QMainWindow::eventFilter(watched, event);
// }

// void MainWindow::handleDropOnPlaceholder(QWidget* source, int targetRow, int targetCol)
// {
//     if (targetRow < 0 || targetRow >= GRID_SIZE ||
//         targetCol < 0 || targetCol >= GRID_SIZE)
//         return;

//     NetworkInfoViewWidget* sourceWidget = qobject_cast<NetworkInfoViewWidget*>(source);
//     if (!sourceWidget)
//         return;

//     int sourceRow = -1, sourceCol = -1;
//     for (int r = 0; r < GRID_SIZE; r++)
//     {
//         for (int c = 0; c < GRID_SIZE; c++)
//         {
//             if (m_gridSlots[r][c] == sourceWidget)
//             {
//                 sourceRow = r;
//                 sourceCol = c;
//                 break;
//             }
//         }
//         if (sourceRow != -1)
//             break;
//     }
//     if (sourceRow == -1 || sourceCol == -1)
//         return;

//     QLayoutItem* targetItem = m_grid->itemAtPosition(targetRow, targetCol);
//     if (targetItem && targetItem->widget())
//     {
//         QWidget* targetWidget = targetItem->widget();
//         m_grid->removeWidget(targetWidget);
//         if (!qobject_cast<NetworkInfoViewWidget*>(targetWidget))
//         {
//             delete targetWidget;  // Delete placeholder
//         }
//     }

//     m_gridSlots[targetRow][targetCol] = sourceWidget;
//     m_gridSlots[sourceRow][sourceCol] = nullptr;

//     QLayoutItem* sourceItem = m_grid->itemAtPosition(sourceRow, sourceCol);
//     if (sourceItem && sourceItem->widget())
//     {
//         QWidget* sourcePositionWidget = sourceItem->widget();
//         m_grid->removeWidget(sourcePositionWidget);
//         if (!qobject_cast<NetworkInfoViewWidget*>(sourcePositionWidget))
//         {
//             delete sourcePositionWidget;
//         }
//     }

//     m_grid->addWidget(sourceWidget, targetRow, targetCol);
//     m_grid->addWidget(createPlaceholderWidget(), sourceRow, sourceCol);

//     updateGridDisplay();
// }

void MainWindow::updateGridDisplay()
{
    clearGrid();

    const auto layout = m_dashboard->currentLayout();

    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            if(NetworkInfoViewModel* vm = m_dashboard->viewModelAt(row, col))
            {
                if(!m_widgets.contains(vm->getMac()))
                {
                    auto* widget = new NetworkInfoViewWidget(vm, this);
                    m_widgets[vm->getMac()] = widget;
                    connect(widget, &NetworkInfoViewWidget::dragInitiated,
                            this, &MainWindow::handleDragInitiated);
                    connect(widget, &NetworkInfoViewWidget::dropReceived,
                            this, &MainWindow::handleDropReceived);
                }
                m_gridLayout->addWidget(m_widgets[vm->getMac()], row, col);
            }
            else
            {
                m_gridLayout->addWidget(createPlaceholder(), row, col);
            }
        }
    }
    ///////////////////////////
    // setUpdatesEnabled(false);

    // QHash<QPair<int, int>, QWidget*> currentItems;
    // for(int row = 0; row < GRID_SIZE; row++)
    // {
    //     for(int col = 0; col < GRID_SIZE; col++)
    //     {
    //         QLayoutItem* item = m_grid->itemAtPosition(row, col);
    //         if(item && item->widget())
    //         {
    //             currentItems[{row, col}] = item->widget();
    //         }
    //     }
    // }

    // for(int row = 0; row < GRID_SIZE; row++)
    // {
    //     for(int col = 0; col < GRID_SIZE; col++)
    //     {
    //         QWidget* targetWidget = m_gridSlots[row][col];
    //         QWidget* currentWidget = currentItems.value({row, col}, nullptr);

    //         if(targetWidget)
    //         {
    //             if(currentWidget != targetWidget)
    //             {
    //                 if(currentWidget)
    //                 {
    //                     m_grid->removeWidget(currentWidget);
    //                     if(!qobject_cast<NetworkInfoViewWidget*>(currentWidget))
    //                     {
    //                         delete currentWidget;
    //                     }
    //                 }
    //                 m_grid->addWidget(targetWidget, row, col);
    //             }
    //         }
    //         else
    //         {
    //             if(!currentWidget || qobject_cast<NetworkInfoViewWidget*>(currentWidget))
    //             {
    //                 if(currentWidget)
    //                     m_grid->removeWidget(currentWidget);
    //                 m_grid->addWidget(createPlaceholderWidget(), row, col);
    //             }
    //         }
    //     }
    // }

    // setUpdatesEnabled(true);
    // arrangeGrid();
}

QPair<int, int> MainWindow::gridPosition(QWidget *widget) const
{
    int index = m_gridLayout->indexOf(widget);
    if(index != -1)
    {
        int row, col, rowSpan, colSpan;
        m_gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
        return {row, col};
    }
    return {-1, -1};
}

void MainWindow::addOrUpdateNetworkWidget(const QNetworkInterface &interface)
{
    QString mac = interface.hardwareAddress();
    bool isUp = interface.flags().testFlag(QNetworkInterface::IsUp);
    bool isRunning = interface.flags().testFlag(QNetworkInterface::IsRunning);

    if(!m_widgets.contains(mac))
    {
        NetworkInfo* netInfo = new NetworkInfo(
            interface.name(),
            mac,
            isUp,
            isRunning,
            QDateTime::currentDateTime(),
            this
            );

        NetworkInfoViewModel* viewModel = new NetworkInfoViewModel(netInfo, this);
        NetworkInfoViewWidget* widget = new NetworkInfoViewWidget(viewModel, this);

        connect(widget, &NetworkInfoViewWidget::dragInitiated,
                this, &MainWindow::handleDragInitiated);
        connect(widget, &NetworkInfoViewWidget::dropReceived,
                this, &MainWindow::handleDropReceived);

        m_widgets.insert(mac, widget);
        m_dashboard->addNetwork(viewModel);
    }
    else
    {
        bool changed = false;

        if(viewModel-> != interface.humanReadableName())
        {
            info->setName(interface.humanReadableName());
            changed = true;
        }

        if(info->getIsUp() != isUp)
        {
            info->setIsUp(isUp);
            changed = true;
        }

        if(info->isRunning() != isRunning)
        {
            info->setIsRunning(isRunning);
            changed = true;
        }

        if(hasIpv4)
        {
            if(info->getIpv4() != ipv4)
            {
                info->setIpv4(ipv4);
                changed = true;
            }
            if(info->getNetmask() != netmask)
            {
                info->setNetmask(netmask);
                changed = true;
            }
            if(info->getBroadcast() != broadcast)
            {
                info->setBroadcast(broadcast);
                changed = true;
            }
        }

        quint64 rx, tx;
        if(getInterfaceStats(interface.humanReadableName(), rx, tx))
        {
            if(info->getLastRxBytes() != rx)
            {
                info->setLastRxBytes(rx);
                changed = true;
            }
            if(info->getLastTxBytes() != tx)
            {
                info->setLastTxBytes(tx);
                changed = true;
            }
        }

        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if(info->getLastUpdateTime() != currentTime)
        {
            info->setLastUpdateTime(currentTime);
            changed = true;
        }

        info->setTimestamp(now);

        if(changed)
        {
            emit networkInfoUpdated(info);
        }
    }
    // NetworkInfoViewWidget* widget = m_widgets[mac];
    // widget->viewModel()->setName(interface.name());
    // widget->viewModel()->setIsUp(interface.isUp());
    // Update other properties as needed
}

// void MainWindow::arrangeGrid()
// {
//     for(int row = 0; row < GRID_SIZE; row++)
//     {
//         for(int col = 0; col < GRID_SIZE; col++)
//         {
//             if(QLayoutItem* item = m_grid->itemAtPosition(row, col))
//             {
//                 if(QWidget* w = item->widget())
//                 {
//                     w->setFixedSize(m_widgetSize);
//                     w->updateGeometry();
//                 }
//             }
//         }
//     }
//     m_grid->update();
// }

// void MainWindow::addInfoViewWidget(NetworkInfoViewModel* infoViewModel)
// {
//     if(m_netInfoViewWidgets.contains(infoViewModel->getMac()))
//         return;

//     for(int row = 0; row < GRID_SIZE; row++)
//     {
//         for(int col = 0; col < GRID_SIZE; col++)
//         {
//             if(!m_gridSlots[row][col])
//             {
//                 NetworkInfoViewWidget* widget = new NetworkInfoViewWidget(infoViewModel, this);
//                 m_netInfoViewWidgets.insert(infoViewModel->getMac(), widget);
//                 m_gridSlots[row][col] = widget;

//                 connect(widget, &NetworkInfoViewWidget::swapRequested,
//                         this, &MainWindow::handleWidgetsSwap);
//                 connect(m_viewInfo, &NetworkInfoView::networkInfoUpdated, widget, &NetworkInfoViewWidget::updateNetworkInfoDisplay);

//                 updateGridDisplay();
//                 return;
//             }
//         }
//     }
//     qWarning() << "Grid is full - cannot add new widget";
// }

// void MainWindow::handleWidgetsSwap(QWidget *source, QWidget *target)
// {
//     NetworkInfoViewWidget* srcWidget = qobject_cast<NetworkInfoViewWidget*>(source);
//     NetworkInfoViewWidget* tgtWidget = qobject_cast<NetworkInfoViewWidget*>(target);

//     if(!srcWidget || !tgtWidget || srcWidget == tgtWidget)
//         return;

//     int srcRow = -1, srcCol = -1;
//     int tgtRow = -1, tgtCol = -1;

//     for(int r = 0; r < GRID_SIZE && (srcRow == -1 || tgtRow == -1); r++)
//     {
//         for(int c = 0; c < GRID_SIZE && (srcCol == -1 || tgtCol == -1); c++)
//         {
//             if(m_gridSlots[r][c] == srcWidget)
//             {
//                 srcRow = r;
//                 srcCol = c;
//             }
//             else if(m_gridSlots[r][c] == tgtWidget)
//             {
//                 tgtRow = r;
//                 tgtCol = c;
//             }
//         }
//     }

//     if(srcRow == -1 || srcCol == -1 || tgtRow == -1 || tgtCol == -1)
//         return;

//     m_grid->removeWidget(srcWidget);
//     m_grid->removeWidget(tgtWidget);

//     m_grid->addWidget(srcWidget, tgtRow, tgtCol);
//     m_grid->addWidget(tgtWidget, srcRow, srcCol);

//     m_gridSlots[srcRow][srcCol] = tgtWidget;
//     m_gridSlots[tgtRow][tgtCol] = srcWidget;

//     //m_grid->activate();
//     updateGridDisplay();
// }

// void MainWindow::addAllNetworkInfoViewWidgets()
// {
//     if(!m_viewInfo)
//         return;

//     for(NetworkInfo* netInfo: m_viewInfo->getNetworkInfos().values())
//     {
//         addInfoViewWidget(netInfo);
//     }
// }
