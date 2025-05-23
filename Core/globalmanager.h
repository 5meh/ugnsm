#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "TaskSystem/taskscheduler.h"
#include "Settings/settingsmanager.h"
#include "componentregistry.h"
#include "../Utilities/MessageBoxManager/messageboxmanager.h"

#include <QCoreApplication>
#include <QtCore/qglobalstatic.h>

class GlobalManager//mb somehow make though globalstatic
{
public:
    static TaskScheduler* taskScheduler();
    static MessageBoxManager* messageBoxManager();
    static SettingsManager* settingsManager();
    static ComponentRegistry* componentRegistry();

private:
    GlobalManager() = delete;
    ~GlobalManager() = delete;

    struct Holder//Why the fuck i need Holder? D: rastrelyat later
    {
        TaskScheduler scheduler;
        SettingsManager settings;
        ComponentRegistry components;
        MessageBoxManager messageBoxManager;
        QMutex mutex;
    };

    static Holder& instance()
    {
        static Holder holder;
        return holder;
    }

};

#endif // GLOBALMANAGER_H
