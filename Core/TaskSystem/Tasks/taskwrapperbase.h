#ifndef TASKWRAPPERBASE_H
#define TASKWRAPPERBASE_H

#include <QRunnable>
//#include <QAtomicInt>
//#include <QMetaType>
#include <QMutex>
#include <QThread>

class TaskWrapperBase : public QRunnable
{
public:
    explicit TaskWrapperBase(QThread::Priority priority = QThread::Priority::NormalPriority)
        : m_priority(priority),
        m_mutex(nullptr)
    {
        setAutoDelete(true);
    }

    virtual void executeTask() = 0;
    void run() final
    {
        executeTask();
    }
    void setMutex(QMutex* mutex) { m_mutex = mutex; }

    QThread::Priority taskPriority() const { return m_priority; }

protected:
    QThread::Priority m_priority = QThread::NormalPriority;
    QMutex* m_mutex = nullptr;
};

#endif // TASKWRAPPERBASE_H
