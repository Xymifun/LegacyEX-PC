#include "nn/nex/JobLogin.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/JobBackEndServicesLogin.hpp"
#include "nn/nex/NgsFacade.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int WAIT_CONTEXT_LOGIN = 250;
const int WAIT_FOREVER = -1;

}

JobLogin::JobLogin()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_protocolContext(0),
      m_uiCallId(0),
      m_uiLoginParam(0),
      m_pNgsFacade(0),
      m_pCredentials(0),
      m_pFirstString(0),
      m_pSecondString(0),
      m_iTimeout(0),
      m_startTime(),
      m_authenticationInfo(),
      m_errorMessage()
{
}

JobLogin::~JobLogin()
{
    AnyObjectHolder<Data, String> emptyData;
    g_registerURLsExtraData = emptyData;
}

void JobLogin::Initialize(unsigned int uiCallId, unsigned int uiLoginParam, NgsFacade* pNgsFacade,
                          const char* pFirstString, char* pSecondString, int iTimeout)
{
    m_uiCallId = uiCallId;
    m_uiLoginParam = uiLoginParam;
    m_pFirstString = pFirstString;
    m_pSecondString = pSecondString;
    m_pNgsFacade = pNgsFacade;
    m_iTimeout = iTimeout;
    m_startTime = Time(Time::GetTime());
    SetStep(Step(static_cast<Step::Function>(&JobLogin::StepFirst), L""));
}

void JobLogin::Complete(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_pNgsFacade->m_loginResult = rResult;
    if (pCallContext != 0)
    {
        if (rResult)
            pCallContext->SignalSuccess(rResult);
        else
            pCallContext->SignalFailure(rResult);
    }

    if (GetState() != Job::Complete)
        SetToComplete();
}

bool JobLogin::Wait()
{
    if (m_iTimeout != WAIT_FOREVER)
    {
        int64_t elapsed = Time::GetTime() - m_startTime;
        if (static_cast<int>(elapsed) >= m_iTimeout)
            return false;
    }

    SetToWaiting(WAIT_CONTEXT_LOGIN);
    return true;
}

}
}
