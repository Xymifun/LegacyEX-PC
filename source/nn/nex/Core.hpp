#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CORE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CORE_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class CallContextRegister;
class JobConnectEndPoint;
class LogToClientController;
class Scheduler;
class SecurityContextManager;
class SystemComponents;
class Timeout;

extern bool g_dispatcherThreadEnabled;
extern bool g_transportThreadEnabled;
extern bool g_threadSafetyEnabled;
extern bool g_fatalState;
extern unsigned int g_coreInstanceCount;

class Core : public RefCountedObject
{
public:
    enum
    {
        ClassID = 3
    };

    enum ThreadMode
    {
        THREAD_MODE_0 = 0,
        THREAD_MODE_1 = 1,
        THREAD_MODE_2 = 2,
        THREAD_MODE_3 = 3,
        THREAD_MODE_4 = 4,
        THREAD_MODE_5 = 5,
        THREAD_MODE_6 = 6
    };

    typedef void (*TerminateImmediatelyCallback)(void* pContext);

    Core();
    virtual ~Core();

    static Core* GetInstance()
    {
        unsigned int uiContextId = g_usingInstantiationContext;
        if (g_usingInstantiationContext)
            uiContextId = g_instantiationContextVariable->GetRef();

        InstanceControl* pInstanceControl = g_instanceTable.FindInstance(ClassID, uiContextId);
        Core* pCore = reinterpret_cast<Core*>(pInstanceControl);
        if (pCore != 0)
            pCore = static_cast<Core*>(pInstanceControl->GetDelegatorInstance());

        return pCore;
    }

    static void AcquireInstance();
    static void ReleaseInstance();

    static void RefreshThreadMode();
    static void SetFatalState(bool bFatal);
    static void SetConnectionEncoding(unsigned int uiEncoding);
    static void SetDefaultThreadAffinity(unsigned int uiAffinity);
    static void SetDefaultThreadPriority(unsigned int uiPriority);
    static void SetThreadStackSize(unsigned int uiStackSize);
    static unsigned int GetThreadStackSize();

    Scheduler* GetScheduler() const { return m_pScheduler; }

    CallContextRegister* GetCallContextRegister() const { return m_pCallContextRegister; }

    SystemComponents* GetSystemComponents() const { return m_pSystemComponents; }

    SecurityContextManager* GetSecurityContextManager() const { return m_pSecurityContextManager; }

    bool GetTerminateImmediately() const { return m_bTerminateImmediately; }

    Time GetTerminateImmediatelyTime() const { return m_terminateImmediatelyTime; }

    void SetTerminateImmediately(bool bTerminateImmediately);
    void SetTerminateImmediatelyCallback(TerminateImmediatelyCallback cbCallback, void* pContext);
    void ResetTerminateImmediatelyCallback();

    void CreateGlobals();
    void DeleteGlobals();

private:
    friend class JobConnectEndPoint;
    friend class LogToClientController;
    friend class Timeout;

    Scheduler* m_pScheduler;
    CallContextRegister* m_pCallContextRegister;
    SystemComponents* m_pSystemComponents;
    SecurityContextManager* m_pSecurityContextManager;
    PseudoSingleton m_instanceControl;
    bool m_bTerminateImmediately;
    Time m_terminateImmediatelyTime;
    TerminateImmediatelyCallback m_cbTerminateImmediately;
    void* m_pTerminateImmediatelyContext;
    bool m_reserved0;
    CriticalSection m_criticalSection;
};

extern Core::ThreadMode g_threadMode;

}
}

#endif
