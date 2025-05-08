#ifndef LAMBDATASK_H
#define LAMBDATASK_H

#include "taskwrapperbase.h"

template<typename Functor>
class LambdaTask : public TaskWrapperBase
{
public:
    explicit LambdaTask(Functor&& func)
        : m_func(std::forward<Functor>(func))
    {
    }

    void executeTask() override
    {
        QMutexLocker lock(m_mutex);
        m_func();
    }

private:
    Functor m_func;
};

#endif // LAMBDATASK_H
