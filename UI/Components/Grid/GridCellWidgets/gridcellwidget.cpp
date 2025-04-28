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
    if(!(event->buttons() & Qt::LeftButton)) return;

    if((event->pos() - m_dragStartPos).manhattanLength()
        >= QApplication::startDragDistance()) {
        QDrag* drag = new QDrag(this);
        QMimeData* mime = new QMimeData;
        mime->setText(cellId());

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

