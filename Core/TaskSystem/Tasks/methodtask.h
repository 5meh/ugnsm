#ifndef METHODTASK_H
#define METHODTASK_H

#include "taskwrapperbase.h"

template<class Receiver, typename... Args>
class MethodTask : public TaskWrapperBase
{
public:
    using Method = void (Receiver::*)(Args...);

    template<typename... ForwardArgs>
    MethodTask(Receiver* receiver,
               Method method,
               QThread::Priority priority,
               ForwardArgs&&... args)
        : TaskWrapperBase(priority),
        m_receiver(receiver),
        m_method(method),
        m_args(std::forward<ForwardArgs>(args)...)
    {
    }

    void executeTask() override
    {
        if (m_receiver) {
            std::apply(
                [this](auto&&... args) {
                    (m_receiver->*m_method)(std::forward<decltype(args)>(args)...);
                },
                m_args
                );
        }
    }

private:
    Receiver* m_receiver;
    Method m_method;
    std::tuple<Args...> m_args;
};

#endif // METHODTASK_H
