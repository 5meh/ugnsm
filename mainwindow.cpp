#include "mainwindow.h"
#include "networkinfoviewwidget.h"
#include "networkinfo.h"
#include "networkinfoview.h"

#include <QtNetwork/QNetworkInterface>
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_gridSlots(GRID_SIZE, QVector<NetworkInfoViewWidget*>(GRID_SIZE, nullptr)),
    m_grid(new QGridLayout()),
    m_viewInfo(new NetworkInfoView(this))
{
    setAcceptDrops(true);
    getAllAvailableNetworksInfo();
    setupUI();
    setGeometry(200, 200, 1000, 800);
}

MainWindow::~MainWindow()
{
    QLayoutItem* item;
    while((item = m_grid->takeAt(0)))
    {
        delete item;
    }

    qDeleteAll(m_netInfoViewWidgets);
    m_netInfoViewWidgets.clear();
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    central->setLayout(m_grid);
    setCentralWidget(central);

    m_grid->setSpacing(m_gridSpacing);
    m_grid->setContentsMargins(m_gridMargins, m_gridMargins,
                               m_gridMargins, m_gridMargins);

    initializeGrid();
    addAllNetworkInfoViewWidgets();
    arrangeGrid();
}

void MainWindow::getAllAvailableNetworksInfo()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for(const auto &interface : interfaces)
    {
        if(interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            m_viewInfo->createOrUpdateInfo(interface, interface.hardwareAddress());
        }
    }
}

void MainWindow::initializeGrid()
{
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            m_grid->addWidget(createPlaceholderWidget(), row, col);
        }
    }
}

QWidget* MainWindow::createPlaceholderWidget()
{
    QWidget* placeholder = new QWidget(this);
    placeholder->setFixedSize(m_widgetSize);
    placeholder->setStyleSheet(
        "border: 2px dashed gray;"
        "background-color: rgba(200, 200, 200, 50);"
        );

    QLabel* label = new QLabel("Drag Network\nWidget Here", placeholder);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { color: gray; font: italic 10pt; }");

    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    layout->addWidget(label);

    placeholder->setAcceptDrops(true);
    placeholder->installEventFilter(this);

    return placeholder;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::DragEnter)
    {
        QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
        if(dragEvent->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
        {
            dragEvent->acceptProposedAction();
            return true;
        }
    }

    if(event->type() == QEvent::Drop)
    {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        if(dropEvent->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
        {
            QWidget* placeholder = qobject_cast<QWidget*>(watched->parent());

            // Add null checks for all parent chain components
            if(placeholder && placeholder->parentWidget())
            {
                QWidget* parentWidget = placeholder->parentWidget();
                QGridLayout* grid = qobject_cast<QGridLayout*>(parentWidget->layout());

                if(grid)
                {
                    int index = grid->indexOf(placeholder);
                    if(index != -1)
                    {
                        int row, col;
                        grid->getItemPosition(index, &row, &col, nullptr, nullptr);
                        handleDropOnPlaceholder(
                            qobject_cast<QWidget*>(dropEvent->mimeData()->property("widget").value<QWidget*>()),
                            row, col
                            );
                        return true;
                    }
                    else
                    {
                        qWarning() << "Placeholder not found in grid";
                    }
                }
                else
                {
                    qWarning() << "Parent widget has no grid layout";
                }
            }
            else
            {
                qWarning() << "Invalid placeholder widget hierarchy";
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::handleDropOnPlaceholder(QWidget* source, int targetRow, int targetCol)
{
    if(targetRow < 0 || targetRow >= GRID_SIZE || targetCol < 0 || targetCol >= GRID_SIZE)
    {
        qWarning() << "Invalid drop position [" << targetRow << "," << targetCol << "]";
        return;
    }

    NetworkInfoViewWidget* sourceWidget = qobject_cast<NetworkInfoViewWidget*>(source);
    if(!sourceWidget)
    {
        qWarning() << "Invalid source widget type";
        return;
    }

    int sourceRow = -1, sourceCol = -1;
    bool found = false;
    for(int r = 0; r < GRID_SIZE && !found; ++r)
    {
        for(int c = 0; c < GRID_SIZE; ++c)
        {
            if(m_gridSlots[r][c] == sourceWidget)
            {
                sourceRow = r;
                sourceCol = c;
                found = true;
                break;
            }
        }
    }

    if(sourceRow == -1 || sourceCol == -1)
    {
        qWarning() << "Source widget not found in grid";
        return;
    }

    if(m_gridSlots[targetRow][targetCol] != nullptr) {
        qWarning() << "Target position already occupied";
        return;
    }

    m_gridSlots[targetRow][targetCol] = sourceWidget;
    m_gridSlots[sourceRow][sourceCol] = nullptr;  // Set old position to placeholder

    updateGridDisplay();
}

void MainWindow::updateGridDisplay()
{
    QLayoutItem* item;
    while((item = m_grid->takeAt(0)))
    {
        QWidget* widget = item->widget();

        // Only delete placeholder widgets, not NetworkInfoViewWidgets
        if(widget && !qobject_cast<NetworkInfoViewWidget*>(widget))
        {
            delete widget;
        }
        delete item; // Always delete the layout item
    }
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            if(m_gridSlots[row][col])
            {
                m_grid->addWidget(m_gridSlots[row][col], row, col);
            }
            else
            {
                m_grid->addWidget(createPlaceholderWidget(), row, col);
            }
        }
    }
    arrangeGrid();
}

void MainWindow::arrangeGrid()
{
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            QLayoutItem* item = m_grid->itemAtPosition(row, col);

            if(item && item->widget())
            {
                item->widget()->setFixedSize(m_widgetSize);
            }
        }
    }
}

void MainWindow::addInfoViewWidget(NetworkInfo *info)
{
    if(m_netInfoViewWidgets.contains(info->getMac())) return;

    // Find first empty slot
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            if(!m_gridSlots[row][col])
            {
                NetworkInfoViewWidget* widget = new NetworkInfoViewWidget(info, this);
                m_netInfoViewWidgets.insert(info->getMac(), widget);
                m_gridSlots[row][col] = widget;

                connect(widget, &NetworkInfoViewWidget::swapRequested,
                        this, &MainWindow::handleWidgetsSwap);

                updateGridDisplay();
                return;
            }
        }
    }
    qWarning() << "Grid is full - cannot add new widget";
}

void MainWindow::handleWidgetsSwap(QWidget *source, QWidget *target)
{
    NetworkInfoViewWidget* srcWidget = qobject_cast<NetworkInfoViewWidget*>(source);
    NetworkInfoViewWidget* tgtWidget = qobject_cast<NetworkInfoViewWidget*>(target);

    if(!srcWidget || !tgtWidget) return;

    // Find positions in grid
    int srcRow = -1, srcCol = -1;
    int tgtRow = -1, tgtCol = -1;

    for(int r = 0; r < GRID_SIZE; r++)
    {
        for(int c = 0; c < GRID_SIZE; c++)
        {
            if(m_gridSlots[r][c] == srcWidget)
            {
                srcRow = r;
                srcCol = c;
            }
            if(m_gridSlots[r][c] == tgtWidget)
            {
                tgtRow = r;
                tgtCol = c;
            }
        }
    }

    if(srcRow != -1 && srcCol != -1 && tgtRow != -1 && tgtCol != -1)
    {
        m_gridSlots[srcRow][srcCol] = tgtWidget;
        m_gridSlots[tgtRow][tgtCol] = srcWidget;
        updateGridDisplay();
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

// Implement empty methods to prevent compilation warnings
void MainWindow::updateInfoViewWidgee(NetworkInfo *info) {}
void MainWindow::removeInfoViewWidge(const QString &mac) {}
