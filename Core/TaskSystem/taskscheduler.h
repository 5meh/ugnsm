#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include "Tasks/methodtask.h"
#include "Tasks/atomicmethodtask.h"
#include "Tasks/qtinvoketask.h"
#include <QThreadPool>
#include <QTimer>
#include <QWaitCondition>
#include <QHash>
#include <QFuture>
#include <QCoreApplication>
#include <QVariant>

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
        m_repeatingTimers.emplace(resourceKey, timer);
    }

    void cancelRepeating(const QString& resourceKey)
    {
        QMutexLocker locker(&m_mapMutex);
        if (m_repeatingTimers.contains(resourceKey))
        {
            QTimer* timer = m_repeatingTimers.take(resourceKey);
            timer->stop();
            timer->deleteLater();
        }
    }

    template <typename Func>
    auto executeMainThread(const QString& resourceKey,
                           Func&& func,
                           QThread::Priority priority = QThread::NormalPriority,//TODO:remove later for compability
                           Qt::ConnectionType connectionType = Qt::QueuedConnection) -> decltype(func())
    {
        if (QThread::currentThread() == qApp->thread())
            return func();

        using ReturnType = decltype(func());
        if constexpr (std::is_void_v<ReturnType>)
        {
            QMetaObject::invokeMethod(qApp, [func = std::forward<Func>(func)] {
                func();
            }, connectionType);
        }
        else
        {
            ReturnType result;
            QMetaObject::invokeMethod(qApp, [&result, func = std::forward<Func>(func)] {
                result = func();
            }, connectionType);
            return result;
        }
    }



    template <typename Func>
    QFuture<typename std::invoke_result_t<Func>> scheduleAsync(
        const QString& taskId,
        Func&& func,
        QThread::Priority priority = QThread::NormalPriority)
    {
        using ResultType = typename std::invoke_result_t<Func>;
        QPromise<ResultType> promise;
        QFuture<ResultType> future = promise.future();

        connect(this, &TaskScheduler::taskCompleted,
                [promise = std::move(promise), taskId](const QString& completedId, const QVariant& result) mutable {
                    if (completedId == taskId)
                    {
                        promise.start();
                        promise.addResult(result.value<ResultType>());
                        promise.finish();
                    }
                });

        schedule(taskId, [this, taskId, func = std::forward<Func>(func)] {
            if constexpr (std::is_void_v<ResultType>)
            {
                func();
                emit taskCompleted(taskId, QVariant());//just empty ret value
            }
            else
                emit taskCompleted(taskId, QVariant::fromValue(func()));
        }, priority);

        return future;
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

        // Use QMetaObject::invokeMethod to execute the task in the main thread
        QMetaObject::invokeMethod(this, [this, task]() {
            task->executeTask();
            delete task;
        }, connectionType);
    }

signals:
    void taskCompleted(const QString& taskId, const QVariant& result);

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
    QHash<QString, QTimer*> m_repeatingTimers;
};

#endif // TASKSCHEDULER_H
