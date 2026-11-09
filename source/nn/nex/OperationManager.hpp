#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OPERATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OPERATIONMANAGER_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Operation;

class OperationCallback : public RootObject
{
public:
    virtual ~OperationCallback() {}
    virtual void CallMethod(Operation* operation) = 0;

    int GetPriority() const { return m_priority; }

protected:
    int m_priority;
};

class OperationManager : public RootObject
{
public:
    OperationManager();
    virtual ~OperationManager();

    Operation* GetCurrentOperation() const;
    void TraceOperationStack(unsigned long long flags);
    void InvokeCallbacks(int firstPriority, int lastPriority, Operation* operation);
    void OperationBegins(Operation* operation);
    void PopOperation(Operation* operation);
    void OperationEnds(Operation* operation);
    void RegisterCallback(OperationCallback* callback);
    bool UnregisterCallback(OperationCallback* callback);

private:
    typedef qList<OperationCallback*> CallbackList;
    typedef qList<Operation*> OperationList;

    CallbackList m_callbacks;
    OperationList m_operations;
};

}
}

#endif
