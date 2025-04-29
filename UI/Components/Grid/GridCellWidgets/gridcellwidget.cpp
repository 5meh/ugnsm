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
        QMimeData* mime = new QMimeData;
        const QPoint idx = getGridIndex();
        mime->setData("application/x-grid-index", QByteArray::number(idx.x()) + ',' + QByteArray::number(idx.y()));

        // Create drag pixmap with transparency
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

    if(event->mimeData()->hasFormat("application/x-grid-index"))
    {
        setProperty("dragOver", false);
        //style()->unpolish(this);
        style()->polish(this);
        emit swapRequested(nullptr, this);
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

