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
    if(!(event->buttons() & Qt::LeftButton))
        return;

    if((event->pos() - m_dragStartPos).manhattanLength()
        >= QApplication::startDragDistance())
    {
        QDrag* drag = new QDrag(this);
        QMimeData *mime = new QMimeData;

        QByteArray indexData;
        QDataStream stream(&indexData, QIODevice::WriteOnly);
        stream << getGridIndex();
        mime->setData("application/x-grid-index", indexData);

        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent);
        render(&pixmap);
        drag->setPixmap(pixmap);
        drag->setMimeData(mime);
        drag->exec(Qt::MoveAction);
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
    if (!event->mimeData()->hasFormat("application/x-grid-index"))
    {
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

