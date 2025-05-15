#include "globalmanager.h"

TaskScheduler* GlobalManager::taskScheduler()
{
    QMutexLocker locker(&instance().mutex);
    return &instance().scheduler;
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
