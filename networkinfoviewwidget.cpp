#include "networkinfoviewwidget.h"
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

#include "ledindicatordelegate.h"
#include "networkinfo.h"

NetworkInfoViewWidget::NetworkInfoViewWidget(QWidget* parent)
    : QFrame(parent),
    keyValueTbl(new QTableView(this)),
    indicatorInfoTbl(new QTableView(this))
{
    setupUI();
}

NetworkInfoViewWidget::NetworkInfoViewWidget(NetworkInfo *info, QWidget* parent)
    :m_info(info),
    QFrame(parent),
    keyValueTbl(new QTableView(this)),
    indicatorInfoTbl(new QTableView(this))
{
    setupUI();
    setKeyValueTbl();
}

NetworkInfoViewWidget::~NetworkInfoViewWidget()
{

}

void NetworkInfoViewWidget::addKeyValue(QPair<QString, QString> keyVal)
{
    QList<QStandardItem*> newRow;
    newRow << new QStandardItem(keyVal.first)
           << new QStandardItem(keyVal.second);

    QStandardItem *ledItem = new QStandardItem();

    if(keyVal.first == "Is Up:" || keyVal.first == "Is Running:")
    {
        bool state = (keyVal.second.compare("True", Qt::CaseInsensitive) == 0);
        ledItem->setData(state ? 1 : 0, Qt::UserRole); // 1=green, 0=red
    }
    else
    {
        ledItem->setData(-1, Qt::UserRole); // No indicator
    }

    newRow << ledItem;
    keyValModel->appendRow(newRow);
}

void NetworkInfoViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPos = event->pos();
    QWidget::mousePressEvent(event);
}

void NetworkInfoViewWidget::mouseMoveEvent(QMouseEvent *event)
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

void NetworkInfoViewWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dualwidget"))
        event->acceptProposedAction();
}

void NetworkInfoViewWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dualwidget"))
    {
        event->acceptProposedAction();
        QWidget *source = qobject_cast<QWidget*>(event->source());
        if (source && source != this)
            emit swapRequested(source, this);
    }
}

void NetworkInfoViewWidget::resizeKeyValTable()
{
    keyValueTbl->resizeColumnsToContents();
    keyValueTbl->resizeRowsToContents();

    QSize sizeHint = keyValueTbl->sizeHint();

    if (keyValueTbl->horizontalHeader()->isHidden())
    {
        sizeHint.setHeight(sizeHint.height() - keyValueTbl->horizontalHeader()->height());
    }
    if (keyValueTbl->verticalScrollBar()->isVisible())
    {
        sizeHint.setWidth(sizeHint.width() + keyValueTbl->verticalScrollBar()->width());
    }
    keyValueTbl->setFixedSize(sizeHint);
    //TODO:fix calculation of size
    // int totalWidth = 0;
    // totalWidth += keyValueTbl->verticalHeader()->width();
    // for (int col = 0; col < keyValModel->columnCount(); ++col)
    // {
    //     totalWidth += keyValueTbl->columnWidth(col);
    // }

    // int totalHeight = 0;
    // totalHeight += keyValueTbl->horizontalHeader()->height();
    // for (int row = 0; row < keyValModel->rowCount(); ++row)
    // {
    //     totalHeight += keyValueTbl->rowHeight(row);
    // }

    // keyValueTbl->setFixedSize(totalWidth*2, totalHeight);
}

void NetworkInfoViewWidget::setupUI()
{
    setAcceptDrops(true);
    setLayout(new QVBoxLayout(this));
    keyValModel = new QStandardItemModel(this);
    keyValueTbl->setModel(keyValModel);
    keyValueTbl->setItemDelegateForColumn(2, new LedIndicatorDelegate(this));
   // keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->setFrameShape(QFrame::Box);
    keyValueTbl->horizontalHeader()->setVisible(false);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->horizontalHeader()->setStretchLastSection(true);
    //keyValueTbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //keyValueTbl->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContentsOnFirstShow);

    indicatorInfoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    indicatorInfoTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->horizontalHeader()->setVisible(false);
    indicatorInfoTbl->verticalHeader()->setVisible(false);
    indicatorInfoTbl->setItemDelegate(new LedIndicatorDelegate(this));

    //TODO: remove later
    indicatorInfoTbl->hide();

    setStyleSheet(R"(
            NetworkView
            {
                border: 1px solid #ccc;
                border-radius: 5px;
                background-color: #f8f8f8;
            }
            QLabel { color: #333; }
        )");
}

void NetworkInfoViewWidget::setKeyValueTbl()
{
    for(const QPair<QString, QString>& keyVal: m_info->getAllKeyValuesAsList())
        addKeyValue(keyVal);
    resizeKeyValTable();
}
