#include "dragmanager.h"

DragManager::DragManager(QObject* parent)
    : QObject(parent)
{
}

bool DragManager::startDrag(const QPoint& gridIndex)
{
    QMutexLocker locker(&m_mutex);
    if (!m_activeDrags.isEmpty())
        return false;
    m_activeDrags.insert(gridIndex);
    return true;
}

void DragManager::endDrag(const QPoint& gridIndex)
{
    QMutexLocker locker(&m_mutex);
    m_activeDrags.remove(gridIndex);
}

bool DragManager::isDragging() const
{
    QMutexLocker locker(&m_mutex);
    return !m_activeDrags.isEmpty();
}
