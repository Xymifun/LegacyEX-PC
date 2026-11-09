#include "nn/nex/ThreadVariableList.hpp"

#include "nn/nex/ThreadVariableRoot.hpp"

namespace nn
{
namespace nex
{

extern ThreadVariableList* g_threadVariableList;
extern bool g_threadVariableListReady;

ThreadVariableList::ThreadVariableList() : m_criticalSection(1)
{
    g_threadVariableListReady = true;
}

ThreadVariableList::~ThreadVariableList()
{
    ResetValues();
    g_threadVariableListReady = false;
}

void ThreadVariableList::AddVariable(ThreadVariableRoot* variable)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);
    m_variables.push_back(variable);
}

ThreadVariableList* ThreadVariableList::GetInstanceRef()
{
    return g_threadVariableList;
}

void ThreadVariableList::ClearValue(uint32_t value)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (qChain<ThreadVariableRoot*, DefaultChainPolicy<ThreadVariableRoot*> >::iterator it = m_variables.begin();
         it != m_variables.end(); ++it)
        (*it)->ClearValue(value);
}

void ThreadVariableList::RemoveVariable(ThreadVariableRoot* variable)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);
    m_variables.remove(variable);
}

void ThreadVariableList::ResetValues()
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (qChain<ThreadVariableRoot*, DefaultChainPolicy<ThreadVariableRoot*> >::iterator it = m_variables.begin();
         it != m_variables.end(); ++it)
        (*it)->ResetValues();
}

void ThreadVariableList::ClearValue()
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (qChain<ThreadVariableRoot*, DefaultChainPolicy<ThreadVariableRoot*> >::iterator it = m_variables.begin();
         it != m_variables.end(); ++it)
        (*it)->ClearValue();
}

}
}
