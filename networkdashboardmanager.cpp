#include "networkdashboardmanager.h"

#include "networkinfo.h"
#include "networkinfoviewmodel.h"
#include "networkinfoviewwidget.h"
#include "networkmonitor.h"

#include <QLayoutItem>
#include <QGridLayout>
#include <QFrame>

NetworkDashboardManager::NetworkDashboardManager(QGridLayout* gridLayout, QObject* parent)
    : QObject(parent), m_gridLayout(gridLayout), m_networkMonitor(nullptr)
{
}

void NetworkDashboardManager::initialize(NetworkMonitor* networkMonitor)
{
    m_networkMonitor = networkMonitor;
    connect(m_networkMonitor, &NetworkMonitor::statsUpdated,
            this, [this](const QString& interface, quint64 rx, quint64 tx)
            {
                if(m_items.contains(interface))
                {
                    m_items[interface].viewModel->updateSpeeds(rx, tx);
                }
            });
}

void NetworkDashboardManager::addNetworkInfo(NetworkInfo* info)
{
    if(m_items.contains(info->getMac()))
        return;

    DashboardItem newItem;
    newItem.viewModel = new NetworkInfoViewModel(info, this);
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
    emit widgetPositionsUpdated(getCurrentPositions());
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
