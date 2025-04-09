#include "networkdashboardmanager.h"

#include "networkinfo.h"
#include "networkinfomodel.h"
#include "networkinfoviewwidget.h"
#include "networkmonitor.h"

#include <QLayoutItem>
#include <QGridLayout>
#include <QProgressBar>
#include <QFrame>

NetworkDashboardManager::NetworkDashboardManager(QGridLayout* gridLayout, QObject* parent)
    : QObject(parent),
    m_gridLayout(gridLayout),
    m_networkMonitor(nullptr),
    m_loadingBar(new QProgressBar()),
    m_draggedMac(QString())
{
    m_loadingBar->setRange(0, 0);
    m_loadingBar->setTextVisible(false);
    m_loadingBar->setFixedSize(WIDGET_SIZE);
}

void NetworkDashboardManager::initialize(NetworkMonitor* networkMonitor)
{
    m_networkMonitor = networkMonitor;
    createInitialGrid();
    connect(m_networkMonitor, &NetworkMonitor::statsUpdated,
            this, &NetworkDashboardManager::handleStatsUpdated);
}

void NetworkDashboardManager::createInitialGrid()
{
    // Fill grid with placeholders and loading bar
    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            if(row == 0 && col == 0)
            {
                m_gridLayout->addWidget(m_loadingBar, row, col);
            }
            else
            {
                m_gridLayout->addWidget(createPlaceholder(), row, col);
            }
        }
    }
}

void NetworkDashboardManager::addNetworkInfo(NetworkInfo* info)
{
    if(m_items.contains(info->getMac()))
        return;

    DashboardItem newItem;
    newItem.viewModel = new NetworkInfoModel(info, this);
    newItem.widget = new NetworkInfoViewWidget(newItem.viewModel);
    newItem.gridPosition = findAvailableGridPosition();

    m_items.insert(info->getMac(), newItem);
    m_positionMap.insert(newItem.gridPosition, info->getMac());

    setupWidgetConnections(newItem.widget);
    updateGridLayout();
}

void NetworkDashboardManager::handleWidgetDropped(const QString& sourceMac, const QString& targetMac)
{
    if(!m_items.contains(sourceMac) || !m_items.contains(targetMac))
        return;

    auto& sourceItem = m_items[sourceMac];
    auto& targetItem = m_items[targetMac];

    // Swap grid positions
    std::swap(sourceItem.gridPosition, targetItem.gridPosition);

    m_positionMap[sourceItem.gridPosition] = sourceMac;
    m_positionMap[targetItem.gridPosition] = targetMac;

    updateGridLayout();
    //emit widgetPositionsUpdated(getCurrentPositions());
}

void NetworkDashboardManager::handleStatsUpdate(const QString &interface, quint64 rx, quint64 tx)
{
    if(!m_items.contains(interface))
    {
        auto* info = new NetworkInfo(this);

        //TODO:provide proper initialization
        info->setName(interface);
        info->setMac(interface); // Simplified for example

        auto* viewModel = new NetworkInfoModel(info, this);
        auto* widget = new NetworkInfoViewWidget(viewModel);

        connect(widget, &NetworkInfoViewWidget::dragInitiated,
                this, [this](const QString& mac)
                {
                    m_draggedMac = mac;
                });

        connect(widget, &NetworkInfoViewWidget::dropReceived,
                this, &NetworkDashboardManager::handleDragDrop);

        m_items[interface] = {widget, QPoint(0,0)};
    }

    m_items[interface].widget->viewModel()->updateSpeeds(rx, tx);
    updateGridLayout();
}

void NetworkDashboardManager::handleDragDrop(const QString &draggedMac, const QPoint &newPos)
{
    if (!m_items.contains(draggedMac))
        return;
    if (newPos.x() < 0 || newPos.x() >= GRID_SIZE ||
        newPos.y() < 0 || newPos.y() >= GRID_SIZE)
        return;

    DashboardItem& draggedItem = m_items[draggedMac];
    const QPoint oldPos = draggedItem.gridPosition;

    QWidget* targetWidget = m_gridSlots.value(newPos, nullptr);

    if (targetWidget && targetWidget->property("isPlaceholder").toBool())
    {
        delete m_gridSlots[newPos];

        m_gridSlots[newPos] = draggedItem.widget;
        draggedItem.gridPosition = newPos;

        m_gridSlots[oldPos] = createPlaceholder();
    }
    else if (targetWidget)
    {
        QString targetMac;
        for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it)
        {
            if (it.value().widget == targetWidget)
            {
                targetMac = it.key();
                break;
            }
        }
        if (targetMac.isEmpty())
            return;

        DashboardItem& targetItem = m_items[targetMac];
        const QPoint targetOldPos = targetItem.position;

        m_gridSlots[oldPos] = targetItem.widget;
        m_gridSlots[targetOldPos] = draggedItem.widget;

        draggedItem.position = targetOldPos;
        targetItem.position = oldPos;
    }

    updateGridLayout();
}

void NetworkDashboardManager::handleStatsUpdated(const QString &interface, quint64 rx, quint64 tx)
{
    if(!m_items.contains(interface))
        return;

    DashboardItem& item = m_items[interface];
    item.totalSpeed = rx + tx;
    item.viewModel->updateSpeeds(rx, tx);

    if(!m_initialized)
    {
        m_loadedCount++;

        if(m_loadedCount >= m_items.size())
        {
            m_initialized = true;
            m_loadingBar->deleteLater();
            updateNetworkRankings();
            emit initializationComplete();
        }
    }
}

QPoint NetworkDashboardManager::findAvailableGridPosition() const
{
    const int GRID_SIZE = 3;
    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            QPoint pos(row, col);
            if(!m_positionMap.contains(pos))
                return pos;
        }
    }
    return QPoint(-1, -1); // Grid full
}

void NetworkDashboardManager::setupWidgetConnections(NetworkInfoViewWidget* widget)
{
    connect(widget, &NetworkInfoViewWidget::dragInitiated, this, [this](QWidget* w)
            {
                NetworkInfoViewWidget* viewWidget = qobject_cast<NetworkInfoViewWidget*>(w);
                viewWidget->setProperty("originalPosition", m_items[viewWidget->getMac()].gridPosition);
            });

    connect(widget, &NetworkInfoViewWidget::dropReceived, this, [this](QWidget* target)
            {
                auto sourceWidget = qobject_cast<NetworkInfoViewWidget*>(sender());
                auto targetWidget = qobject_cast<NetworkInfoViewWidget*>(target);

                if(sourceWidget && targetWidget)
                {
                    handleWidgetDropped(sourceWidget->getMac(), targetWidget->getMac());
                }
            });
}

void NetworkDashboardManager::updateGridLayout()
{
    //TODO:refactor to remove blinking
    // Clear existing layout
    while(QLayoutItem* item = m_gridLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    for(const auto& [mac, item] : m_items.asKeyValueRange())
    {
        m_gridLayout->addWidget(item.widget,
                                item.gridPosition.x(),
                                item.gridPosition.y());
    }

    emit gridLayoutChanged();
}

QHash<QString, QPair<int, int>> NetworkDashboardManager::getCurrentPositions() const
{
    QHash<QString, QPair<int, int>> positions;
    for(auto it = m_items.constBegin(); it != m_items.constEnd(); ++it)
    {
        positions.insert(it.key(), it.value().gridPosition);
    }
    return positions;
}
