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

#include "../../../../Utilities/Delegates/ledindicatordelegate.h"
#include "../../../../Utilities/LedIndicator/ledindicator.h"
//#include "networkinfo.h"
#include "../../../../Core/Network/Information/networkinfomodel.h"

NetworkInfoViewWidget::NetworkInfoViewWidget(NetworkInfoModel *viewModel, QFrame *parent)
    : GridCellWidget(parent), m_viewModel(viewModel)
{
    //setGeometry(m_widgetSize);
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

void NetworkInfoViewWidget::setViewModel(NetworkInfoModel* model)
{
    if (m_viewModel == model)
        return;

    if (m_viewModel)
        disconnect(m_viewModel, &NetworkInfoModel::propertiesChanged,
                   this, &NetworkInfoViewWidget::handlePropertiesChanged);

    m_viewModel = model;

    if (m_viewModel)
    {
        connect(m_viewModel, &NetworkInfoModel::propertiesChanged,
                this, &NetworkInfoViewWidget::handlePropertiesChanged);

    }

    // Full UI refresh
    updateNetworkInfoDisplay();
}

const NetworkInfoModel *NetworkInfoViewWidget::getModel() const
{
    return m_viewModel ? m_viewModel : nullptr;
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

    if (propertyName == "downloadSpeed" ||
        propertyName == "uploadSpeed" ||
        propertyName == "totalSpeed")
    {
        updateSpeedIndicators();
    }

    setProperty("updating", true);
    QTimer::singleShot(150, this, [this]()
                       {
                           setProperty("updating", false);
                           //style()->unpolish(this);
                           style()->polish(this);
                       });

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
    //resizeKeyValTable();
    setUpdatesEnabled(true);
    keyValueTbl->setUpdatesEnabled(true);
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

    // Set up the LED indicator delegate
    LedIndicatorDelegate* ledDelegate = new LedIndicatorDelegate(this);
    keyValueTbl->setItemDelegateForColumn(2, ledDelegate);

    // Configure table view properties
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->setSelectionMode(QAbstractItemView::NoSelection);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->horizontalHeader()->setVisible(false);
    keyValueTbl->horizontalHeader()->setStretchLastSection(true);
    keyValueTbl->setShowGrid(false);
    keyValueTbl->setFocusPolicy(Qt::NoFocus);

    // Set the table view to use transparent background
    keyValueTbl->setAttribute(Qt::WA_TranslucentBackground);
    keyValueTbl->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    // Set up the table view's style
    keyValueTbl->setStyleSheet("QTableView { background: transparent; border: none; }");

    setKeyValueTbl();
    connectViewModel();

    keyValueTbl->setColumnWidth(0, 120); // Parameter column
    keyValueTbl->setColumnWidth(1, 150); // Value column
    keyValueTbl->setColumnWidth(2, 30);  // LED indicator
    // Install event filter for the viewport
    keyValueTbl->viewport()->installEventFilter(this);
    layout()->addWidget(keyValueTbl);
}

void NetworkInfoViewWidget::connectViewModel()
{
    connect(m_viewModel, &NetworkInfoModel::nameChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoModel::macChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoModel::ipAddressChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoModel::netmaskChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    connect(m_viewModel, &NetworkInfoModel::speedChanged, this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
}

bool NetworkInfoViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    QTableView* table = qobject_cast<QTableView*>(watched->parent());
    if(table && (table == keyValueTbl))
    {
        // Only block unwanted interactions
        switch(event->type())
        {
        case QEvent::ContextMenu:
            return true;
        default:
            return false;
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
    QPair<QString, QString> rxSpeed = m_viewModel->getKeyValue("RX Speed");
    QPair<QString, QString> txSpeed = m_viewModel->getKeyValue("TX Speed");

    for(int row = 0; row < keyValModel->rowCount(); ++row)
    {
        QStandardItem* paramItem = keyValModel->item(row, 0);
        if(paramItem->text() == "RX Speed" || paramItem->text() == "TX Speed")
        {
            QStandardItem* valueItem = keyValModel->item(row, 1);
            valueItem->setText(paramItem->text() == "RX Speed" ? rxSpeed.second : txSpeed.second);
        }
    }
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

    layout()->setSpacing(0);
    keyValueTbl->viewport()->installEventFilter(this);
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
