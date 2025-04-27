#include "gridcellwidget.h"

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>
#include <QStyle>

GridCellWidget::GridCellWidget(QWidget* parent)
    :QFrame(parent)
{
    setAcceptDrops(true);
}

GridCellWidget::~GridCellWidget()
{

}

void GridCellWidget::mousePressEvent(QMouseEvent *event)
{
    setProperty("dragStartPos", event->pos());
    QFrame::mousePressEvent(event);
}

void GridCellWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint startPos = property("dragStartPos").toPoint();
    if ((event->pos() - startPos).manhattanLength() >= QApplication::startDragDistance())
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        // Use the cell's identifier as MIME data.
        mimeData->setText(cellId());
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        render(&pixmap);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos());

        emit dragInitiated(this);
        // if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
        // {
        //     // Optionally perform additional logic on successful drag.
        // }
    }
    QFrame::mouseMoveEvent(event);
}

void GridCellWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
    {
        setProperty("dragOver", true);
        style()->unpolish(this);
        style()->polish(this);
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void GridCellWidget::dropEvent(QDropEvent *event)
{

    if(event->mimeData()->hasText())
    {
        QString sourceId = event->mimeData()->text();
        qDebug() << "GridCellWidget (" << cellId() << ") received drop from:" << sourceId;
        // In a real scenario, you might look up the source cell widget by comparing IDs.
        // Here we simply emit swapRequested with a dummy target (this cell).
        setProperty("dragOver", false);
        style()->unpolish(this);
        style()->polish(this);
        emit swapRequested(nullptr, this);
        emit dropReceived(this);
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
    QFrame::dropEvent(event);
}

void GridCellWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    setProperty("dragOver", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::dragLeaveEvent(event);
}

