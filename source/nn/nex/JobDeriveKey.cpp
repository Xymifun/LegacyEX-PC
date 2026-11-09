#include "nn/nex/JobDeriveKey.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ChecksumAlgorithm.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/KeyCache.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum DeriveKeyResult
{
    KEY_DERIVED_RESULT = 0x10001
};

const unsigned int TIME_CHECK_INTERVAL_MASK = 0x3F;

}

JobDeriveKey::JobDeriveKey(JobType jobType, unsigned int uiCallId, ChecksumAlgorithm* pChecksumAlgorithm,
                           const Buffer* pBuffer, unsigned int uiIterations, Key* pKey)
    : Job(jobType, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pChecksumAlgorithm(pChecksumAlgorithm),
      m_current(),
      m_uiCompletedIterations(0),
      m_uiIterations(uiIterations),
      m_pKey(pKey),
      m_cacheKey()
{
    m_current = *pBuffer;
    m_pKeyCache = 0;
    m_cacheKey = *pBuffer;
}

JobDeriveKey::~JobDeriveKey()
{
    delete m_pChecksumAlgorithm;
}

void JobDeriveKey::CompleteJob(const Key& rKey)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        *m_pKey = rKey;
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(KEY_DERIVED_RESULT), true);
    }

    SetToComplete();
}

void JobDeriveKey::Execute()
{
    if (m_uiCompletedIterations == 0 && m_pKeyCache != 0)
    {
        Key cachedKey;
        if (m_pKeyCache->RetrieveKey(m_uiIterations, m_cacheKey, &cachedKey))
        {
            CompleteJob(cachedKey);
            return;
        }
    }

    unsigned char uiChecksumLength = m_pChecksumAlgorithm->GetChecksumLength();
    Buffer derived(uiChecksumLength, 0);
    bool bTimedOut;
    while (m_uiIterations > m_uiCompletedIterations)
    {
        derived.Clear();
        m_pChecksumAlgorithm->ComputeChecksum(m_current, &derived);
        m_current = derived;
        ++m_uiCompletedIterations;
        bTimedOut = Time::ConvertDeadlineToTimeout(m_executionTime) == 0;
        if (bTimedOut || (m_uiCompletedIterations & TIME_CHECK_INTERVAL_MASK) == 0)
            break;
    }

    if (m_uiCompletedIterations == m_uiIterations)
    {
        Key derivedKey(m_current.GetData(), m_current.GetLength());
        CompleteJob(derivedKey);
        if (m_pKeyCache != 0)
            m_pKeyCache->AddKey(m_uiIterations, m_cacheKey, derivedKey);
    }
    else
    {
        SetToWaiting(0);
    }
}

}
}
