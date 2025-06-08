#include "globalmanager.h"

TaskScheduler* GlobalManager::taskScheduler()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().scheduler;
}

MessageBoxManager* GlobalManager::messageBoxManager()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().messageBoxManager;
}

SettingsManager* GlobalManager::settingsManager()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().settings;
}

ComponentRegistry* GlobalManager::componentRegistry()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().components;
}

DragManager *GlobalManager::dragManager()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().dragManager;
}
