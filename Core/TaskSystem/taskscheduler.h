#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include "methodtask.h"
#include "atomicmethodtask.h"
#include <QMap>
#include <QThreadPool>

class TaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit TaskScheduler(QObject* parent = nullptr)
        : QObject(parent), m_pool(new QThreadPool(this))
    {
        m_pool->setMaxThreadCount(4);
    }

    template<class Receiver, typename... Args>
    void schedule(const QString& resourceKey,
                  Receiver* receiver,
                  void (Receiver::*method)(Args...),
                  Args... args,
                  TaskWrapperBase::Priority priority = TaskWrapperBase::NormalPriority)
    {
        MethodTask<Receiver, Args...>* task = new MethodTask<Receiver, Args...>(
            receiver, method, priority, args...
            );
        startTask(resourceKey, task);
    }

    template<class Receiver, typename... Args>
    void scheduleAtomic(QAtomicInt& flag,
                        const QString& resourceKey,
                        Receiver* receiver,
                        void (Receiver::*method)(Args...),
                        Args... args,
                        TaskWrapperBase::Priority priority = TaskWrapperBase::NormalPriority)
    {
        AtomicMethodTask<Receiver, Args...>* task = new AtomicMethodTask<Receiver, Args...>(
            flag, receiver, method, args...
            );
        startTask(resourceKey, task);
    }

private:
    void startTask(const QString& resourceKey, TaskWrapperBase* task) {
        QMutex* mutex = getResourceMutex(resourceKey);
        task->setProperty("resourceMutex", QVariant::fromValue(mutex));

        switch(task->taskPriority()) {
        case TaskWrapperBase::HighPriority:
            m_pool->start(task, QThread::HighPriority);
            break;
        case TaskWrapperBase::LowPriority:
            m_pool->start(task, QThread::LowPriority);
            break;
        default:
            m_pool->start(task, QThread::InheritPriority);
        }
    }

    QMutex* getResourceMutex(const QString& key) {
        QMutexLocker lock(&m_mapMutex);
        if(!m_mutexes.contains(key)) {
            m_mutexes[key] = new QMutex(QMutex::Recursive);
        }
        return m_mutexes[key];
    }

    QThreadPool* m_pool;
    QMap<QString, QMutex*> m_mutexes;
    QMutex m_mapMutex;
};

#endif // TASKSCHEDULER_H
