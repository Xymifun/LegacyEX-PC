#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

ProtocolCallContext::ProtocolCallContext(Credentials* pCredentials)
    : CallContext(),
      m_returnValues(),
      m_completionFlag(0),
      m_completionResult(0),
      m_credentials(0)
{
    SetCredentials(pCredentials);
}

ProtocolCallContext::~ProtocolCallContext()
{
    if (GetState() == CallInProgress)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (GetState() == CallInProgress)
            Cancel(CallCanceled);
    }
    SetCredentials(0);
}

void ProtocolCallContext::SetCredentials(Credentials* pCredentials)
{
    if (m_credentials != 0)
    {
        m_credentials->ReleaseRef();
        m_credentials = 0;
    }
    if (pCredentials != 0)
    {
        m_credentials = pCredentials;
        m_credentials->AddRef();
    }
}

void ProtocolCallContext::AddReturnValuePtr(void* pValue)
{
    m_returnValues.push_back(pValue);
}

void* ProtocolCallContext::GetReturnValuePtr(unsigned int uiIndex)
{
    if (m_returnValues[uiIndex] != 0)
        return m_returnValues[uiIndex];
    return 0;
}

void ProtocolCallContext::BeginTransition(State state, qResult result, bool publishResult)
{
    if (state == Ready)
    {
        m_returnValues.clear();
        return;
    }

    if (state != CallFailed)
        return;

    if (publishResult)
    {
        if (m_completionFlag != 0)
        {
            *m_completionFlag = false;
            m_completionFlag = 0;
        }
        if (m_completionResult != 0)
        {
            *m_completionResult = result;
            m_completionResult = 0;
        }
    }
}

}
}
