#ifndef METHODTASK_H
#define METHODTASK_H

#include "taskwrapperbase.h"
#include <QObject>
#include <QMutex>

template<class Receiver, typename... Args>
class MethodTask : public TaskWrapperBase
{
public:
    typedef void (Receiver::*Method)(Args...);

    MethodTask(Receiver* receiver, Method method, Args... args)
        : TaskWrapperBase(QThread::NormalPriority),
        m_receiver(receiver),
        m_method(method),
        m_args(std::make_tuple(args...))
    {

    }
    MethodTask(Receiver* receiver, Method method, QThread::Priority priority, Args... args)
        : TaskWrapperBase(priority),
        m_receiver(receiver),
        m_method(method),
        m_args(std::make_tuple(args...))
    {

    }

//    void setMutex(QMutex* mutex) { m_mutex = mutex; }

    void executeTask() override
    {
        QMutexLocker lock(m_mutex);
        if(m_receiver)
        {
            executeImpl(std::index_sequence_for<Args...>{});
        }
    }

private:
    template<std::size_t... Is>
    void executeImpl(std::index_sequence<Is...>)
    {
        (m_receiver->*m_method)(std::get<Is>(m_args)...);
    }

    Receiver* m_receiver;
    Method m_method;
    std::tuple<Args...> m_args;
//    QMutex* m_mutex = nullptr;
};

#endif // METHODTASK_H
