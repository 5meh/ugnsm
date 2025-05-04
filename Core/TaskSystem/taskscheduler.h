#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <QObject>
#include <QThreadPool>
#include <QMap>
#include <QMutex>

class TaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit TaskScheduler(QObject* parent = nullptr);
    ~TaskScheduler();

    enum class Priority
    {
        Normal,
        High,
        Background
    };

    template<typename T>
    void schedule(const QString& resourceKey,
                  void (T::*method)(),
                  T* receiver,
                  Priority priority = Priority::Normal);

    void cancelAll(const QString& resourceKey = "");

private:
    class ResourceMutex;

    QThreadPool m_pool;
    QMap<QString, ResourceMutex*> m_resources;
    QMutex m_resourceMapMutex;
};

class TaskScheduler::ResourceMutex
{
public:
    QMutex mutex;
    QAtomicInt refCount{0};
};

#endif // TASKSCHEDULER_H
