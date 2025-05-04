#ifndef ATOMICMETHODTASK_H
#define ATOMICMETHODTASK_H

#include "methodtask.h"

template<class Receiver, typename... Args>
class AtomicMethodTask : public MethodTask<Receiver, Args...>
{
public:
    AtomicMethodTask(QAtomicInt& flag, Receiver* receiver,
                     typename MethodTask<Receiver, Args...>::Method method,
                     Args... args)
        : MethodTask<Receiver, Args...>(receiver, method, args...),
        m_flag(flag) {}

    void executeTask() override {
        m_flag.ref();
        MethodTask<Receiver, Args...>::executeTask();
        m_flag.deref();
    }

private:
    QAtomicInt& m_flag;
};

#endif // ATOMICMETHODTASK_H
