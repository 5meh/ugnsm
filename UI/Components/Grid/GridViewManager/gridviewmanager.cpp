#include "gridviewmanager.h"
#include "../GridCellWidgets/gridcellwidget.h"
#include "../GridCellWidgets/networkinfoviewwidget.h"
#include "../GridCellWidgets/placeholdercellwidget.h"
#include "../Core/Network/Information/networkinfomodel.h"

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
            cell->setGridIndex(QPoint(row, col));
            m_gridLayout->addWidget(cell, row, col, Qt::AlignCenter);
            m_cells[row][col] = cell;
        }
    }
}

void GridViewManager::setCell(QPoint indx, GridCellWidget* widget)
{
    if(indx.x() < 0 || indx.x() >= m_cells.size() || indx.y() < 0 || indx.y() >= m_cells[indx.x()].size())
        return;

    GridCellWidget* oldWidget = m_cells[indx.x()][indx.y()];
    if(oldWidget)
    {
        m_gridLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }


    widget->setGridIndex(indx);
    connect(widget, &GridCellWidget::swapRequested,
            this, &GridViewManager::handleSwapRequested);

    m_gridLayout->addWidget(widget, indx.x(), indx.y());
    m_cells[indx.x()][indx.y()] = widget;
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
    return nullptr;
}

// void GridViewManager::dragEnterEvent(QDragEnterEvent* event)
// {
//     if(event->mimeData()->hasText())
//     {
//         event->acceptProposedAction();
//     }
//     else
//     {
//         event->ignore();
//     }
// }

// void GridViewManager::dragMoveEvent(QDragMoveEvent* event)
// {
//     const QPoint pos = event->position().toPoint();
//     for(int row = 0; row < m_cells.size(); ++row)
//     {
//         for(int col = 0; col < m_cells[row].size(); ++col)
//         {
//             if(m_cells[row][col]->geometry().contains(pos))
//             {
//                 highlightCell(row, col);
//                 event->acceptProposedAction();
//                 return;
//             }
//         }
//     }
//     event->ignore();
// }

// void GridViewManager::dragLeaveEvent(QDragLeaveEvent* event)
// {
//     Q_UNUSED(event)
//     clearHighlight();
// }

// void GridViewManager::dropEvent(QDropEvent* event)
// {
//     clearHighlight();

//     QPoint sourceIndex = event->mimeData()->property("gridIndex").toPoint();
//     QPoint dropIndex = getCellIndexFromPos(event->position().toPoint());

//     if(sourceIndex != QPoint(-1, -1) && dropIndex != QPoint(-1, -1))
//     {
//         emit cellSwapRequested(sourceIndex, dropIndex);
//         event->acceptProposedAction();
//     }
//     else
//     {
//         event->ignore();
//     }
// }

void GridViewManager::handleSwapRequested(QPoint source, QPoint target)
{
    // Validate indices
    if (source.x() < 0 || source.x() >= gridRows() || source.y() < 0 || source.y() >= gridCols() ||
        target.x() < 0 || target.x() >= gridRows() || target.y() < 0 || target.y() >= gridCols())
        return;

    // Check if we're trying to swap a cell with itself
    if (source == target)
        return;

    // Get the source and target widgets
    GridCellWidget* sourceWidget = cellAt(source.x(), source.y());
    GridCellWidget* targetWidget = cellAt(target.x(), target.y());

    // Check if either widget is null
    if (!sourceWidget || !targetWidget)
        return;

    // Check if either widget is a placeholder
    bool sourceIsPlaceholder = qobject_cast<PlaceHolderCellWidget*>(sourceWidget);
    bool targetIsPlaceholder = qobject_cast<PlaceHolderCellWidget*>(targetWidget);

    // Allow swap if at least one of the cells is not a placeholder
    if (!sourceIsPlaceholder || !targetIsPlaceholder)
    {
        // Perform the swap
        m_gridLayout->removeWidget(sourceWidget);
        m_gridLayout->removeWidget(targetWidget);

        // Update the grid indices
        sourceWidget->setGridIndex(target);
        targetWidget->setGridIndex(source);

        // Swap the widgets in the layout
        m_gridLayout->addWidget(sourceWidget, target.x(), target.y());
        m_gridLayout->addWidget(targetWidget, source.x(), source.y());

        // Update the cells vector
        std::swap(m_cells[source.x()][source.y()], m_cells[target.x()][target.y()]);

        // Emit the swap request to the data manager
        emit cellSwapRequestToDataManager(source, target);
    }
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
    clearHighlight();
    auto* cell = cellAt(row, col);
    if (!cell)
        return;
    cell->setProperty("highlighted", true);
    cell->style()->unpolish(cell);
    cell->style()->polish(cell);
    m_highlightedCell = cell;
}

void GridViewManager::clearHighlight()
{
    if (!m_highlightedCell)
        return;
    m_highlightedCell->setProperty("highlighted", false);
    m_highlightedCell->style()->unpolish(m_highlightedCell);
    m_highlightedCell->style()->polish(m_highlightedCell);
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
