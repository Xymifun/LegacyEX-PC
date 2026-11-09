#include "nn/nex/CallContextRegister.hpp"

#include <cwchar>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/MethodCallJob.hpp"
#include "nn/nex/PeriodicJob.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/SystemSetting.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

SystemSetting g_checkExpiredCallsPeriod(L"CallContextRegisterCheckExpiredCalls", 100);

const int CALL_CONTEXT_RESULT_CONNECTION_LOST = static_cast<int>(0x80010004u);
const int CALL_CONTEXT_RESULT_TIMEOUT = static_cast<int>(0x8001000Bu);

CallContextRegister::CallContextRegister()
    : SystemComponent(String(L"CallContextRegister")),
      m_callContexts(),
      m_associatedIds(),
      m_uiNextCallId(1),
      m_pCheckExpiredCallsJob(0)
{
}

CallContextRegister::~CallContextRegister()
{
    if (!g_fatalState)
    {
        qResult result;

        CallContextMap::iterator callContext = m_callContexts.begin();
        while (callContext != m_callContexts.end())
            (callContext++)->second->SetStateImpl(CallContext::CallCanceled, result, true);
    }
}

const wchar_t* CallContextRegister::GetType() const
{
    return L"CallContextRegister";
}

bool CallContextRegister::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"CallContextRegister", pType) == 0 || SystemComponent::IsAKindOf(pType);
}

void CallContextRegister::EnforceDeclareSysComponentMacro()
{
}

bool CallContextRegister::BeginInitialization()
{
    Start();
    return true;
}

bool CallContextRegister::BeginTermination()
{
    Stop();
    return true;
}

unsigned int CallContextRegister::Register(CallContext* pCallContext)
{
    unsigned int uiCallId = m_uiNextCallId;
    m_uiNextCallId++;
    if (m_uiNextCallId == 0)
        m_uiNextCallId = 1;

    m_callContexts[uiCallId] = pCallContext;
    return uiCallId;
}

void CallContextRegister::Unregister(unsigned int uiCallId)
{
    CallContextMap::iterator callContext = m_callContexts.find(uiCallId);
    m_callContexts.erase(callContext);
    std::pair<AssociatedIdMap::iterator, AssociatedIdMap::iterator> associatedIds = m_associatedIds.equal_range(
        uiCallId);
    m_associatedIds.erase(associatedIds.first, associatedIds.second);
}

CallContext* CallContextRegister::GetCallContext(unsigned int uiCallId)
{
    CallContextMap::iterator callContext = m_callContexts.find(uiCallId);
    if (callContext == m_callContexts.end())
        return 0;

    return callContext->second;
}

void CallContextRegister::AssociateCallContext(unsigned int uiCallId, unsigned int uiAssociatedId)
{
    m_associatedIds.insert(std::make_pair(uiCallId, uiAssociatedId));
}

void CallContextRegister::GetAssociatedId(unsigned int uiCallId, qVector<unsigned int>* pAssociatedIds)
{
    pAssociatedIds->clear();

    std::pair<AssociatedIdMap::iterator, AssociatedIdMap::iterator> associatedIds = m_associatedIds.equal_range(
        uiCallId);
    for (; associatedIds.first != associatedIds.second; ++associatedIds.first)
        pAssociatedIds->push_back(associatedIds.first->second);
}

void CallContextRegister::CancelCallContexts(void* pConnection, unsigned int uiConnectionId)
{
    qVector<unsigned int> associatedIds;

    for (CallContextMap::iterator callContext = m_callContexts.begin(); callContext != m_callContexts.end();
         ++callContext)
    {
        CallContext* pCallContext = callContext->second;

        if (pCallContext->m_state != CallContext::CallInProgress)
            continue;

        if (pCallContext->m_pDependentConnection != pConnection
            && (uiConnectionId == 0 || pCallContext->m_uiDependentConnectionId != uiConnectionId))
            continue;

        qVector<unsigned int> callAssociatedIds;

        Core::GetInstance()->GetCallContextRegister()->GetAssociatedId(pCallContext->m_uiCallId, &callAssociatedIds);

        associatedIds.insert(associatedIds.end(), callAssociatedIds.begin(), callAssociatedIds.end());

        pCallContext->SignalFailure(qResult(CALL_CONTEXT_RESULT_CONNECTION_LOST));
    }

    for (qVector<unsigned int>::iterator associatedId = associatedIds.begin(); associatedId != associatedIds.end();
         ++associatedId)
    {
        CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(*associatedId);

        if (pCallContext != 0)
            pCallContext->Cancel(CallContext::CallCanceled);
    }
}

void CallContextRegister::CancelExpiredCalls()
{
    Time currentTime(Time::GetTime());

    qVector<unsigned int> associatedIds;

    for (CallContextMap::iterator callContext = m_callContexts.begin(); callContext != m_callContexts.end();
         ++callContext)
    {
        CallContext* pCallContext = callContext->second;

        if (pCallContext->m_state != CallContext::CallInProgress)
            continue;

        if (pCallContext->m_expirationTime == 0)
            continue;

        if (!(pCallContext->m_expirationTime < currentTime))
            continue;

        qVector<unsigned int> callAssociatedIds;

        Core::GetInstance()->GetCallContextRegister()->GetAssociatedId(pCallContext->m_uiCallId, &callAssociatedIds);

        associatedIds.insert(associatedIds.end(), callAssociatedIds.begin(), callAssociatedIds.end());

        pCallContext->SignalFailure(qResult(CALL_CONTEXT_RESULT_TIMEOUT));
    }

    for (qVector<unsigned int>::iterator associatedId = associatedIds.begin(); associatedId != associatedIds.end();
         ++associatedId)
    {
        CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(*associatedId);

        if (pCallContext != 0)
            pCallContext->Cancel(CallContext::CallCanceled);
    }
}

void CallContextRegister::CheckExpiredCalls(int)
{
    CancelExpiredCalls();
}

void CallContextRegister::Trace(unsigned long long)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    for (CallContextMap::iterator callContext = m_callContexts.begin(); callContext != m_callContexts.end();
         ++callContext)
    {
    }
}

void CallContextRegister::Start()
{
    m_pCheckExpiredCallsJob = new MethodCallJob<CallContextRegister, int, PeriodicJob>(
        Job::JOB_TYPE_0, String(L"CallContextRegister::CheckExpiredCalls"), &CallContextRegister::CheckExpiredCalls,
        this);

    m_pCheckExpiredCallsJob->SetPeriod(g_checkExpiredCallsPeriod.m_value);
    m_pCheckExpiredCallsJob->AddRef();

    Scheduler::GetInstance()->Queue(m_pCheckExpiredCallsJob, false);
}

void CallContextRegister::Stop()
{
    if (!Scheduler::GetInstance()->Cancel(m_pCheckExpiredCallsJob))
        m_pCheckExpiredCallsJob->SetToCancel();

    m_pCheckExpiredCallsJob->ReleaseRef();
    m_pCheckExpiredCallsJob = 0;
}

}
}
