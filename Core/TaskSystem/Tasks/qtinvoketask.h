#ifndef QTINVOKETASK_H
#define QTINVOKETASK_H

#include "taskwrapperbase.h"
#include <QMetaObject>

template<typename Functor>
class QtInvokeTask : public TaskWrapperBase
{
public:
    QtInvokeTask(Functor&& func,
                 QThread::Priority priority = QThread::NormalPriority,
                 Qt::ConnectionType connectionType = Qt::QueuedConnection)
        : TaskWrapperBase(priority),
        m_func(std::forward<Functor>(func)),
        m_connectionType(connectionType)
    {
    }

    void executeTask() override
    {
        m_func();
    }

private:
    Functor m_func;
    Qt::ConnectionType m_connectionType;
};

#endif // QTINVOKETASK_H
