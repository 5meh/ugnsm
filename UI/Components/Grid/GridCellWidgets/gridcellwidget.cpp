#include "gridcellwidget.h"

#include "../GridViewManager/gridviewmanager.h"
#include "../globalmanager.h"

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QPointer>

GridCellWidget::GridCellWidget(QWidget* parent)
    :QFrame(parent)
{
    setAcceptDrops(true);
    resize(sizeHint());
}

GridCellWidget::~GridCellWidget()
{

}

QSize GridCellWidget::sizeHint() const
{
    return m_widgetSize;
}

void GridCellWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragStartPos = event->pos();
    }
    QFrame::mousePressEvent(event);
}

void GridCellWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance())
        return;

    if (!GlobalManager::dragManager()->startDrag(getGridIndex()))
        return;

    QPointer<GridViewManager> gridManager;
    if (QWidget* parent = parentWidget())
    {
        if (QWidget* grandParent = parent->parentWidget())
            gridManager = qobject_cast<GridViewManager*>(grandParent);
    }

    if (gridManager)
    {
        GlobalManager::dragManager()->startDrag(getGridIndex());
        gridManager->setUpdatesEnabled(false);
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    // Store the grid index in the mime data
    QByteArray indexData;
    QDataStream stream(&indexData, QIODevice::WriteOnly);
    stream << getGridIndex();
    mimeData->setData("application/x-grid-index", indexData);

    // Create a semi-transparent drag pixmap
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    render(&pixmap);
    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->pos() - rect().topLeft());

    Qt::DropAction result = drag->exec(Qt::MoveAction);

    // QDragLeaveEvent dragEndEvent;
    // QCoreApplication::sendEvent(this, &dragEndEvent);

    // If the drag was cancelled, ensure we clean up properly
    if (result == Qt::IgnoreAction)
    {
        // Explicitly reset drag state
        QDragLeaveEvent cancelEvent;
        QCoreApplication::sendEvent(this, &cancelEvent);
    }

    if (gridManager)
    {
        GlobalManager::dragManager()->endDrag(getGridIndex());
        gridManager->setUpdatesEnabled(true);
    }
}

void GridCellWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("application/x-grid-index"))
    {
        setProperty("dragOver", true);
        style()->polish(this);
        event->acceptProposedAction();
    }
    QFrame::dragEnterEvent(event);
}

void GridCellWidget::dropEvent(QDropEvent *event)
{
    // if (!event->mimeData()->hasFormat("application/x-grid-index"))
    // {
    //     event->ignore();
    //     return;
    // }

    event->acceptProposedAction();

    setProperty("dragOver", false);
    style()->polish(this);

    QByteArray receivedData = event->mimeData()->data("application/x-grid-index");
    QDataStream stream(&receivedData, QIODevice::ReadOnly);
    QPoint sourceIndex;
    stream >> sourceIndex;
    if(sourceIndex != getGridIndex())
        emit swapRequested(sourceIndex, getGridIndex());
    event->acceptProposedAction();

    QFrame::dropEvent(event);
}

void GridCellWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    setProperty("dragOver", false);
    style()->polish(this);
    QFrame::dragLeaveEvent(event);
}

QPoint GridCellWidget::getGridIndex() const
{
    return m_gridIndex;
}

void GridCellWidget::setGridIndex(QPoint newGridIndex)
{
    if (m_gridIndex == newGridIndex)
        return;
    m_gridIndex = newGridIndex;
    emit gridIndexChanged();//TODO; mb change
}

void GridCellWidget::highlightCell()
{
    clearHighlight();
    setProperty("highlighted", true);
    style()->polish(this);
}

void GridCellWidget::clearHighlight()
{
    setProperty("highlighted", false);
    style()->polish(this);
}

