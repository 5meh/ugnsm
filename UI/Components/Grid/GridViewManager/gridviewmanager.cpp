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

void GridViewManager::setCell(int row, int col, GridCellWidget* widget)
{
    if(row < 0 || row >= m_cells.size() || col < 0 || col >= m_cells[row].size())
        return;

    GridCellWidget* oldWidget = m_cells[row][col];
    if(oldWidget)
    {
        m_gridLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }


    widget->setGridIndex(QPoint(row, col));
    connect(widget, &GridCellWidget::swapRequested,
            this, &GridViewManager::handleSwapRequested);

    m_gridLayout->addWidget(widget, row, col);
    m_cells[row][col] = widget;
}

void GridViewManager::updateCell(int row, int col, NetworkInfoModel* model)
{
    if (row < 0 || row >= gridRows() || col < 0 || col >= gridCols())
        return;

    GridCellWidget* current = cellAt(row, col);
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
            newWidget->setGridIndex(QPoint(row, col));
            setCell(row, col, newWidget);
        }
    }
    else
    {
        // Replace with placeholder if not already one
        if (!qobject_cast<PlaceHolderCellWidget*>(current))
        {
            PlaceHolderCellWidget* placeholder = new PlaceHolderCellWidget(this);
            placeholder->setGridIndex(QPoint(row, col));
            setCell(row, col, placeholder);
        }
    }
    setUpdatesEnabled(true);
}

void GridViewManager::clearCell(int row, int col)
{
    if(auto* current = cellAt(row, col)) {
        if(current->metaObject()->className() != PlaceHolderCellWidget::staticMetaObject.className())
        {
            setCell(row, col, new PlaceHolderCellWidget(this));
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
        setCell(row, col, createCellWidgetForModel(model));
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
