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
    m_gridLayout->setSpacing(5);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
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

    for(int row = 0; row < rows; ++row)
    {
        m_cells[row].resize(cols);
        for(int col = 0; col < cols; ++col)
        {
            PlaceHolderCellWidget* cell = new PlaceHolderCellWidget();
            cell->setObjectName(QString("%1,%2").arg(row).arg(col));
            connect(cell, &GridCellWidget::swapRequested,
                    this, &GridViewManager::handleSwapRequested);

            if (row == 0 && col == 0)
            {
                cell->setProperty("bestNetwork", true);
                cell->style()->unpolish(cell);
                cell->style()->polish(cell);
            }

            m_gridLayout->addWidget(cell, row, col);
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

    widget->setObjectName(QString("%1,%2").arg(row).arg(col));
    connect(widget, &GridCellWidget::swapRequested,
            this, &GridViewManager::handleSwapRequested);

    m_gridLayout->addWidget(widget, row, col);
    m_cells[row][col] = widget;
}

void GridViewManager::updateCell(int row, int col, NetworkInfoModel* model)
{
    if(row < 0 || row >= gridRows() || col < 0 || col >= gridCols())
        return;

    GridCellWidget* current = cellAt(row, col);

    if(model)
    {
        if(current && current->metaObject()->className() == NetworkInfoViewWidget::staticMetaObject.className())
        {
            // Update existing widget
            static_cast<NetworkInfoViewWidget*>(current)->setViewModel(model);
        }
        else
        {
            // Replace with new widget
            setCell(row, col, createCellWidgetForModel(model));
        }
    }
    else
    {
        clearCell(row, col);
    }
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

void GridViewManager::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasText())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void GridViewManager::dragMoveEvent(QDragMoveEvent* event)
{
    const QPoint pos = event->position().toPoint();
    for(int row = 0; row < m_cells.size(); ++row)
    {
        for(int col = 0; col < m_cells[row].size(); ++col)
        {
            if(m_cells[row][col]->geometry().contains(pos))
            {
                highlightCell(row, col);
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void GridViewManager::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)
    clearHighlight();
}

void GridViewManager::dropEvent(QDropEvent* event)
{
    // clearHighlight();

    // if(!event->mimeData()->hasText())
    // {
    //     event->ignore();
    //     return;
    // }

    // const QString sourceId = event->mimeData()->text();
    // const QPoint sourcePos = parseCellPosition(sourceId);
    // QPoint dropPos(-1, -1);

    // // Find drop position using actual widget geometry
    // const QPoint cursorPos = event->position().toPoint();
    // for(int row = 0; row < gridRows(); ++row)
    // {
    //     for(int col = 0; col < gridCols(); ++col)
    //     {
    //         if(cellAt(row, col)->geometry().contains(cursorPos))
    //         {
    //             dropPos = QPoint(row, col);
    //             break;
    //         }
    //     }
    // }

    // if(sourcePos.isValid() && dropPos.isValid())
    // {
    //     emit cellSwapRequested(sourcePos.x(), sourcePos.y(),
    //                            dropPos.x(), dropPos.y());
    //     event->acceptProposedAction();
    // }
    // else
    // {
    //     event->ignore();
    // }
    ////////////////
    clearHighlight();

    if(!event->mimeData()->hasText())
    {
        event->ignore();
        return;
    }

    const QString sourceId = event->mimeData()->text();
    const QPoint sourcePos = parseCellPosition(sourceId);
    const QPoint dropPos = parseCellPosition(
        cellAt(event->position().toPoint().y() / height(),
               event->position().toPoint().x() / width())->cellId()
        );

    if(sourcePos.x() >= 0 && sourcePos.y() >= 0 &&
        dropPos.x() >= 0 && dropPos.y() >= 0)
    {
        emit cellSwapRequested(sourcePos.x(), sourcePos.y(),
                               dropPos.x(), dropPos.y());
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void GridViewManager::handleSwapRequested(GridCellWidget* source, GridCellWidget* target)
{
    const QPoint sourcePos = parseCellPosition(source->cellId());
    const QPoint targetPos = parseCellPosition(target->cellId());
    emit cellSwapRequested(sourcePos.x(), sourcePos.y(),
                           targetPos.x(), targetPos.y());
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
    m_highlightedCell = cellAt(row, col);
    if (m_highlightedCell)
    {
        m_highlightedCell->setProperty("highlighted", true);
        m_highlightedCell->style()->unpolish(m_highlightedCell);
        m_highlightedCell->style()->polish(m_highlightedCell);
    }
}

void GridViewManager::clearHighlight()
{
    if (m_highlightedCell)
    {
        m_highlightedCell->setProperty("highlighted", false);
        m_highlightedCell->style()->unpolish(m_highlightedCell);
        m_highlightedCell->style()->polish(m_highlightedCell);
        m_highlightedCell = nullptr;
    }
}

QPoint GridViewManager::parseCellPosition(const QString& cellId) const
{
    QStringList parts = cellId.split(',');
    if(parts.size() == 2)
    {
        return QPoint(parts[0].toInt(), parts[1].toInt());
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

    // Connect model changes to specific widget updates
    connect(model, &NetworkInfoModel::propertyChanged,
            widget, &NetworkInfoViewWidget::updateProperty,
            Qt::QueuedConnection);

    widget->setUpdatesEnabled(true);
    return widget;
}
