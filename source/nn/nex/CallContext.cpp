#include "nn/nex/CallContext.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SpinTest.hpp"
#include "nn/nex/SystemError.hpp"

bool CallContextPreventBlockCallCallback(nn::nex::RootObject* pObject, unsigned int uiTimeout);

namespace nn
{
namespace nex
{

const int CALL_CONTEXT_RESULT_PENDING = static_cast<int>(0x80010001u);

CallbackRoot::~CallbackRoot()
{
}

void CallbackRoot::Call()
{
    CallObjectMethod();
}

CallContext::CallContext()
    : RefCountedObject(),
      m_state(Ready),
      m_completionCallbacks(),
      m_userContexts(),
      m_result(),
      m_uiCallId(0),
      m_pDependentConnection(0),
      m_pCancellationCallback(0),
      m_expirationTime()
{
    m_result = qResult(CALL_CONTEXT_RESULT_PENDING);
    m_uiFlags = 0;
    m_expirationTime = Time(0);
}

CallContext::~CallContext()
{
    if (Scheduler::GetInstance() != 0)
    {
        Scheduler* pScheduler = Scheduler::GetInstance();
        pScheduler->GetCriticalSection().Enter();

        if (m_state == CallInProgress)
            Cancel(CallCanceled);

        if (IsRegistered())
            Unregister();

        pScheduler->GetCriticalSection().Leave();
    }

    if (m_pCancellationCallback != 0)
    {
        delete m_pCancellationCallback;
        m_pCancellationCallback = 0;
    }

    m_expirationTime = Time(0);
}

bool CallContext::Wait(unsigned int uiTimeout)
{
    if (m_state == CallInProgress)
    {
        bool bCompleted = false;

        if (Scheduler::PreventBlockCall(uiTimeout != 0, uiTimeout, false, &CallContextPreventBlockCallCallback, this,
                                        &bCompleted)
            != 0)
            return bCompleted;

        SpinTest spinTest(10, uiTimeout);
        spinTest.LeaveOnTimeout();

        while (m_state == CallInProgress)
        {
            Scheduler::GlobalSingleThreadDispatch(spinTest.GetRemainingTime());

            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/"
                                  L"CallContext.cpp",
                                  0x1A4, L"GetState()==CallContext::CallInProgress")
                == 0)
                break;
        }
    }

    return m_state >= CallSucceeded && m_state <= CallCanceled;
}

bool CallContext::IsRegistered() const
{
    return m_uiCallId != 0;
}

void CallContext::Register()
{
    m_uiCallId = Core::GetInstance()->GetCallContextRegister()->Register(this);
}

void CallContext::Unregister()
{
    Core::GetInstance()->GetCallContextRegister()->Unregister(m_uiCallId);
    m_uiCallId = 0;
}

bool CallContext::Reset()
{
    if (m_state == CallInProgress)
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/CallContext.cpp",
                                 0xDB, 0xE000000E, 0);
        return false;
    }

    if (m_state == Ready)
        return true;

    SetStateImpl(Ready, qResult(CALL_CONTEXT_RESULT_PENDING), false);

    m_expirationTime = Time(0);
    m_completionCallbacks.clear();

    if (m_pCancellationCallback != 0)
    {
        delete m_pCancellationCallback;
        m_pCancellationCallback = 0;
    }

    SetDependentConnection(0, 0);
    return true;
}

bool CallContext::InitiateCall()
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();

    if (!SetCallInProgressState())
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }

    Register();

    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool CallContext::InitiateSpecialCall()
{
    return SetCallInProgressState();
}

bool CallContext::SetCallInProgressState()
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();

    if (m_state == CallInProgress)
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }

    if (m_state >= CallSucceeded && m_state <= CallCanceled)
        Reset();

    SetStateImpl(CallInProgress, qResult(CALL_CONTEXT_RESULT_PENDING), false);

    pScheduler->GetCriticalSection().Leave();
    return true;
}

void CallContext::SignalSuccess(qResult result)
{
    SetStateImpl(CallSucceeded, result, true);
}

void CallContext::SignalFailure(qResult result)
{
    SetStateImpl(CallFailed, result, true);
}

bool CallContext::Cancel(State state)
{
    bool bCanceled;

    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();

    {
        qVector<unsigned int> associatedIds;
        Core::GetInstance()->GetCallContextRegister()->GetAssociatedId(m_uiCallId, &associatedIds);

        bCanceled = CancelImpl(state);

        for (qVector<unsigned int>::iterator associatedId = associatedIds.begin(); associatedId != associatedIds.end();
             ++associatedId)
        {
            CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(*associatedId);

            if (pCallContext != 0)
                pCallContext->Cancel(CallCanceled);
        }
    }

    pScheduler->GetCriticalSection().Leave();
    return bCanceled;
}

bool CallContext::CancelImpl(State state)
{
    if (m_state >= CallSucceeded && m_state <= CallCanceled)
        return true;

    if (m_state != CallInProgress)
        return false;

    if (!TransitionIsValid(m_state, state))
        return false;

    AddRef();

    SetStateImpl(state, qResult(CALL_CONTEXT_RESULT_PENDING), false);

    if (m_pCancellationCallback != 0)
        m_pCancellationCallback->Call();

    ReleaseRef();
    return true;
}

bool CallContext::TransitionIsValid(State currentState, State newState)
{
    if (currentState == Ready && newState == CallInProgress)
        return true;

    if (currentState == CallInProgress
        && (newState == CallSucceeded || newState == CallFailed || newState == CallCanceled))
        return true;

    if (currentState == CallSucceeded && newState == Ready)
        return true;

    if (currentState == CallFailed && newState == Ready)
        return true;

    if (currentState == CallCanceled && newState == Ready)
        return true;

    return false;
}

void CallContext::SetStateImpl(State state, qResult result, bool bPublishResult)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();

    if (state == Ready)
        m_result = qResult(CALL_CONTEXT_RESULT_PENDING);

    if (m_state == CallInProgress && IsRegistered())
        Unregister();

    BeginTransition(state, result, bPublishResult);

    m_state = state;

    if (bPublishResult)
        m_result = result;

    pScheduler->GetCriticalSection().Leave();

    if (state >= CallSucceeded && state <= CallCanceled)
    {
        for (CompletionCallbackList::iterator completionCallback = m_completionCallbacks.begin();
             completionCallback != m_completionCallbacks.end(); ++completionCallback)
        {
            CompletionCallback cbCallback = completionCallback->m_cbCallback;
            UserContext userContext = completionCallback->m_userContext;
            cbCallback(this, &userContext);
        }

        ProcessCallCompletion();

        if (FlagIsSet(CALL_CONTEXT_FLAG_SELF_RELEASE))
            ReleaseRef();
    }
}

void CallContext::SetDependentConnection(void* pConnection, unsigned int uiConnectionId)
{
    m_pDependentConnection = pConnection;
    m_uiDependentConnectionId = uiConnectionId;
}

void CallContext::SetFlag(unsigned int uiFlag)
{
    m_uiFlags |= uiFlag;
}

bool CallContext::FlagIsSet(unsigned int uiFlag) const
{
    return (m_uiFlags & uiFlag) == uiFlag;
}

bool CallContext::FlagsAreValid() const
{
    if (!FlagIsSet(CALL_CONTEXT_FLAG_CALLBACK_REQUIRED))
        return true;

    return FlagIsSet(CALL_CONTEXT_FLAG_CALLBACK_EXPECTED);
}

void CallContext::RegisterCompletionCallback(CompletionCallback cbCallback, const UserContext& rUserContext,
                                             bool bAppend)
{
    InternalData internalData;
    internalData.m_cbCallback = cbCallback;
    internalData.m_userContext = rUserContext;

    if (bAppend)
        m_completionCallbacks.push_back(internalData);
    else
        m_completionCallbacks.push_front(internalData);
}

void CallContext::RegisterCompletionCallback(CallbackRoot* pCallback, bool bOnSuccessOnly, bool bAppend)
{
    if (bOnSuccessOnly)
        RegisterCompletionCallback(&InvokeCallbackOnSuccess, UserContext(pCallback), bAppend);
    else
        RegisterCompletionCallback(&InvokeCallbackOnCompletion, UserContext(pCallback), bAppend);
}

void CallContext::BeginTransition(State, qResult, bool)
{
}

void CallContext::ProcessCallCompletion()
{
}

void InvokeCallbackOnSuccess(CallContext* pCallContext, const UserContext* pUserContext)
{
    CallbackRoot* pCallback = static_cast<CallbackRoot*>(pUserContext->GetPointer());

    if (pCallContext->GetState() == CallContext::CallSucceeded)
    {
        pCallback->Call();
        delete pCallback;
        return;
    }

    if (pCallback != 0)
        delete pCallback;
}

void InvokeCallbackOnCompletion(CallContext*, const UserContext* pUserContext)
{
    CallbackRoot* pCallback = static_cast<CallbackRoot*>(pUserContext->GetPointer());

    pCallback->Call();
    delete pCallback;
}

}
}

bool CallContextPreventBlockCallCallback(nn::nex::RootObject* pObject, unsigned int uiTimeout)
{
    return static_cast<nn::nex::CallContext*>(pObject)->Wait(uiTimeout);
}
