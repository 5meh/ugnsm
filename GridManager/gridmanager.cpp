#include "gridmanager.h"

#include "gridviewmanager.h"
#include "griddatamanager.h"

GridManager::GridManager(QObject *parent)
    : QObject{parent}
{
    m_dataManager = new GridDataManager(this);
    m_viewManager = new GridViewManager(this);
}

GridManager::~GridManager()
{

}

GridViewManager* GridManager::view() const
{
    return m_viewManager;
}
