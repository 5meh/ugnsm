#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include "Tasks/methodtask.h"
#include "Tasks/atomicmethodtask.h"
#include "Tasks/lambdatask.h"
#include "Tasks/qtinvoketask.h"
#include <QThreadPool>
#include <QTimer>
#include <QWaitCondition>
#include <QHash>
#include <QCoreApplication>

class TaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit TaskScheduler(QObject* parent = nullptr)
        : QObject(parent),
        m_pool(new QThreadPool(this))
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

        connect(timer, &QTimer::timeout, this, [=]() mutable
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

        //QMutex* mutex = getResourceMutex(resourceKey);
        //task->setMutex(mutex);
        // Since this is for main thread execution, we don't need resource locking
        task->setAutoDelete(true);

        // Use QMetaObject::invokeMethod to execute the task in the main thread
        QMetaObject::invokeMethod(this, [this, task]() {
            task->executeTask();
            delete task;
        }, connectionType);
    }

    template<typename Func, typename ResultType = typename std::result_of<Func()>::type>
    ResultType executeBlocking(const QString& resourceKey,
                               Func&& func,
                               int timeoutMs = -1,
                               QThread::Priority priority = QThread::NormalPriority)
    {
        // QMutex localMutex;
        // QWaitCondition condition;
        // ResultType result;
        // bool finished = false;
        // bool success = false;
        // std::exception_ptr exception;
        // bool isMainThread = (QThread::currentThread() == QCoreApplication::instance()->thread());

        // QMutex* resourceMutex = getResourceMutex(resourceKey);

        // // Schedule the task
        // schedule(resourceKey, [&]() {

        //     QMutexLocker resourceLocker(resourceMutex);
        //     QMutexLocker localLocker(&localMutex);

        //     try
        //     {
        //         result = func();
        //         success = true;
        //     }
        //     catch(...)
        //     {
        //         exception = std::current_exception();
        //     }

        //     finished = true;
        //     condition.wakeAll();
        // }, priority);

        // // Cooperative waiting
        // QMutexLocker locker(&localMutex);
        // QElapsedTimer timer;
        // timer.start();

        // while (!finished)
        // {
        //     int remaining = (timeoutMs < 0) ? -1 : (timeoutMs - timer.elapsed());

        //     if (remaining <= 0 && timeoutMs >= 0)
        //         throw std::runtime_error("Operation timed out");

        //     // Process events only if we're on main thread
        //     if (isMainThread)
        //     {
        //         // Release local mutex temporarily to avoid deadlock
        //         {
        //             QMutexUnlocker unlocker(&localMutex);
        //             QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        //         }

        //         condition.wait(&localMutex, 100);
        //     }
        //     else
        //         condition.wait(&localMutex, remaining > 0 ? std::min(100, remaining) : 100);
        // }
        // if (exception)
        //     std::rethrow_exception(exception);
        // if (!success)
        //     throw std::runtime_error("Task failed");

        // return result;
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
        QMutexLocker mapLocker(&m_mapMutex);
        if(!m_mutexes.contains(key))
            m_mutexes[key] = new QMutex();

        return m_mutexes[key];
    }

    QThreadPool* m_pool;
    QHash<QString, QMutex*> m_mutexes;
    QMutex m_mapMutex;
    QList<QTimer*> m_repeatingTimers;
};

#endif // TASKSCHEDULER_H
