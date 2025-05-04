#include "taskscheduler.h"

#include "taskwrapper.h"

TaskScheduler::TaskScheduler(QObject* parent)
    : QObject(parent)
{
    m_pool.setMaxThreadCount(QThread::idealThreadCount());
}

TaskScheduler::~TaskScheduler()
{
    m_pool.waitForDone();
    qDeleteAll(m_resources);
}

void TaskScheduler::cancelAll(const QString &resourceKey)
{
    QMutexLocker lock(&m_mapMutex);
    if(resourceKey.isEmpty())
    {
        m_pool.clear();
    }
    else if(m_mutexes.contains(resourceKey))
    {
        QMutex* mutex = m_mutexes[resourceKey];
        QMutexLocker resourceLock(mutex);
    }
}

template<typename T>
void TaskScheduler::schedule(const QString& resourceKey,
                             void (T::*method)(),
                             T* receiver,
                             Priority priority)
{
    QMutexLocker lock(&m_resourceMapMutex);

    if(!m_resources.contains(resourceKey))
    {
        m_resources[resourceKey] = new ResourceMutex();
    }

    ResourceMutex* res = m_resources[resourceKey];
    res->refCount.ref();

    TaskWrapper* task = new TaskWrapper(method, receiver, &res->mutex);
    task->setAutoDelete(true);

    switch(priority)
    {
    case Priority::High:
        m_pool.start(task, QThread::HighPriority);
        break;
    case Priority::Background:
        m_pool.start(task, QThread::LowPriority);
        break;
    default:
        m_pool.start(task);
    }
}
