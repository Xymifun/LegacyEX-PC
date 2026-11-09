#include "nn/nex/GlobalVariables.hpp"

#include <nn/middleware.h>

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ThreadVariableList.hpp"

namespace nn
{
namespace nex
{

GlobalVariables* g_globalVariables = 0;
ThreadVariableList* g_threadVariableList = 0;
bool g_threadVariableListReady = false;

ForcedCriticalSection g_globalVariablesCriticalSection(1);

GlobalVariables::GlobalVariables()
    : m_broadcastAddress(),
      m_errorCode(0),
      m_errorDetail(0),
      m_currentThread(static_cast<ObjectThreadRoot*>(0)),
      m_binaryDataMap(),
      m_nexTokenMap(),
      m_nexPasswordMap(),
      m_nexHostMap(),
      m_nexPortMap(),
      m_eventLog(),
      m_traceLog(),
      m_cacheManager(),
      m_packetBufferParams()
{
    g_criticalSectionDisabled = false;
    m_packetBufferPoolEnabled = true;
    m_broadcastAddress = L"255.255.255.255";
}

GlobalVariables::~GlobalVariables()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);
    g_globalVariables = 0;
}

void GlobalVariables::AcquireInstance()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);
    SetModuleId("[SDK+Nintendo:NEX_3_10_0]");

    if (g_globalVariables == 0)
    {
        g_threadVariableList = new ThreadVariableList;
        g_globalVariables = new GlobalVariables;
    }
    else
    {
        g_globalVariables->AddRef();
    }
}

void GlobalVariables::ReleaseInstance()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);

    if (g_globalVariables == 0)
    {
        return;
    }

    bool lastReference = false;
    if (g_globalVariables->GetRefCount() == 1)
    {
        lastReference = true;
    }
    g_globalVariables->ReleaseRef();

    if (lastReference)
    {
        g_globalVariables = 0;
        delete g_threadVariableList;
        g_threadVariableList = 0;
    }
}

int32_t GlobalVariables::GetCount()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);

    if (g_globalVariables == 0)
    {
        return 0;
    }

    int32_t count = g_globalVariables->GetRefCount();
    if (Core::GetInstance() != 0 && count > 0)
    {
        --count;
    }

    return count;
}

void GlobalVariables::ClearCache()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);
}

}
}
