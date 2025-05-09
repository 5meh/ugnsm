#ifndef ATOMICMETHODTASK_H
#define ATOMICMETHODTASK_H

#include "methodtask.h"

template<class Receiver, typename... Args>
class AtomicMethodTask : public MethodTask<Receiver, Args...>
{
public:
    template<typename... ForwardArgs>
    AtomicMethodTask(QAtomicInt& flag,
                     Receiver* receiver,
                     typename MethodTask<Receiver, Args...>::Method method,
                     ForwardArgs&&... args)
        : MethodTask<Receiver, Args...>(receiver, method, QThread::NormalPriority,
                                        std::forward<ForwardArgs>(args)...),
        m_flag(flag)
    {
    }

    void executeTask() override
    {
        m_flag.ref();
        MethodTask<Receiver, Args...>::executeTask();
        m_flag.deref();
    }

private:
    QAtomicInt& m_flag;
};

#endif // ATOMICMETHODTASK_H
