#include "methodtask.h"

template<class Receiver, typename Args>
MethodTask<Receiver, Args>::MethodTask(Receiver *receiver, Method method, Args args)
    : TaskWrapperBase(NormalPriority),
    m_receiver(receiver),
    m_method(method),
    m_args(std::make_tuple(args...))
{

}

template<class Receiver, typename Args>
MethodTask<Receiver, Args>::MethodTask(Receiver *receiver, Method method, Priority priority, Args args)
    : TaskWrapperBase(priority),
    m_receiver(receiver),
    m_method(method),
    m_args(std::make_tuple(args...))
{

}

template<class Receiver, typename Args>
void MethodTask<Receiver, Args>::executeTask()
{
    QMutexLocker lock(&m_mutex);
    if(m_receiver)
    {
        executeImpl(std::index_sequence_for<Args...>{});
    }
}

template<class Receiver, typename Args>
template<std::size_t Is>
void MethodTask<Receiver, Args>::executeImpl(std::index_sequence<MethodTask::Is>)
{
    (m_receiver->*m_method)(std::get<Is>(m_args)...);
}
