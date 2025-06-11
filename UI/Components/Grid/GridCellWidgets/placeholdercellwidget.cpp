#include "placeholdercellwidget.h"

#include <QStyle>
#include <QVBoxLayout>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QIODevice>
#include <QMimeData>

PlaceHolderCellWidget::PlaceHolderCellWidget(QWidget* parent)
    : GridCellWidget(parent),
    m_infoLabel(new QLabel(this))
{
    //setProperty("isPlaceholder", true);
    setAcceptDrops(true); // Allow drops but not drags

    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("color: #888888; font-weight: bold;");
    m_infoLabel->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_infoLabel);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);
}

void PlaceHolderCellWidget::setGridIndex(QPoint newGridIndex)
{
    GridCellWidget::setGridIndex(newGridIndex);

    if (newGridIndex == QPoint(0, 0))
    {
        m_infoLabel->setText("No Active Network");
        setProperty("bestNetworkPlaceholder", true);
    }
    else
    {
        m_infoLabel->setText("");
        setProperty("bestNetworkPlaceholder", false);
    }

    style()->unpolish(this);
    style()->polish(this);
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
        style()->unpolish(this);
        style()->polish(this);
        event->acceptProposedAction();
    }
    QFrame::dragEnterEvent(event);
}

void PlaceHolderCellWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    setProperty("dragOver", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::dragLeaveEvent(event);
}

void PlaceHolderCellWidget::dropEvent(QDropEvent* event)
{
    // if (!event->mimeData()->hasFormat("application/x-grid-index")) {
    //     event->ignore();
    //     return;
    // }

    event->acceptProposedAction();


    setProperty("dragOver", false);
    style()->unpolish(this);
    style()->polish(this);

    QByteArray receivedData = event->mimeData()->data("application/x-grid-index");
    QDataStream stream(&receivedData, QIODevice::ReadOnly);
    QPoint sourceIndex;
    stream >> sourceIndex;

    emit swapRequested(sourceIndex, getGridIndex());
    event->acceptProposedAction();

    QFrame::dropEvent(event);
}
