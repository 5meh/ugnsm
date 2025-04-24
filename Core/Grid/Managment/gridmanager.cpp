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
    handleModelChanged();
}

void GridManager::initializeData()
{
    m_dataManager->initializeGrid(GRID_ROWS_DEFAULT, GRID_COLUMNS_DEFAUT);
    QMetaObject::invokeMethod(m_dataManager, "refreshData", Qt::QueuedConnection);
}

void GridManager::handleSwapRequest(int fr, int fc, int tr, int tc)
{
    m_dataManager->swapCells(fr, fc, tr, tc);
}

void GridManager::setupGridManager()
{
    initializeData();
    initializeView();
}

void GridManager::setupConnections()
{
    connect(m_dataManager, &GridDataManager::modelChanged,
            this, &GridManager::handleModelChanged);
    connect(m_dataManager, &GridDataManager::gridDimensionsChanged,
            this, &GridManager::gridDimensionsChanged);
    connect(m_viewManager.data(), &GridViewManager::cellSwapRequested,
            this, &GridManager::handleSwapRequest);
}

GridViewManager* GridManager::getView() const
{
    return m_viewManager.data();
}

void GridManager::handleModelChanged()
{

}
