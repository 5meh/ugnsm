#include "gridmanager.h"
#include "../../../UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "griddatamanager.h"
#include "../Utilities/Logger/logger.h"
#include "../TaskSystem/taskscheduler.h"

GridManager::GridManager(QObject* parent)
    :m_scheduler(new TaskScheduler(this)),
    m_dataManager(new GridDataManager(m_scheduler, this)),
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

void GridManager::initializeView()
{
    m_viewManager->setGridSize(getRows(), getCols());
}

void GridManager::initializeData()
{
    m_dataManager->initializeGrid(GRID_ROWS_DEFAULT, GRID_COLUMNS_DEFAUT);
    //QMetaObject::invokeMethod(m_dataManager, "refreshData", Qt::QueuedConnection);
}

void GridManager::setupGridManager()
{
    initializeView();
    initializeData();
}

void GridManager::setupConnections()
{
    connect(m_dataManager, &GridDataManager::cellChanged,
            this, [=](QPoint indx)
            {
                m_scheduler->scheduleMainThread(
                    "ui_update",
                    [=]
                    {
                        m_viewManager->updateCell(indx.x(), indx.y(), m_dataManager->cellData(indx));
                    },
                    QThread::HighPriority
                );
            });

    connect(m_dataManager, &GridDataManager::gridDimensionsChanged,
            m_viewManager.get(), [this]()
            {
                m_viewManager->setGridSize(m_dataManager->getRows(),
                                           m_dataManager->getCols());
            });

    connect(m_viewManager.get(), &GridViewManager::cellSwapRequestToDataManager,
            m_dataManager, &GridDataManager::swapCells);
}

GridViewManager* GridManager::getView() const
{
    return m_viewManager.data();
}
