#ifndef LAMBDATASK_H
#define LAMBDATASK_H

#include "taskwrapperbase.h"

template<typename Functor>
class LambdaTask : public TaskWrapperBase
{
public:
    LambdaTask(Functor&& func, QThread::Priority priority = QThread::NormalPriority)
        : TaskWrapperBase(priority),
        m_func(std::forward<Functor>(func))
    {
    }

    void executeTask() override
    {
        m_func();
    }

private:
    Functor m_func;
};

#endif // LAMBDATASK_H
