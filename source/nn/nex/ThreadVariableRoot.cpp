#include "nn/nex/ThreadVariableRoot.hpp"

#include "nn/nex/ObjectThreadRoot.hpp"
#include "nn/nex/ThreadVariableList.hpp"

namespace nn
{
namespace nex
{

ThreadVariableRoot::ThreadVariableRoot()
{
    ThreadVariableList::GetInstanceRef()->AddVariable(this);
}

ThreadVariableRoot::~ThreadVariableRoot()
{
    ThreadVariableList::GetInstanceRef()->RemoveVariable(this);
}

void ThreadVariableRoot::ResetValues()
{
}

void ThreadVariableRoot::ClearValue()
{
}

void ThreadVariableRoot::ClearValue(uint32_t)
{
}

uint32_t ThreadVariableRoot::GetThreadID()
{
    return ObjectThreadRoot::GetCurrentThreadID();
}

}
}
