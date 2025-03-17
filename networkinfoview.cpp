#include "networkinfoview.h"
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
//#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

//#include "ledindicator.h"
#include "ledindicatordelegate.h"

NetworkInfoView::NetworkInfoView(QWidget* parent):
    QWidget(parent),
    keyValueTbl(new QTableView(this)),
    indicatorInfoTbl(new QTableView(this))
{
    setAcceptDrops(true);
    // setSelectionMode(QAbstractItemView::NoSelection);
    // setDragDropMode(QAbstractItemView::DragDrop);
    // setDragEnabled(true);
    // setDropIndicatorShown(true);
    // setDefaultDropAction(Qt::MoveAction);

    setLayout(new QVBoxLayout(this));
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //keyValueTbl->resizeColumnsToContents();
    keyValueTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->setFrameShape(QFrame::NoFrame);
    keyValueTbl->horizontalHeader()->setVisible(false);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    indicatorInfoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    indicatorInfoTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->horizontalHeader()->setVisible(false);
    indicatorInfoTbl->verticalHeader()->setVisible(false);
    indicatorInfoTbl->setItemDelegate(new LedIndicatorDelegate(this));

    keyValModel = new QStandardItemModel(this);
    keyValueTbl->setModel(keyValModel);
    keyValueTbl->setItemDelegateForColumn(2, new LedIndicatorDelegate(this));
    //TODO:add indicator inicialization

    layout()->addWidget(keyValueTbl);
    layout()->addWidget(indicatorInfoTbl);
    //TODO: remove later
    indicatorInfoTbl->hide();
}

NetworkInfoView::~NetworkInfoView()
{

}

void NetworkInfoView::addKeyValue(QPair<QString, QString> keyVal)
{
    QList<QStandardItem*> newRow;
    if(keyVal.first == "is Up:" || keyVal.first == "is Running:")
    {

        newRow << new QStandardItem(QString(keyVal.first)) << new QStandardItem(QString(keyVal.second)) <<
            new QStandardItem(Qt::UserRole);
    }
    else
        newRow << new QStandardItem(QString(keyVal.first)) << new QStandardItem(QString(keyVal.second)) << new QStandardItem("");
    keyValModel->appendRow(newRow);
    resizeKeyValTable();
}

void NetworkInfoView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPos = event->pos();
    QWidget::mousePressEvent(event);
}

void NetworkInfoView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;
    if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance()) return;

    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData;
    mime->setData("application/x-dualwidget", QByteArray());
    drag->setMimeData(mime);

    QPixmap pixmap(size());
    render(&pixmap);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos());
    drag->exec(Qt::MoveAction);
}

void NetworkInfoView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dualwidget"))
        event->acceptProposedAction();
}

void NetworkInfoView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dualwidget")) {
        event->acceptProposedAction();
        QWidget *source = qobject_cast<QWidget*>(event->source());
        if (source && source != this)
            emit swapRequested(source, this);
    }
}

void NetworkInfoView::resizeKeyValTable()
{
    //TODO:fix calculation of size
    int totalWidth = 0;
    // Add width of vertical header even if hidden (it may contribute if not set to 0)
    totalWidth += keyValueTbl->verticalHeader()->width();
    for (int col = 0; col < keyValModel->columnCount(); ++col)
    {
        totalWidth += keyValueTbl->columnWidth(col);
    }

    int totalHeight = 0;
    // Add height of horizontal header even if hidden
    totalHeight += keyValueTbl->horizontalHeader()->height();
    for (int row = 0; row < keyValModel->rowCount(); ++row)
    {
        totalHeight += keyValueTbl->rowHeight(row);
    }

    // Set fixed size based on calculated dimensions
    keyValueTbl->setFixedSize(totalWidth * 2, totalHeight);
}
