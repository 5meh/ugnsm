#include "dragmanager.h"

DragManager* DragManager::instance()
{
    static DragManager instance;
    return &instance;
}

DragManager::DragManager(QObject* parent)
    : QObject(parent)
{
}

void DragManager::tryStartDrag()
{
    // Atomic operation to ensure only one drag at a time
    // return !m_dragActive.testAndSetRelaxed(false, true);
    if (!m_dragActive.testAndSetRelaxed(false, true))
        return;
    emit dragStarted();
}

void DragManager::endDrag()
{
    //m_dragActive.storeRelaxed(false);
    if (!m_dragActive.testAndSetRelaxed(true, false))
        return;
    emit dragEnded();
}

bool DragManager::isDragging() const
{
    return m_dragActive.loadRelaxed();
}
