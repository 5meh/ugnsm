#include "placeholdercellwidget.h"

#include <QStyle>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QIODevice>
#include <QMimeData>

PlaceHolderCellWidget::PlaceHolderCellWidget(QWidget* parent)
    : GridCellWidget(parent)
{
    setProperty("isPlaceholder", true);
    setAcceptDrops(true); // Allow drops but not drags
}

void PlaceHolderCellWidget::mousePressEvent(QMouseEvent* event)
{
    // Don't start drag for placeholder cells
    Q_UNUSED(event);
}

void PlaceHolderCellWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Don't allow dragging for placeholder cells
    Q_UNUSED(event);
}

void PlaceHolderCellWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-grid-index"))
    {
        setProperty("dragOver", true);
        style()->polish(this);
        event->acceptProposedAction();
    }
    QFrame::dragEnterEvent(event);
}

void PlaceHolderCellWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    setProperty("dragOver", false);
    style()->polish(this);
    QFrame::dragLeaveEvent(event);
}

void PlaceHolderCellWidget::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasFormat("application/x-grid-index")) {
        event->ignore();
        return;
    }

    setProperty("dragOver", false);
    style()->polish(this);

    QByteArray receivedData = event->mimeData()->data("application/x-grid-index");
    QDataStream stream(&receivedData, QIODevice::ReadOnly);
    QPoint sourceIndex;
    stream >> sourceIndex;

    emit swapRequested(sourceIndex, getGridIndex());
    event->acceptProposedAction();

    QFrame::dropEvent(event);
}
