#include "gridmanager.h"
#include "../../../UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "griddatamanager.h"

GridManager::GridManager(QObject *parent)
    : QObject{parent},
    m_dataManager{new GridDataManager(this)},
    m_viewManager{new GridViewManager()},
    m_rows(GRID_ROWS_DEFAULT),
    m_cols(GRID_COLUMNS_DEFAUT)
{
    syncManagers();
}

GridManager::~GridManager()
{

}

void GridManager::setGridDimensions(int rows, int cols)
{
    if(rows == m_rows && cols == m_cols)
        return;
    m_rows = rows;
    m_cols = cols;
    syncManagers();
    emit gridDimensionsChanged();
}

void GridManager::syncManagers()
{
    m_dataManager->initializeData(m_rows, m_cols);
    m_viewManager->setGridSize(m_rows, m_cols);
}

GridViewManager* GridManager::view() const
{
    return m_viewManager.data();
}
