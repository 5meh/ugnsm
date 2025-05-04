#include "taskwrapper.h"

template<typename T>
TaskWrapper::TaskWrapper(void (T::*method)(), T *receiver, QMutex *mutex)
    : m_receiver(receiver), m_mutex(mutex)
{
    m_callback = [this, method]()
    {
        (static_cast<T*>(m_receiver)->*method)();
    };
}

void TaskWrapper::run()
{
    QMutexLocker lock(m_mutex);
    m_callback();
}
