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
    setAcceptDrops(true);
    setupUI();
}

void PlaceHolderCellWidget::setGridIndex(QPoint newGridIndex)
{
    GridCellWidget::setGridIndex(newGridIndex);

    if (newGridIndex == QPoint(0, 0))
    {
        m_infoLabel->setText("No Active Network\nDrag networks here");
        setProperty("bestNetworkPlaceholder", true);
    }
    else
    {
        m_infoLabel->setText("Empty Slot\nDrag network here");
        setProperty("bestNetworkPlaceholder", false);
    }

    style()->unpolish(this);
    style()->polish(this);
}

void PlaceHolderCellWidget::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

void PlaceHolderCellWidget::mouseMoveEvent(QMouseEvent* event)
{
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

void PlaceHolderCellWidget::setupUI()
{
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("color: #888888; font-weight: bold;");
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setText("Empty Slot\nDrag network here");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_infoLabel);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);
}
