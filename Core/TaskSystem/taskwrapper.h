#ifndef TASKWRAPPER_H
#define TASKWRAPPER_H

#include <QRunnable>
#include <QMutex>
#include <QObject>

class TaskWrapper : public QRunnable
{
public:
    template<typename T>
    TaskWrapper(void (T::*method)(), T* receiver, QMutex* mutex);

    void run() override;

private:
    QObject* m_receiver;
    QMutex* m_mutex;
    std::function<void()> m_callback;
};

#endif // TASKWRAPPER_H
