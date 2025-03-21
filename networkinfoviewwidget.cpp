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
#include <QPainter>
#include <QApplication>
#include <qpainter.h>

#include "ledindicatordelegate.h"
#include "networkinfo.h"

NetworkInfoViewWidget::NetworkInfoViewWidget(QWidget* parent)
    : QFrame(parent),
    keyValueTbl(new QTableView(this)),
    indicatorInfoTbl(new QTableView(this))
{
    setupUI();
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
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
     if(keyValModel) keyValModel->deleteLater();
}

QString NetworkInfoViewWidget::getMac() const
{
    return m_info ? m_info->getMac() : QString();
}

bool NetworkInfoViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    QTableView* table = qobject_cast<QTableView*>(watched->parent());
    if (table && (table == keyValueTbl || table == indicatorInfoTbl))
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton)
                {
                    dragStartPos = me->pos();
                }
                return true; // Consume the event
            }
            case QEvent::MouseMove:
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                if ((me->buttons() & Qt::LeftButton) &&
                    (me->pos() - dragStartPos).manhattanLength() >= QApplication::startDragDistance())
                {
                    // Forward the drag event to the widget
                    QMouseEvent* newEvent = new QMouseEvent(
                        QEvent::MouseMove,
                        me->localPos(),
                        me->windowPos(),
                        me->screenPos(),
                        me->button(),
                        me->buttons(),
                        me->modifiers()
                        );
                    QApplication::postEvent(this, newEvent);
                }
                return true; // Consume the event
            }
            default:
                return false;
        }
    }
    return QWidget::eventFilter(watched, event);
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
    if (event->button() == Qt::LeftButton &&
          !keyValueTbl->geometry().contains(event->pos()) &&
          !indicatorInfoTbl->geometry().contains(event->pos()))
    {
        dragStartPos = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void NetworkInfoViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;
    if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance()) return;

    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData;

    // Set both standard and custom MIME data
    mime->setText("NetworkInfoWidget");
    mime->setData("application/x-networkinfoviewwidget", QByteArray());
    mime->setProperty("widget", QVariant::fromValue<QWidget*>(this));

    // Create drag pixmap with visual feedback
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    render(&pixmap);

    // Add drop target indicator
    QPainter painter(&pixmap);
    painter.setBrush(QColor(0, 255, 0, 50)); // Green placeholder
    painter.drawRect(rect());

    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - rect().topLeft());
    drag->setMimeData(mime);

    Qt::DropAction result = drag->exec(Qt::MoveAction);
}

void NetworkInfoViewWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-networkinfoviewwidget") &&
        event->source() != this)
    {
        event->acceptProposedAction();
        setStyleSheet("background-color: rgba(0, 255, 0, 50); border: 2px dashed darkgreen;");
        update();
    }
}

void NetworkInfoViewWidget::dropEvent(QDropEvent *event)
{
    setStyleSheet(""); // Reset visual feedback
    update();

    if (event->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
    {
        QWidget *sourceWidget = qobject_cast<QWidget*>(event->source());
        NetworkInfoViewWidget *source = qobject_cast<NetworkInfoViewWidget*>(
            event->mimeData()->property("widget").value<QWidget*>()
            );

        if (source && source != this)
        {
            // Validate MAC address before swap
            if (!source->getMac().isEmpty() && !this->getMac().isEmpty())
            {
                emit swapRequested(source, this);
                event->acceptProposedAction();
            }
        }
    }
}

void NetworkInfoViewWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    setStyleSheet("");
    update();
    QWidget::dragLeaveEvent(event);
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
}

void NetworkInfoViewWidget::setupUI()
{
    setAcceptDrops(true);
    setLayout(new QVBoxLayout(this));

    keyValModel = new QStandardItemModel(this);

    keyValueTbl->setModel(keyValModel);
    keyValueTbl->setItemDelegateForColumn(2, new LedIndicatorDelegate(this));
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->setFrameShape(QFrame::Box);
    keyValueTbl->horizontalHeader()->setVisible(false);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->horizontalHeader()->setStretchLastSection(true);

    keyValueTbl->setSelectionMode(QAbstractItemView::NoSelection);
    keyValueTbl->setFocusPolicy(Qt::NoFocus);
    keyValueTbl->setStyleSheet(
        "QTableView { selection-background-color: transparent; }"
        "QTableView::item:selected { background: transparent; }"
        );
    keyValueTbl->viewport()->installEventFilter(this);

    //keyValueTbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //keyValueTbl->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContentsOnFirstShow);

    indicatorInfoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    indicatorInfoTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->horizontalHeader()->setVisible(false);
    indicatorInfoTbl->verticalHeader()->setVisible(false);
    indicatorInfoTbl->setItemDelegate(new LedIndicatorDelegate(this));

    indicatorInfoTbl->setSelectionMode(QAbstractItemView::NoSelection);
    indicatorInfoTbl->setFocusPolicy(Qt::NoFocus);
    indicatorInfoTbl->setStyleSheet(
        "QTableView { selection-background-color: transparent; }"
        "QTableView::item:selected { background: transparent; }"
        );


    indicatorInfoTbl->viewport()->installEventFilter(this);
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
