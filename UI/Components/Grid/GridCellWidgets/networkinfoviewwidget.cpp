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
    setFixedSize(m_widgetSize);
    setupUI();
    setStyleSheet("background-color: white; border-radius: 8px;");
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

    // Disconnect old model signals
    if (m_viewModel)
    {
        disconnect(m_viewModel, &NetworkInfoModel::propertyChanged,
                   this, &NetworkInfoViewWidget::updateProperty);
        disconnect(m_viewModel, &NetworkInfoModel::nameChanged,
                   this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        disconnect(m_viewModel, &NetworkInfoModel::macChanged,
                   this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        disconnect(m_viewModel, &NetworkInfoModel::ipAddressChanged,
                   this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        disconnect(m_viewModel, &NetworkInfoModel::netmaskChanged,
                   this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        disconnect(m_viewModel, &NetworkInfoModel::speedChanged,
                   this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    }

    m_viewModel = model;

    // Connect new model signals
    if (m_viewModel)
    {
        connect(m_viewModel, &NetworkInfoModel::propertyChanged,
                this, &NetworkInfoViewWidget::updateProperty);
        connect(m_viewModel, &NetworkInfoModel::nameChanged,
                this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        connect(m_viewModel, &NetworkInfoModel::macChanged,
                this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        connect(m_viewModel, &NetworkInfoModel::ipAddressChanged,
                this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        connect(m_viewModel, &NetworkInfoModel::netmaskChanged,
                this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
        connect(m_viewModel, &NetworkInfoModel::speedChanged,
                this, &NetworkInfoViewWidget::updateNetworkInfoDisplay);
    }

    // Full UI refresh
    updateNetworkInfoDisplay();
}

const NetworkInfoModel *NetworkInfoViewWidget::getModel() const
{
    return m_viewModel;
}

QString NetworkInfoViewWidget::cellId() const
{
    return objectName();
}

void NetworkInfoViewWidget::updateProperty(const QString &propertyName)
{
    if (!m_viewModel)
        return;

    setUpdatesEnabled(false);

    // Get mapped display property name
    const QString displayName = m_viewModel->propertyMap().key(propertyName);
    const QPair<QString, QString> data = m_viewModel->getKeyValue(displayName);

    // Find and update specific row
    for (int row = 0; row < keyValModel->rowCount(); ++row)
    {
        QStandardItem* paramItem = keyValModel->item(row, 0);
        if (paramItem->text() == data.first)
        {
            // Update value column
            QStandardItem* valueItem = keyValModel->item(row, 1);
            valueItem->setText(data.second);

            // Update status indicator if needed
            QStandardItem* statusItem = keyValModel->item(row, 2);
            updateStatusIndicator(statusItem, data.first, data.second);

            break;
        }
    }

    // Special handling for speed updates
    if (propertyName == "downloadSpeed" ||
        propertyName == "uploadSpeed" ||
        propertyName == "totalSpeed")
    {
        updateSpeedIndicators();
    }

    // Visual update feedback
    setProperty("updating", true);
    QTimer::singleShot(150, this, [this]()
                       {
                           setProperty("updating", false);
                           style()->unpolish(this);
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
    keyValueTbl->setItemDelegateForColumn(2, new LedIndicatorDelegate(this));
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyValueTbl->setSelectionMode(QAbstractItemView::NoSelection);
    keyValueTbl->verticalHeader()->hide();
    keyValueTbl->horizontalHeader()->setStretchLastSection(true);
    keyValueTbl->setShowGrid(false);
    setKeyValueTbl();
    connectViewModel();
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
