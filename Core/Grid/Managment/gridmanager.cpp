#include "gridmanager.h"
#include "../../../UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "griddatamanager.h"
#include "../Utilities/Logger/logger.h"
#include "../TaskSystem/taskscheduler.h"
#include "../globalmanager.h"
#include "dragmanager.h"

GridManager::GridManager(QObject* parent)
    : m_dataManager(new GridDataManager (this)),
    m_viewManager(new GridViewManager()),
    QObject(parent)
{
    setupConnections();
    setupGridManager();
    Logger::instance().log(Logger::Info, "GridManager initialized", "Grid");
}

GridManager::~GridManager()
{

}

QWidget* GridManager::gridWidget() const
{
    return m_viewManager.get();
}

void GridManager::updateGridDisplay()
{
    if (m_viewManager)
        m_viewManager->update();
}

int GridManager::getRows() const
{
    return m_dataManager->getRows();
}

int GridManager::getCols() const
{
    return m_dataManager->getCols();
}

void GridManager::setGridDimensions(int rows, int cols)
{
    m_dataManager->initializeGrid(rows, cols);
}

void GridManager::refresh()
{
    m_dataManager->refreshData();
}

void GridManager::initializeView()
{
    m_viewManager->setGridSize(getRows(), getCols());
}

void GridManager::initializeData()
{
    m_dataManager->initializeGrid(GRID_ROWS_DEFAULT, GRID_COLUMNS_DEFAUT);
}

void GridManager::setupGridManager()
{
    initializeView();
    initializeData();
}

void GridManager::setupConnections()
{
    connect(m_dataManager, &GridDataManager::cellChanged,
            m_viewManager.get(), &GridViewManager::updateCell,
            Qt::AutoConnection);

    connect(m_dataManager, &GridDataManager::gridDimensionsChanged,
            m_viewManager.get(), [this]()
            {
                m_viewManager->setGridSize(m_dataManager->getRows(),
                                           m_dataManager->getCols());
            });

    connect(m_viewManager.get(), &GridViewManager::cellSwapRequestToDataManager,
            m_dataManager, &GridDataManager::swapCells);

    connect(m_viewManager.get(), &GridViewManager::pauseGridUpdates,
            m_dataManager, &GridDataManager::setUpdatesPaused);

    connect(GlobalManager::dragManager(), &DragManager::dragStarted,
            m_dataManager, [this]() { m_dataManager->setUpdatesPaused(true);});
    connect(GlobalManager::dragManager(), &DragManager::dragEnded,
            m_dataManager, [this]() { m_dataManager->setUpdatesPaused(false); });

    SettingsManager* settings = GlobalManager::settingsManager();

    auto refreshStrategy = [this]() {
        GlobalManager::taskScheduler()->executeMainThread(
            "strategy_refresh",
            [this]() { m_dataManager->refreshData(); }
            );
    };

    auto refreshDisplay = [this]() {
        GlobalManager::taskScheduler()->scheduleMainThread(
            "display_refresh",
            [this]() {
                for (int r = 0; r < m_dataManager->getRows(); r++)
                {
                    for (int c = 0; c < m_dataManager->getCols(); c++)
                    {
                        if (auto model = m_dataManager->cellData({r, c}))
                            emit m_dataManager->cellChanged({r, c}, model);
                    }
                }
            }
            );
    };

    connect(settings, &SettingsManager::sortStrategyChanged, this, refreshStrategy);
    connect(settings, &SettingsManager::gridUpdateStrategyChanged, this, refreshStrategy);

    connect(settings, &SettingsManager::dataUnitsChanged, this, refreshDisplay);
    connect(settings, &SettingsManager::decimalPrecisionChanged, this, refreshDisplay);

    connect(settings, &SettingsManager::updateIntervalChanged, this, [this](int interval) {
        GlobalManager::taskScheduler()->cancelRepeating("data_refresh");
        GlobalManager::taskScheduler()->scheduleRepeating("data_refresh",
                                                          interval,
                                                          m_dataManager,
                                                          &GridDataManager::refreshData,
                                                          QThread::NormalPriority);
    });
    connect(settings, &SettingsManager::dataUnitsChanged,
            this, refreshDisplay);
    connect(settings, &SettingsManager::decimalPrecisionChanged,
            this, refreshDisplay);

    connect(GlobalManager::settingsManager(), &SettingsManager::autoRefreshChanged,
            this, &GridManager::handleAutoRefreshChanged);

    connect(GlobalManager::settingsManager(), &SettingsManager::updateIntervalChanged,
            this, &GridManager::handleUpdateIntervalChanged);
}

void GridManager::updateRefreshTask()
{
    GlobalManager::taskScheduler()->cancelRepeating("data_refresh");

    if (GlobalManager::settingsManager()->getAutoRefresh())
    {
        int interval = GlobalManager::settingsManager()->getUpdateInterval();
        GlobalManager::taskScheduler()->scheduleRepeating("data_refresh",
                                                          interval,
                                                          m_dataManager,
                                                          &GridDataManager::refreshData,
                                                          QThread::NormalPriority);

        Logger::instance().log(Logger::Info,
                               QString("Scheduled auto-refresh every %1 ms").arg(interval),
                               "GridManager");
    }
    else
    {
        Logger::instance().log(Logger::Info, "Auto-refresh disabled", "GridManager");
    }
}

void GridManager::handleAutoRefreshChanged(bool enabled)
{
    Logger::instance().log(Logger::Info,
                           QString("Auto-refresh changed: %1").arg(enabled ? "Enabled" : "Disabled"),
                           "GridManager");

    updateRefreshTask();
}

void GridManager::handleUpdateIntervalChanged(int interval)
{
    Logger::instance().log(Logger::Info,
                           QString("Update interval changed: %1 ms").arg(interval),
                           "GridManager");

    updateRefreshTask();
}

// GridViewManager* GridManager::getView() const
// {
//     return m_viewManager.data();
// }
