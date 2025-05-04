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

    MethodTask(Receiver* receiver, Method method, Args... args);
    MethodTask(Receiver* receiver, Method method, TaskWrapperBase::Priority priority, Args... args);

    void executeTask() override;

private:
    template<std::size_t... Is>
    void executeImpl(std::index_sequence<Is...>);

    Receiver* m_receiver;
    Method m_method;
    std::tuple<Args...> m_args;
    QMutex m_mutex;
};

#endif // METHODTASK_H
