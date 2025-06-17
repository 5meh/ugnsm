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
#include <QTimer>
#include <QThread>

#include "../../../../Utilities/Delegates/ledindicatordelegate.h"
#include "../../../../Utilities/LedIndicator/ledindicator.h"
#include "../../../../Utilities/Logger/logger.h"
//#include "networkinfo.h"
#include "../../../../Core/Network/Information/networkinfomodel.h"

NetworkInfoViewWidget::NetworkInfoViewWidget(QSharedPointer<NetworkInfoModel> viewModel, QFrame* parent)
    : GridCellWidget(parent),
    m_viewModel(viewModel)
{
    setupUI();

    m_propertyRowMap =
        {
            {"Interface", 0},
            {"MAC Address", 1},
            {"IP Address", 2},
            {"Netmask", 3},
            {"Status", 4},
            {"Download Speed", 5},
            {"Upload Speed", 6},
            {"Total Speed", 7},
            {"Last Update", 8}
        };
}

NetworkInfoViewWidget::~NetworkInfoViewWidget()
{
    if(keyValModel)
        keyValModel->deleteLater();
}

void NetworkInfoViewWidget::setViewModel(QSharedPointer<NetworkInfoModel> model)
{
    if (m_viewModel.data() == model.data() || !m_viewModel.data())
        return;

        disconnect(m_viewModel.get(), &NetworkInfoModel::propertiesChanged,
                   this, &NetworkInfoViewWidget::handlePropertiesChanged);

    m_viewModel = model;

        connect(m_viewModel.get(), &NetworkInfoModel::propertiesChanged,
                this, &NetworkInfoViewWidget::handlePropertiesChanged);

    // Full UI refresh
    updateNetworkInfoDisplay();
}

const QSharedPointer<NetworkInfoModel> NetworkInfoViewWidget::getModel() const
{
    return m_viewModel.get() ? m_viewModel : nullptr;
}

void NetworkInfoViewWidget::updateProperty(const QString& propertyName)
{
    if (!m_viewModel)
        return;

    setUpdatesEnabled(false);

    const QString displayName = m_viewModel->propertyMap().key(propertyName);
    const QPair<QString, QString> data = m_viewModel->getKeyValue(displayName);

    for (int row = 0; row < keyValModel->rowCount(); ++row)
    {
        QStandardItem* paramItem = keyValModel->item(row, 0);
        if (paramItem->text() == data.first)
        {
            QStandardItem* valueItem = keyValModel->item(row, 1);
            valueItem->setText(data.second);

            QStandardItem* statusItem = keyValModel->item(row, 2);
            updateStatusIndicator(statusItem, data.first, data.second);

            break;
        }
    }    

    setProperty("updating", true);
    style()->unpolish(this);
    style()->polish(this);
    adjustSize();

    setUpdatesEnabled(true);
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
    fitTableToContents();
    adjustSize();
    setUpdatesEnabled(true);
    keyValueTbl->setUpdatesEnabled(true);
}

void NetworkInfoViewWidget::handlePropertiesChanged(const QStringList& propertiesList)
{
    for (const QString& property : propertiesList)
    {
        if (property == "downloadSpeed" ||
            property == "uploadSpeed" ||
            property == "totalSpeed")
            updateSpeedIndicators();
        else
            updateProperty(property);
    }
    update();
}

void NetworkInfoViewWidget::addKeyValue(QPair<QString, QString> keyVal)
{
    QList<QStandardItem*> newRow;
    newRow << new QStandardItem(keyVal.first)
           << new QStandardItem(keyVal.second);

    QStandardItem *ledItem = new QStandardItem();

    if(keyVal.first == "Status")
    {
        bool state = (keyVal.second.compare("Connected", Qt::CaseInsensitive) == 0);
        ledItem->setData(state ? 2 : 0, Qt::UserRole); // 1=green, 0=red
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

    LedIndicatorDelegate* ledDelegate = new LedIndicatorDelegate(this);
    keyValueTbl->setItemDelegateForColumn(2, ledDelegate);

    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->setSelectionMode(QAbstractItemView::NoSelection);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->horizontalHeader()->setVisible(false);

    keyValueTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    keyValueTbl->setShowGrid(false);
    keyValueTbl->setFocusPolicy(Qt::NoFocus);
    keyValueTbl->setAttribute(Qt::WA_TransparentForMouseEvents);

    keyValueTbl->setAttribute(Qt::WA_TranslucentBackground);
    keyValueTbl->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    keyValueTbl->viewport()->installEventFilter(this);

    setKeyValueTbl();
    fitTableToContents();
    connectViewModel();

    layout()->addWidget(keyValueTbl);
}

void NetworkInfoViewWidget::connectViewModel()
{
    connect(m_viewModel.get(), &NetworkInfoModel::nameChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel.get(), &NetworkInfoModel::macChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel.get(), &NetworkInfoModel::ipAddressChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel.get(), &NetworkInfoModel::netmaskChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel.get(), &NetworkInfoModel::speedChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
}

void NetworkInfoViewWidget::fitTableToContents()
{
    keyValueTbl->resizeRowsToContents();
    keyValueTbl->resizeColumnsToContents();

    int totalWidth = keyValueTbl->verticalHeader()->width();
    for (int c = 0; c < keyValModel->columnCount(); ++c)
        totalWidth += keyValueTbl->columnWidth(c);

    int totalHeight = keyValueTbl->horizontalHeader()->height();
    for (int r = 0; r < keyValModel->rowCount(); ++r)
        totalHeight += keyValueTbl->rowHeight(r);

    keyValueTbl->setFixedSize(totalWidth, totalHeight - 150);//TODO:magic number, cause currently can't fix it
}

bool NetworkInfoViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == keyValueTbl->viewport())
    {
        // Block all mouse events
        switch(event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::Wheel:
        case QEvent::ContextMenu:
            return true;
        default:
            break;
        }
    }
    return GridCellWidget::eventFilter(watched, event);
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

void NetworkInfoViewWidget::updateSpeedIndicators()
{
    setUpdatesEnabled(false);
    QString downloadKey = m_propertyRowMap.key(5);  // "Download Speed"
    QString uploadKey = m_propertyRowMap.key(6);    // "Upload Speed"

    QPair<QString, QString> rxSpeed = m_viewModel->getKeyValue(downloadKey);
    QPair<QString, QString> txSpeed = m_viewModel->getKeyValue(uploadKey);

    for(int row = 0; row < keyValModel->rowCount(); ++row)
    {
        QStandardItem* paramItem = keyValModel->item(row, 0);
        if(paramItem->text() == downloadKey || paramItem->text() == uploadKey)
        {
            QStandardItem* valueItem = keyValModel->item(row, 1);
            valueItem->setText(paramItem->text() == downloadKey ? rxSpeed.second : txSpeed.second);
            Logger::instance().log(Logger::Debug,
                                   QString("Updating speeds: RX=%1, TX=%2")
                                       .arg(rxSpeed.second).arg(txSpeed.second),
                                   "NetworkView");
        }
    }
    setUpdatesEnabled(true);
}

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
    // layout()->addWidget(&crownLbl);

    //layout()->setSpacing(5);
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
}

bool NetworkInfoViewWidget::isUpdating() const
{
    return m_updating;
}

void NetworkInfoViewWidget::setUpdating(bool newUpdating)
{
    if (m_updating == newUpdating)
        return;
    m_updating = newUpdating;
    emit updatingChanged();
}
