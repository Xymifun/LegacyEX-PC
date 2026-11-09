#include "nn/nex/Core.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/InstanceControl.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/SystemComponents.hpp"

namespace nn
{
namespace nex
{

bool g_dispatcherThreadEnabled = true;
bool g_transportThreadEnabled = false;
bool g_threadSafetyEnabled = true;
bool g_fatalState = false;
unsigned int g_coreInstanceCount = 0;

Core::ThreadMode g_threadMode = Core::THREAD_MODE_0;

Core::Core()
    : RefCountedObject(),
      m_instanceControl(ClassID),
      m_bTerminateImmediately(false),
      m_terminateImmediatelyTime(),
      m_cbTerminateImmediately(0),
      m_pTerminateImmediatelyContext(0),
      m_reserved0(false)
{
    g_fatalState = false;

    Time::GetTime();
    RefreshThreadMode();

    if (g_dispatcherThreadEnabled && !g_threadSafetyEnabled)
        g_threadSafetyEnabled = true;

    g_criticalSectionDisabled = !g_threadSafetyEnabled;

    {
        ScopedCS lock(&g_instanceTableCriticalSection);
        ++g_coreInstanceCount;
    }

    m_instanceControl.SetDelegatorInstance(this);

    m_pScheduler = new Scheduler();
    if (g_dispatcherThreadEnabled)
        m_pScheduler->StartDispatcherThread();

    m_pCallContextRegister = new CallContextRegister();

    m_pSystemComponents = new SystemComponents();
    m_pSystemComponents->GetCoreGroup()->RegisterComponent(m_pCallContextRegister);

    m_pSecurityContextManager = new SecurityContextManager();

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_pSystemComponents->GetCoreGroup()->Initialize();
}

Core::~Core()
{
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        m_pSystemComponents->GetCoreGroup()->Terminate();
    }

    m_pSystemComponents->WaitForTerminatedState(0xFFFFFFFF);
    m_pScheduler->WaitForAllJobsToComplete(60000, true);
    m_pSystemComponents->GetCoreGroup()->UnregisterComponent(m_pCallContextRegister);

    delete m_pScheduler;
    delete m_pSystemComponents;
    delete m_pSecurityContextManager;

    {
        ScopedCS lock(&g_instanceTableCriticalSection);
        --g_coreInstanceCount;
    }

    GlobalVariables::ClearCache();
}

void Core::AcquireInstance()
{
    g_instanceTableCriticalSection.m_mutex.Lock();

    GlobalVariables::AcquireInstance();

    if (GetInstance() == 0)
        new Core;
    else
        GetInstance()->AddRef();

    g_instanceTableCriticalSection.m_mutex.Unlock();
}

void Core::ReleaseInstance()
{
    g_instanceTableCriticalSection.m_mutex.Lock();

    GetInstance()->ReleaseRef();

    if (g_coreInstanceCount == 0)
        g_criticalSectionDisabled = false;

    GlobalVariables::ReleaseInstance();

    g_instanceTableCriticalSection.m_mutex.Unlock();
}

void Core::RefreshThreadMode()
{
    switch (g_threadMode)
    {
    case THREAD_MODE_0:
        return;

    case THREAD_MODE_1:
        g_dispatcherThreadEnabled = false;
        g_transportThreadEnabled = true;
        g_threadSafetyEnabled = true;
        return;

    case THREAD_MODE_2:
        g_transportThreadEnabled = true;
        g_dispatcherThreadEnabled = false;
        g_threadSafetyEnabled = false;
        return;

    case THREAD_MODE_3:
        g_transportThreadEnabled = false;
        g_dispatcherThreadEnabled = false;
        g_threadSafetyEnabled = false;
        return;

    case THREAD_MODE_4:
        g_transportThreadEnabled = false;
        g_dispatcherThreadEnabled = false;
        g_threadSafetyEnabled = true;
        return;

    case THREAD_MODE_5:
        g_transportThreadEnabled = false;
        g_dispatcherThreadEnabled = true;
        g_threadSafetyEnabled = true;
        return;

    case THREAD_MODE_6:
        g_transportThreadEnabled = true;
        g_dispatcherThreadEnabled = true;
        g_threadSafetyEnabled = true;
        return;
    }
}

void Core::SetFatalState(bool bFatal)
{
    g_fatalState = bFatal;
}

void Core::SetConnectionEncoding(unsigned int)
{
}

void Core::SetTerminateImmediately(bool bTerminateImmediately)
{
    TerminateImmediatelyCallback cbCallback = 0;
    void* pContext = 0;

    {
        ScopedCS lock(&m_criticalSection);

        if (m_bTerminateImmediately == bTerminateImmediately)
            return;

        m_bTerminateImmediately = bTerminateImmediately;

        if (bTerminateImmediately)
        {
            m_terminateImmediatelyTime = Time(Time::GetTime());

            if (m_cbTerminateImmediately != 0)
            {
                cbCallback = m_cbTerminateImmediately;
                pContext = m_pTerminateImmediatelyContext;
                m_cbTerminateImmediately = 0;
            }
        }
    }

    if (cbCallback != 0)
        cbCallback(pContext);
}

void Core::SetTerminateImmediatelyCallback(TerminateImmediatelyCallback cbCallback, void* pContext)
{
    ScopedCS lock(&m_criticalSection);
    m_cbTerminateImmediately = cbCallback;
    m_pTerminateImmediatelyContext = pContext;
}

void Core::ResetTerminateImmediatelyCallback()
{
    ScopedCS lock(&m_criticalSection);
    m_cbTerminateImmediately = 0;
    m_pTerminateImmediatelyContext = 0;
}

}
}
