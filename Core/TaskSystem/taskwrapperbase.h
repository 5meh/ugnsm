#ifndef TASKWRAPPERBASE_H
#define TASKWRAPPERBASE_H

#include <QRunnable>
#include <QAtomicInt>
#include <QMetaType>

class TaskWrapperBase : public QRunnable
{
public:
    enum Priority
    {
        HighPriority,
        NormalPriority,
        LowPriority
    };

    explicit TaskWrapperBase(Priority priority = NormalPriority)
        : m_priority(priority) {}

    virtual void executeTask() = 0;
    void run() final { executeTask(); }

    Priority taskPriority() const { return m_priority; }

protected:
    Priority m_priority;
};

#endif // TASKWRAPPERBASE_H
