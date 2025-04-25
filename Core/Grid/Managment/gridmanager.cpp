#include "gridmanager.h"
#include "../../../UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "griddatamanager.h"

GridManager::GridManager(QObject* parent)
    :m_dataManager(new GridDataManager(this)),
    m_viewManager(new GridViewManager()),
    QObject(parent)
{
    setupGridManager();
    setupConnections();
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
    QMetaObject::invokeMethod(m_dataManager, "refreshData", Qt::QueuedConnection);
}

void GridManager::setupGridManager()
{
    initializeData();
    initializeView();
}

void GridManager::setupConnections()
{
    connect(m_dataManager, &GridDataManager::cellChanged,
            this, [this](int row, int col)
            {
                m_viewManager->setUpdatesEnabled(false);
                m_viewManager->updateCell(row, col, m_dataManager->cellData(row, col));
                m_viewManager->setUpdatesEnabled(true);
            });

    connect(m_dataManager, &GridDataManager::gridDimensionsChanged,
            m_viewManager.get(), [this]()
            {
                m_viewManager->setGridSize(m_dataManager->getRows(),
                                           m_dataManager->getCols());
            });

    connect(m_viewManager.get(), &GridViewManager::cellSwapRequested,
            m_dataManager, &GridDataManager::swapCells);
}

GridViewManager* GridManager::getView() const
{
    return m_viewManager.data();
}
