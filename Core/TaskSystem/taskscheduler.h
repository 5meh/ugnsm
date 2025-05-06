#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include "Tasks/methodtask.h"
#include "Tasks/atomicmethodtask.h"
#include <QMap>
#include <QThreadPool>
#include <QTimer>

class TaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit TaskScheduler(QObject* parent = nullptr)
        : QObject(parent), m_pool(new QThreadPool(this))
    {
        m_pool->setMaxThreadCount(4);
    }

    ~TaskScheduler()
    {
        qDeleteAll(m_mutexes);
        qDeleteAll(m_repeatingTimers);
    }

    template<class Receiver, typename... Args>
    void schedule(const QString& resourceKey,
                  Receiver* receiver,
                  void (Receiver::*method)(Args...),
                  Args... args,
                  QThread::Priority priority = QThread::NormalPriority)
    {
        QMutex* mutex = getResourceMutex(resourceKey);
        MethodTask<Receiver, Args...>* task = new MethodTask<Receiver, Args...>(
            receiver, method, priority, args...
            );
        task->setMutex(mutex);
        startTask(task, priority);
    }

    template<class Receiver, typename... Args>
    void scheduleAtomic(QAtomicInt& flag,
                        const QString& resourceKey,
                        Receiver* receiver,
                        void (Receiver::*method)(Args...),
                        Args... args,
                        QThread::Priority priority = QThread::NormalPriority)
    {
        QMutex* mutex = getResourceMutex(resourceKey);
        AtomicMethodTask<Receiver, Args...>* task = new AtomicMethodTask<Receiver, Args...>(
            flag, receiver, method, args...
            );
        task->setMutex(mutex);
        startTask(task, priority);
    }

    template<class Receiver, typename... Args>
    void scheduleRepeating(const QString& resourceKey,
                           int intervalMs,
                           Receiver* receiver,
                           void (Receiver::*method)(Args...),
                           Args... args,
                           QThread::Priority priority = QThread::NormalPriority)
    {
        QTimer* timer = new QTimer(this);
        timer->setInterval(intervalMs);

        connect(timer, &QTimer::timeout, this, [=]
                {
                    this->schedule(resourceKey, receiver, method, args..., priority);
                });

        timer->start();
    }

private:
    void startTask(TaskWrapperBase* task, QThread::Priority priority)
    {
        m_pool->start(task, priority);
    }

    QMutex* getResourceMutex(const QString& key)
    {
        QMutexLocker lock(&m_mapMutex);
        if(!m_mutexes.contains(key))
        {
            m_mutexes[key] = new QMutex();
        }
        return m_mutexes[key];
    }

    QThreadPool* m_pool;
    QMap<QString, QMutex*> m_mutexes;
    QMutex m_mapMutex;
    QList<QTimer*> m_repeatingTimers;
};

#endif // TASKSCHEDULER_H
