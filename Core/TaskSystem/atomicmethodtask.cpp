#include "atomicmethodtask.h"

template<class Receiver, typename Args>
AtomicMethodTask<Receiver, Args>::AtomicMethodTask(QAtomicInt& flag, Receiver* receiver, Method method, Args args)
    : MethodTask<Receiver, Args...>(receiver, method, args...),
    m_flag(flag)
{

}

template<class Receiver, typename Args>
void AtomicMethodTask<Receiver, Args>::executeTask()
{
    m_flag.ref();
    MethodTask<Receiver, Args...>::executeTask();
    m_flag.deref();
}
