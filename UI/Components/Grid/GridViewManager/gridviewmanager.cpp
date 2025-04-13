#include "gridviewmanager.h"
#include "gridcellwidget.h"
#include "networkinfoviewwidget.h"
#include "placeholdercellwidget.h"

#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QDebug>

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

            // Highlight best network at (0,0)
            if(row == 0 && col == 0)
            {
                cell->setStyleSheet("border: 3px solid green;");
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
        dropPos.x() >= 0 && dropPos.y() >= 0) {
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
    if(m_highlightedCell)
    {
        m_highlightedCell->setStyleSheet(
            m_highlightedCell->styleSheet() + "border: 2px dashed #0078d4;"
            );
    }
}

void GridViewManager::clearHighlight()
{
    if(m_highlightedCell)
    {
        QString style = m_highlightedCell->styleSheet();
        style.replace("border: 2px dashed #0078d4;", "");
        m_highlightedCell->setStyleSheet(style);
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
