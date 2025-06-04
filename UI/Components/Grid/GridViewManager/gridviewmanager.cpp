#include "gridviewmanager.h"
#include "../GridCellWidgets/gridcellwidget.h"
#include "../GridCellWidgets/networkinfoviewwidget.h"
#include "../GridCellWidgets/placeholdercellwidget.h"
#include "../Core/Network/Information/networkinfomodel.h"
#include "../Core/globalmanager.h"

#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QDebug>
#include <QStyle>

GridViewManager::GridViewManager(QWidget* parent)
    : QWidget(parent),
    m_gridLayout(new QGridLayout(this))
{
    m_gridLayout->setSpacing(10);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(m_gridLayout);
    setAcceptDrops(true);
}

GridViewManager::~GridViewManager()
{
    clearGrid();
}

void GridViewManager::setGridSize(int rows, int cols)
{
    clearGrid();
    m_cells.resize(rows);

    m_gridLayout->setSpacing(10);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);

    for(int row = 0; row < rows; ++row)
    {
        m_cells[row].resize(cols);
        for(int col = 0; col < cols; ++col)
        {
            PlaceHolderCellWidget* cell = new PlaceHolderCellWidget();
            setCell(QPoint(row, col), cell);
        }
    }
}

void GridViewManager::setCell(QPoint indx, GridCellWidget* widget)
{
    if(indx.x() < 0 || indx.x() >= m_cells.size() || indx.y() < 0 || indx.y() >= m_cells[indx.x()].size())//TODO:mb remove range check everywhere?
        return;

    GridCellWidget* oldWidget = m_cells[indx.x()][indx.y()];
    if(oldWidget)
    {
        m_gridLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }

    widget->setGridIndex(indx);

    m_gridLayout->addWidget(widget, indx.x(), indx.y());
    m_cells[indx.x()][indx.y()] = widget;

    if (indx == QPoint(0, 0))
    {
        widget->setProperty("bestNetwork", true);

        if (!isPlaceholder(widget))
            highlightCell(0, 0);
        else
            clearHighlight(0, 0);
    }
    else
        widget->setProperty("bestNetwork", false);

connect(widget, &GridCellWidget::swapRequested,
        this, &GridViewManager::handleSwapRequested);
}

void GridViewManager::updateCell(QPoint indx, NetworkInfoModel* model)
{
    if (indx.x() < 0 || indx.x() >= gridRows() || indx.y() < 0 || indx.y() >= gridCols())
        return;

    GridCellWidget* current = cellAt(indx.x(), indx.y());
    setUpdatesEnabled(false);

    if (model)
    {
        NetworkInfoViewWidget* networkWidget = qobject_cast<NetworkInfoViewWidget*>(current);
        if (networkWidget)
        {
            // Update existing widget if model is different
            if (networkWidget->getModel() != model)
            {
                networkWidget->setViewModel(model);
            }
        }
        else
        {
            // Replace placeholder with new NetworkInfoViewWidget
            auto* newWidget = createCellWidgetForModel(model);
            newWidget->setGridIndex(indx);
            setCell(indx, newWidget);
        }
    }
    else
    {
        // Replace with placeholder if not already one
        if (!qobject_cast<PlaceHolderCellWidget*>(current))
        {
            PlaceHolderCellWidget* placeholder = new PlaceHolderCellWidget(this);
            placeholder->setGridIndex(indx);
            setCell(indx, placeholder);
        }
    }
    setUpdatesEnabled(true);
}

void GridViewManager::clearCell(int row, int col)
{
    if(auto* current = cellAt(row, col)) {
        if(current->metaObject()->className() != PlaceHolderCellWidget::staticMetaObject.className())
        {
            setCell(QPoint(row, col), new PlaceHolderCellWidget(this));
        }
    }
}

GridCellWidget* GridViewManager::cellAt(int row, int col) const
{
    if(row >= 0 && row < m_cells.size() &&
        col >= 0 && col < m_cells[row].size())
    {
        return m_cells[row][col];
    }
    return nullptr;//TODO:it's imposible
}

void GridViewManager::handleSwapRequested(QPoint source, QPoint target)
{
    if (source.x() < 0 || source.x() >= gridRows() || source.y() < 0 || source.y() >= gridCols() ||
        target.x() < 0 || target.x() >= gridRows() || target.y() < 0 || target.y() >= gridCols())
        return;

    GridCellWidget* sourceWidget = cellAt(source.x(), source.y());
    GridCellWidget* targetWidget = cellAt(target.x(), target.y());

    // Check if swap involves best network position (0,0)
    if (source == QPoint(0,0) || target == QPoint(0,0))
    {
        GridCellWidget* bestNetworkCell = nullptr;
        GridCellWidget* otherCell = nullptr;
        QPoint otherPos;

        // Identify which cell is at (0,0)
        if (source == QPoint(0,0))
        {
            bestNetworkCell = sourceWidget;
            otherCell = targetWidget;
            otherPos = target;
        }
        else
        {
            bestNetworkCell = targetWidget;
            otherCell = sourceWidget;
            otherPos = source;
        }

        // Case 1: Swap between two networks (one is best network)
        if (!isPlaceholder(bestNetworkCell) && !isPlaceholder(otherCell))
        {
            const QString dialogId = "SwapWarning";
            auto result = GlobalManager::messageBoxManager()->showDialog(
                dialogId,
                "Best Network Swap Warning",
                "You are trying to swap the best network.\nDo you want to continue?",
                "Do not show this message again"
                );

            if (result == QMessageBox::No)
            {
                sourceWidget->clearHighlight();
                targetWidget->highlightCell();
                return;
            }
        }
        // Case 2: Swap best network with placeholder
        else if (!isPlaceholder(bestNetworkCell) && isPlaceholder(otherCell))
        {
            const QString dialogId = "CancelNetworkWarning";
            auto result = GlobalManager::messageBoxManager()->showDialog(
                dialogId,
                "Cancel Network Connection",
                "You are disconnecting the best network.\nDo you want to continue?",
                "Do not show this message again"
                );

            if (result == QMessageBox::No)
            {
                sourceWidget->clearHighlight();
                targetWidget->highlightCell();
                return;
            }
        }
        // Case 3: Swap placeholder at (0,0) with network
        else if (isPlaceholder(bestNetworkCell) && !isPlaceholder(otherCell))
        {
            const QString dialogId = "SetActiveWarning";
            auto result = GlobalManager::messageBoxManager()->showDialog(
                dialogId,
                "Set Active Network",
                "You are setting this network as active.\nDo you want to continue?",
                "Do not show this message again"
                );

            if (result == QMessageBox::No)
            {
                sourceWidget->clearHighlight();
                targetWidget->highlightCell();
                return;
            }
        }
    }

    if (!sourceWidget || !targetWidget)
        return;

    // Perform the swap operation
    m_gridLayout->removeWidget(sourceWidget);
    m_gridLayout->removeWidget(targetWidget);    

    m_gridLayout->addWidget(sourceWidget, target.x(), target.y());
    m_gridLayout->addWidget(targetWidget, source.x(), source.y());

    sourceWidget->setGridIndex(target);
    targetWidget->setGridIndex(source);

    std::swap(m_cells[source.x()][source.y()], m_cells[target.x()][target.y()]);

    emit cellSwapRequestToDataManager(source, target);
}

void GridViewManager::clearGrid()
{
    for(auto& row : m_cells)
    {
        for(auto cell : row)
        {
            m_gridLayout->removeWidget(cell);
            cell->deleteLater();
        }
    }
    m_cells.clear();
}

void GridViewManager::highlightCell(int row, int col)
{
    if (m_highlightedCell)
        m_highlightedCell->clearHighlight();

    auto* cell = cellAt(row, col);
    cell->highlightCell();
    m_highlightedCell = cell;
}

void GridViewManager::clearHighlight(int row, int col)
{
    if (!m_highlightedCell)
        return;
    GridCellWidget* cell = cellAt(row, col);
    if (cell && !isPlaceholder(cell))
    {
        cell->highlightCell();
        m_highlightedCell = cell;
    }
    else
        m_highlightedCell = nullptr;
}

QPoint GridViewManager::getCellIndexFromPos(const QPoint& indx)
{
    if(indx.x() >= 0 && indx.x() < gridRows() &&
        indx.y() >= 0 && indx.y() < gridCols())
    {
        return indx;
    }
    return QPoint(-1, -1);
}

void GridViewManager::updateCellContent(int row, int col, NetworkInfoModel* model)
{
    GridCellWidget* current = cellAt(row, col);

    // Handle existing NetworkInfoViewWidget updates
    if (current && qobject_cast<NetworkInfoViewWidget*>(current))
    {
        NetworkInfoViewWidget* viewWidget = static_cast<NetworkInfoViewWidget*>(current);

        // Only update if model changed
        if (viewWidget->getMac() != model->getMac())
        {
            viewWidget->setUpdatesEnabled(false);

            // Disconnect old model signals
            disconnect(viewWidget->getModel(), &NetworkInfoModel::propertyChanged,
                       viewWidget, &NetworkInfoViewWidget::updateProperty);

            // Connect to new model
            viewWidget->setViewModel(model);
            connect(model, &NetworkInfoModel::propertyChanged,
                    viewWidget, &NetworkInfoViewWidget::updateProperty,
                    Qt::QueuedConnection);

            viewWidget->setUpdatesEnabled(true);
        }
    }
    else
    {
        setCell(QPoint(row,col), createCellWidgetForModel(model));
    }
}

GridCellWidget* GridViewManager::createCellWidgetForModel(NetworkInfoModel* model)
{
    if (!model)
        return new PlaceHolderCellWidget(this);

    NetworkInfoViewWidget* widget = new NetworkInfoViewWidget(model);
    widget->setUpdatesEnabled(false);

    connect(model, &NetworkInfoModel::propertyChanged,
            widget, &NetworkInfoViewWidget::updateProperty,
            Qt::QueuedConnection);

    widget->setUpdatesEnabled(true);
    return widget;
}

bool GridViewManager::isPlaceholder(GridCellWidget *widget) const
{
    return qobject_cast<PlaceHolderCellWidget*>(widget) != nullptr;
}
