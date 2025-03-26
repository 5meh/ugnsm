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
#include <QList>
#include <QApplication>

#include "ledindicatordelegate.h"
//#include "networkinfo.h"
#include "networkinfoviewmodel.h"

NetworkInfoViewWidget::NetworkInfoViewWidget(NetworkInfoViewModel *viewModel, QWidget *parent)
    : QFrame(parent), m_viewModel(viewModel)
{
    setFixedSize(400, 300);
    setupUI();
    setStyleSheet("background-color: white; border-radius: 8px;");
}


NetworkInfoViewWidget::~NetworkInfoViewWidget()
{
    if(keyValModel)
        keyValModel->deleteLater();
}

QString NetworkInfoViewWidget::getMac() const
{
    return m_viewModel->getMac();
}

void NetworkInfoViewWidget::updateNetworkInfoDisplay()
{
    if(!m_viewModel)
        return;

    setUpdatesEnabled(false);
    keyValueTbl->setUpdatesEnabled(false);

    //keyValModel->clear();
    QList<QPair<QString, QString>> currentData = m_viewModel->getAllKeyValuesAsList();

    for(int row = 0; row < keyValModel->rowCount(); ++row)
    {
        QStandardItem* paramItem = keyValModel->item(row, 0);
        QStandardItem* valueItem = keyValModel->item(row, 1);
        QStandardItem* statusItem = keyValModel->item(row, 2);

        auto it = std::find_if(currentData.begin(), currentData.end(),
                               [&](const QPair<QString, QString>& item)
                                {
                                    return item.first == paramItem->text();
                                });

        if(it != currentData.end())
        {
            valueItem->setText(it->second);
            updateStatusIndicator(statusItem, it->first, it->second);
            currentData.erase(it);
        }
        else
        {
            keyValModel->removeRow(row--);
        }
    }

    setUpdatesEnabled(true);
    keyValueTbl->setUpdatesEnabled(true);
    //resizeKeyValTable();
}

// bool NetworkInfoViewWidget::eventFilter(QObject* watched, QEvent* event)
// {
//     QTableView* table = qobject_cast<QTableView*>(watched->parent());
//     if (table && (table == keyValueTbl))
//     {
//         switch (event->type())
//         {
//             case QEvent::MouseButtonPress:
//             {
//                 QMouseEvent* me = static_cast<QMouseEvent*>(event);
//                 if (me->button() == Qt::LeftButton)
//                 {
//                     dragStartPos = me->pos();
//                 }
//                 return true;
//             }
//             case QEvent::MouseMove:
//             {
//                 QMouseEvent* me = static_cast<QMouseEvent*>(event);
//                 if ((me->buttons() & Qt::LeftButton) &&
//                     (me->pos() - dragStartPos).manhattanLength() >= QApplication::startDragDistance())
//                 {
//                     // Forward the drag event to the widget
//                     QMouseEvent* newEvent = new QMouseEvent(
//                         QEvent::MouseMove,
//                         me->position(),
//                         me->scenePosition(),
//                         me->globalPosition(),
//                         me->button(),
//                         me->buttons(),
//                         me->modifiers()
//                         );
//                     QApplication::postEvent(this, newEvent);
//                 }
//                 return true;
//             }
//             default:
//                 return false;
//         }
//     }
//     return QWidget::eventFilter(watched, event);
// }

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
        ledItem->setData(-1, Qt::UserRole);
    }

    newRow << ledItem;
    keyValModel->appendRow(newRow);
}

void NetworkInfoViewWidget::setupTableView()
{
    keyValueTbl = new QTableView(this);
    keyValModel = new QStandardItemModel(this);

    keyValueTbl->setModel(keyValModel);
    keyValueTbl->setItemDelegateForColumn(2, new LedIndicatorDelegate(this));
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->setSelectionMode(QAbstractItemView::NoSelection);
    keyValueTbl->verticalHeader()->hide();
    keyValueTbl->horizontalHeader()->setStretchLastSection(true);
    keyValueTbl->setShowGrid(false);
    //keyValModel->setHorizontalHeaderLabels({"Property", "Value"});
    setKeyValueTbl();
    connectViewModel();
    keyValueTbl->viewport()->installEventFilter(this);
    layout()->addWidget(keyValueTbl);
}

void NetworkInfoViewWidget::connectViewModel()
{
    connect(m_viewModel, &NetworkInfoViewModel::nameChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoViewModel::macChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoViewModel::ipAddressChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoViewModel::netmaskChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoViewModel::speedChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
}

QString NetworkInfoViewWidget::formatSpeed(quint64 bytes) const
{
    const QStringList units = {"B/s", "KB/s", "MB/s", "GB/s"};
    int unitIndex = 0;
    double speed = bytes;

    while (speed >= 1024 && unitIndex < units.size() - 1)
    {
        speed /= 1024;
        unitIndex++;
    }

    return QString("%1 %2").arg(speed, 0, 'f', unitIndex > 0 ? 2 : 0).arg(units[unitIndex]);
}

void NetworkInfoViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton &&
          !keyValueTbl->geometry().contains(event->pos()))
    {
        dragStartPos = event->pos();
        m_isDragging = false;
    }
    QWidget::mousePressEvent(event);
}

void NetworkInfoViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance())
        return;



    m_isDragging = true;
    update();

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-networkwidget", QByteArray());
    drag->setMimeData(mimeData);

    QPixmap pixmap(size());
    render(&pixmap);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos());

    emit dragInitiated(this);
    drag->exec(Qt::MoveAction);
    m_isDragging = false;
    update();

    // QDrag* drag = new QDrag(this);
    // QMimeData* mime = new QMimeData;

    // // Set both standard and custom MIME data
    // mime->setText("NetworkInfoWidget");
    // mime->setData("application/x-networkinfoviewwidget", QByteArray());
    // mime->setProperty("widget", QVariant::fromValue<QWidget*>(this));

    // QPixmap pixmap(size());
    // pixmap.fill(Qt::transparent);
    // render(&pixmap);

    // QPainter painter(&pixmap);
    // painter.setBrush(QColor(0, 255, 0, 50)); // Green placeholder
    // painter.drawRect(rect());

    // drag->setPixmap(pixmap);
    // drag->setHotSpot(event->pos() - rect().topLeft());
    // drag->setMimeData(mime);

    // Qt::DropAction result = drag->exec(Qt::MoveAction);
}

void NetworkInfoViewWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-networkwidget"))
    {
        event->acceptProposedAction();
        update();
    }

    // if (event->mimeData()->hasFormat("application/x-networkinfoviewwidget") &&
    //     event->source() != this)
    // {
    //     event->acceptProposedAction();
    //     setStyleSheet("background-color: rgba(0, 255, 0, 50); border: 2px dashed darkgreen;");
    //     update();
    // }
}

void NetworkInfoViewWidget::dropEvent(QDropEvent* event)
{

    if (event->mimeData()->hasFormat("application/x-networkwidget"))
    {
        emit dropReceived(this);
        event->acceptProposedAction();
    }
    setStyleSheet(""); // Reset visual feedback
    update();

    // if (event->mimeData()->hasFormat("application/x-networkinfoviewwidget"))
    // {
    //     QWidget* sourceWidget = qobject_cast<QWidget*>(event->source());
    //     NetworkInfoViewWidget* source = qobject_cast<NetworkInfoViewWidget*>(
    //         event->mimeData()->property("widget").value<QWidget*>()
    //         );

    //     if (source && source != this)
    //     {
    //         // Validate MAC address before swap
    //         if (!source->getMac().isEmpty() && !this->getMac().isEmpty())
    //         {
    //             emit swapRequested(source, this);
    //             event->acceptProposedAction();
    //         }
    //     }
    // }
}

void NetworkInfoViewWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    setStyleSheet("");
    update();
    QWidget::dragLeaveEvent(event);
}

void NetworkInfoViewWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(m_isDragging ? m_dragBorder : m_normalBorder, 2);
    painter.setPen(pen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void NetworkInfoViewWidget::updateStatusIndicator(QStandardItem *item, const QString &key, const QString &value)
{
    if(key == "Is Up:" || key == "Is Running:")
    {
        bool state = (value.compare("True", Qt::CaseInsensitive) == 0);
        item->setData(state ? 1 : 0, Qt::UserRole);
    }
    else
    {
        item->setData(-1, Qt::UserRole);
    }
}

// void NetworkInfoViewWidget::resizeKeyValTable()
// {
//     //TODO:make proper table calculation
//     // keyValueTbl->resizeColumnsToContents();
//     // keyValueTbl->resizeRowsToContents();

//     // QSize sizeHint = keyValueTbl->sizeHint();

//     // if (keyValueTbl->horizontalHeader()->isHidden())
//     // {
//     //     sizeHint.setHeight(sizeHint.height() - keyValueTbl->horizontalHeader()->height());
//     // }
//     // if (keyValueTbl->verticalScrollBar()->isVisible())
//     // {
//     //     sizeHint.setWidth(sizeHint.width() + keyValueTbl->verticalScrollBar()->width());
//     // }
//     // keyValueTbl->setFixedSize(sizeHint);
//     //keyValueTbl->setFixedSize(m_widgetSize);
// }

void NetworkInfoViewWidget::setupUI()
{
    setAcceptDrops(true);
    setLayout(new QVBoxLayout(this));
    setupTableView();
    // QPixmap pixmap(":/resources/crown.png");
    // crownLbl.setPixmap(pixmap);
    // crownLbl.setScaledContents(true);
    // crownLbl.setFixedWidth(50);
    // crownLbl.setFixedHeight(50);
    //layout()->addWidget(&crownLbl);

    keyValueTbl->viewport()->installEventFilter(this);

    layout()->setSpacing(0);
}

void NetworkInfoViewWidget::setKeyValueTbl()
{
    if(m_viewModel)
    {
        for(const auto& keyVal : m_viewModel->getAllKeyValuesAsList())
        {
            addKeyValue(keyVal);
        }
    }
    //resizeKeyValTable();
}
