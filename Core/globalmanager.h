#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "TaskSystem/taskscheduler.h"
#include "Settings/settingsmanager.h"
#include "componentregistry.h"

#include <QCoreApplication>

class GlobalManager
{
public:
    static TaskScheduler* taskScheduler();
    static SettingsManager* settingsManager();
    static ComponentRegistry* componentRegistry();

private:
    GlobalManager() = delete;
    ~GlobalManager() = delete;

    struct Holder
    {
        TaskScheduler scheduler;
        SettingsManager settings;
        ComponentRegistry components;
        QMutex mutex;
    };

    static Holder& instance()
    {
        static QBasicAtomicInt flag = Q_BASIC_ATOMIC_INITIALIZER(Uninitialized);
        static Holder* holder = nullptr;

        if (!QGlobalStatic::testAndSetAcquire(&flag, &holder, Uninitialized, Initialized))
        {
            if (holder == nullptr)
            {
                holder = new Holder();
                qAddPostRoutine([](){ delete holder; });
            }
        }
        return *holder;
    }
};

#endif // GLOBALMANAGER_H
