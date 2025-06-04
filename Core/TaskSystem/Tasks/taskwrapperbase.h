#ifndef TASKWRAPPERBASE_H
#define TASKWRAPPERBASE_H

#include <QRunnable>
#include <QMutex>
#include <QThread>
#include <tuple>

class TaskWrapperBase : public QRunnable
{
public:
    explicit TaskWrapperBase(QThread::Priority priority = QThread::NormalPriority)
        : m_priority(priority), m_mutex(nullptr)
    {
        setAutoDelete(true);
    }

    virtual ~TaskWrapperBase() = default;

    void setMutex(QMutex* mutex) { m_mutex = mutex; }
    QMutex* mutex() const { return m_mutex; }

    virtual void executeTask() = 0;

    void run() override
    {
        if(m_mutex)
        {
            QMutexLocker locker(m_mutex);
            executeTask();
        }
        else
            executeTask();
    }

    QThread::Priority priority() const { return m_priority; }

protected:
    QThread::Priority m_priority;
    QMutex* m_mutex;
};

#endif // TASKWRAPPERBASE_H
