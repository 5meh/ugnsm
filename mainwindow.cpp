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

    QLabel* label = new QLabel("Empty", placeholder);
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
            if(placeholder && placeholder->parentWidget())
            {
                QWidget* parentWidget = placeholder->parentWidget();
                QGridLayout* grid = qobject_cast<QGridLayout*>(parentWidget->layout());
                if(grid)
                {
                    // Find the placeholder's position by iterating through grid items
                    int targetRow = -1, targetCol = -1;
                    bool found = false;
                    for(int i = 0; i < grid->count(); ++i)
                    {
                        int r, c, rs, cs;
                        grid->getItemPosition(i, &r, &c, &rs, &cs);
                        QLayoutItem* item = grid->itemAtPosition(r, c);
                        if(item && item->widget() == placeholder)
                        {
                            targetRow = r;
                            targetCol = c;
                            found = true;
                            break;
                        }
                    }
                    if(found)
                    {
                        handleDropOnPlaceholder(
                            qobject_cast<QWidget*>(dropEvent->mimeData()->property("widget").value<QWidget*>()),
                            targetRow, targetCol
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
    if(targetRow < 0 || targetRow >= GRID_SIZE ||
        targetCol < 0 || targetCol >= GRID_SIZE)
        return;

    NetworkInfoViewWidget* sourceWidget = qobject_cast<NetworkInfoViewWidget*>(source);
    if(!sourceWidget) return;

    int sourceRow = -1, sourceCol = -1;
    for(int r = 0; r < GRID_SIZE; r++)
    {
        for(int c = 0; c < GRID_SIZE; c++)
        {
            if(m_gridSlots[r][c] == sourceWidget)
            {
                sourceRow = r;
                sourceCol = c;
                break;
            }
        }
        if(sourceRow != -1) break; // Exit outer loop once found
    }

    if(sourceRow == -1 || sourceCol == -1) return;

    // Remove any existing widget in target cell (should be a placeholder)
    QLayoutItem* targetItem = m_grid->itemAtPosition(targetRow, targetCol);
    if(targetItem && targetItem->widget())
    {
        m_grid->removeWidget(targetItem->widget());
        if(!qobject_cast<NetworkInfoViewWidget*>(targetItem->widget()))
        {
            delete targetItem->widget();
        }
    }

    // Update grid slots
    m_gridSlots[targetRow][targetCol] = sourceWidget;
    m_gridSlots[sourceRow][sourceCol] = nullptr;

    // Remove source widget from old position
    QLayoutItem* sourceItem = m_grid->itemAtPosition(sourceRow, sourceCol);
    if(sourceItem && sourceItem->widget())
    {
        m_grid->removeWidget(sourceItem->widget());
        if(!qobject_cast<NetworkInfoViewWidget*>(sourceItem->widget()))
        {
            delete sourceItem->widget();
        }
    }

    // Add source widget to target and placeholder to source
    m_grid->addWidget(sourceWidget, targetRow, targetCol);
    m_grid->addWidget(createPlaceholderWidget(), sourceRow, sourceCol);

    updateGridDisplay(); // Refresh the entire grid to ensure consistency
}

void MainWindow::updateGridDisplay()
{
    setUpdatesEnabled(false);

    QHash<QPair<int, int>, QWidget*> currentItems;
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            QLayoutItem* item = m_grid->itemAtPosition(row, col);
            if(item && item->widget())
            {
                currentItems[{row, col}] = item->widget();
            }
        }
    }

    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            QWidget* targetWidget = m_gridSlots[row][col];
            QWidget* currentWidget = currentItems.value({row, col}, nullptr);

            if(targetWidget)
            {
                if(currentWidget != targetWidget)
                {
                    // Remove existing widget if present
                    if(currentWidget)
                    {
                        m_grid->removeWidget(currentWidget);
                        if(!qobject_cast<NetworkInfoViewWidget*>(currentWidget))
                        {
                            delete currentWidget;
                        }
                    }
                    m_grid->addWidget(targetWidget, row, col);
                }
            }
            else
            {
                if(!currentWidget || qobject_cast<NetworkInfoViewWidget*>(currentWidget))
                {
                    if(currentWidget)
                        m_grid->removeWidget(currentWidget);
                    m_grid->addWidget(createPlaceholderWidget(), row, col);
                }
            }
        }
    }

    setUpdatesEnabled(true);
    arrangeGrid();
}

void MainWindow::arrangeGrid()
{
    for(int row = 0; row < GRID_SIZE; row++)
    {
        for(int col = 0; col < GRID_SIZE; col++)
        {
            if(QLayoutItem* item = m_grid->itemAtPosition(row, col))
            {
                if(QWidget* w = item->widget())
                {
                    w->setFixedSize(m_widgetSize);
                    w->updateGeometry();
                }
            }
        }
    }
    m_grid->update();
}

void MainWindow::addInfoViewWidget(NetworkInfo* info)
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
                connect(m_viewInfo, &NetworkInfoView::networkInfoUpdated, widget, &NetworkInfoViewWidget::updateNetworkInfoDisplay);

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

    for(NetworkInfo* netInfo: m_viewInfo->getNetworkInfos().values())
    {
        addInfoViewWidget(netInfo);
    }
}
