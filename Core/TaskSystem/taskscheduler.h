#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include "Tasks/methodtask.h"
#include "Tasks/atomicmethodtask.h"
#include "Tasks/lambdatask.h"
#include "Tasks/qtinvoketask.h"
#include <QThreadPool>
#include <QTimer>
#include <QHash>

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
        qDeleteAll(m_repeatingTimers);
        qDeleteAll(m_mutexes);
    }

    template<class Receiver, typename... MethodArgs, typename... Args>
    void schedule(const QString& resourceKey,
                  Receiver* receiver,
                  void (Receiver::*method)(MethodArgs...),
                  QThread::Priority priority = QThread::NormalPriority,
                  Args&&... args)
    {
        auto task = new MethodTask<Receiver, MethodArgs...>(
            receiver,
            method,
            priority,
            std::forward<Args>(args)...
            );
        scheduleTask(resourceKey, task, priority);
    }

    template<class Receiver, typename... MethodArgs, typename... Args>
    void scheduleAtomic(QAtomicInt& flag,
                        const QString& resourceKey,
                        Receiver* receiver,
                        void (Receiver::*method)(MethodArgs...),
                        QThread::Priority priority = QThread::NormalPriority,
                        Args&&... args)
    {
        auto task = new AtomicMethodTask<Receiver, MethodArgs...>(
            flag,
            receiver,
            method,
            std::forward<Args>(args)...
            );
        scheduleTask(resourceKey, task, priority);
    }

    template<class Receiver, typename... MethodArgs, typename... Args>
    void scheduleRepeating(const QString& resourceKey,
                           int intervalMs,
                           Receiver* receiver,
                           void (Receiver::*method)(MethodArgs...),
                           QThread::Priority priority = QThread::NormalPriority,
                           Args&&... args)
    {
        QTimer* timer = new QTimer(this);
        timer->setInterval(intervalMs);

        auto argsTuple = std::make_shared<std::tuple<Args...>>(std::forward<Args>(args)...);

        connect(timer, &QTimer::timeout, this, [=, argsTuple]() mutable
                {
                    std::apply([&](auto&&... args)
                               {
                                   this->schedule(resourceKey, receiver, method, priority, args...);
                               }, *argsTuple);
                });

        timer->start();
        m_repeatingTimers.append(timer);
    }

    template<typename Functor>
    void scheduleMainThread(const QString& resourceKey,
                            Functor&& func,
                            QThread::Priority priority = QThread::NormalPriority,
                            Qt::ConnectionType connectionType = Qt::QueuedConnection)
    {
        auto task = new QtInvokeTask<Functor>(
            std::forward<Functor>(func),
            priority,
            connectionType
            );

        // Since this is for main thread execution, we don't need resource locking
        task->setAutoDelete(true);

        // Use QMetaObject::invokeMethod to execute the task in the main thread
        QMetaObject::invokeMethod(this, [this, task]() {
            task->executeTask();
            delete task;
        }, connectionType);
    }

private:
    void scheduleTask(const QString& resourceKey, TaskWrapperBase* task, QThread::Priority priority)
    {
        QMutex* mutex = getResourceMutex(resourceKey);
        task->setMutex(mutex);
        task->setAutoDelete(true);
        m_pool->start(task, priority);
    }

    QMutex* getResourceMutex(const QString& key)
    {
        if(!m_mutexes.contains(key))
        {
            m_mutexes[key] = new QMutex();
        }
        return m_mutexes[key];
    }

    QThreadPool* m_pool;
    QHash<QString, QMutex*> m_mutexes;
    QMutex m_mapMutex;
    QList<QTimer*> m_repeatingTimers;
};

#endif // TASKSCHEDULER_H
