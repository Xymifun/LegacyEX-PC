#include "nn/nex/JobRetrieveGameAuthToken.hpp"

#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum RetrieveResult
{
    RETRIEVE_SUCCEEDED_RESULT = 0x10001,
    RETRIEVE_ABORTED_RESULT = static_cast<int>(0x80010004u)
};

enum
{
    CACHED_KEY_CAPACITY = 64
};

unsigned int s_uiCachedPrincipalId = 0;
char s_acCachedKey[CACHED_KEY_CAPACITY] = "";

}

JobRetrieveGameAuthToken::JobRetrieveGameAuthToken()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_uiPrincipalId(0),
      m_password(),
      m_pKey(0)
{
}

JobRetrieveGameAuthToken::~JobRetrieveGameAuthToken()
{
}

qResult JobRetrieveGameAuthToken::Initialize(unsigned int uiCallId, unsigned int uiPrincipalId, const String& rPassword,
                                             String* pToken, String* pKey)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_uiPrincipalId = uiPrincipalId;
    m_password = rPassword;
    m_pKey = pKey;
    *pToken = L"normaltoken";
    if (s_uiCachedPrincipalId == uiPrincipalId && strncmp(s_acCachedKey, "", CACHED_KEY_CAPACITY) != 0)
    {
        *m_pKey = s_acCachedKey;
        SetStep(Step(static_cast<Step::Function>(&JobRetrieveGameAuthToken::StepDummyToComplete), L""));
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobRetrieveGameAuthToken::StepDeriveKey), L""));
    }

    return qResult(true);
}

void JobRetrieveGameAuthToken::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
        {
            char* pKey = 0;
            m_pKey->CreateCopy(&pKey);
            strncpy(s_acCachedKey, pKey, CACHED_KEY_CAPACITY);
            String::ReleaseCopy(pKey);
            pCallContext->SignalSuccess(rResult);
        }
        else
        {
            pCallContext->SignalFailure(rResult);
        }
    }

    SetToComplete();
}

void JobRetrieveGameAuthToken::StepDeriveKey()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RETRIEVE_ABORTED_RESULT));
        return;
    }

    char* pPassword = 0;
    m_password.CreateCopy(&pPassword);
    *m_pKey = KerberosEncryption::CreateKey(m_uiPrincipalId, pPassword).ToString();
    String::ReleaseCopy(pPassword);
    s_uiCachedPrincipalId = m_uiPrincipalId;
    CompleteJob(qResult(RETRIEVE_SUCCEEDED_RESULT));
}

void JobRetrieveGameAuthToken::StepDummyToComplete()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RETRIEVE_ABORTED_RESULT));
        return;
    }

    CompleteJob(qResult(RETRIEVE_SUCCEEDED_RESULT));
}

}
}
