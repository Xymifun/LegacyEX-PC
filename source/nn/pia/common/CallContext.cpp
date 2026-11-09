#include "nn/pia/common/CallContext.hpp"

namespace nn
{
namespace pia
{
namespace common
{


CallContext::CallContext() : m_state(cStateNotCalled), m_result(0)
{
    Reset();
}

CallContext::~CallContext()
{
}

void CallContext::Reset()
{
    m_completionCallbackParam = 0;
    m_cancelRequested = false;
    m_completionCallback = 0;
    m_state = cStateNotCalled;
    m_result = 0;
}

void CallContext::RegisterCompletionCallback(CompletionCallback callback, void* param)
{
    m_completionCallback = callback;
    m_completionCallbackParam = param;
}

void CallContext::SetState(State state)
{
    m_state = state;
}

bool CallContext::InitiateCall()
{
    SetState(cStateInProgress);
    m_cancelRequested = false;
    return true;
}

void CallContext::SetResult(Result result)
{
    m_result = result;
}

void CallContext::Complete(Result result)
{
    if (m_completionCallback != 0)
        m_completionCallback(result, m_completionCallbackParam);
}

void CallContext::SignalSuccess(Result result)
{
    SetResult(result);
    SetState(cStateSuccess);
    Complete(m_result);
}

void CallContext::SignalFailure(Result result)
{
    SetResult(result);
    SetState(cStateFailure);
    Complete(m_result);
}

void CallContext::SignalCancel()
{
    SetResult(static_cast<Result>(0x6c05));
    SetState(cStateCancelled);
    Complete(m_result);
}

void CallContext::Cancel()
{
    m_cancelRequested = true;
}

}
}
}
