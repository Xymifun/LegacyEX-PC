#include "nn/nex/OperationManager.hpp"

#include <algorithm>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

OperationManager::OperationManager()
{
}

OperationManager::~OperationManager()
{
}

Operation* OperationManager::GetCurrentOperation() const
{
    if (m_operations.empty())
        return 0;

    return m_operations.back();
}

void OperationManager::TraceOperationStack(unsigned long long)
{
    for (OperationList::const_iterator it = m_operations.begin(); it != m_operations.end(); ++it)
    {
    }
}

void OperationManager::InvokeCallbacks(int firstPriority, int lastPriority, Operation* operation)
{
    for (CallbackList::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        OperationCallback* callback = *it;
        if (callback->GetPriority() >= firstPriority)
        {
            if (callback->GetPriority() > lastPriority)
                return;

            callback->CallMethod(operation);
        }
    }
}

void OperationManager::OperationBegins(Operation* operation)
{
    m_operations.push_back(operation);
    InvokeCallbacks(-1024, -1, operation);
}

void OperationManager::PopOperation(Operation*)
{
    m_operations.pop_back();
}

void OperationManager::OperationEnds(Operation* operation)
{
    InvokeCallbacks(1, 0x400, operation);
    PopOperation(operation);
}

void OperationManager::RegisterCallback(OperationCallback* callback)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    CallbackList::iterator it = m_callbacks.begin();
    while (it != m_callbacks.end())
    {
        if (callback->GetPriority() < (*it)->GetPriority())
        {
            m_callbacks.insert(it, callback);
            return;
        }
        ++it;
    }

    m_callbacks.insert(it, callback);
}

bool OperationManager::UnregisterCallback(OperationCallback* callback)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    CallbackList::iterator it = std::find(m_callbacks.begin(), m_callbacks.end(), callback);
    if (it != m_callbacks.end())
    {
        m_callbacks.erase(it);
        return true;
    }

    return false;
}

}
}
