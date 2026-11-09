#include "nn/nex/ChecksumAlgorithm.hpp"

#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/JobDeriveKey.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

}

ChecksumAlgorithm::ChecksumAlgorithm() : m_uiChecksumLength(0)
{
}

unsigned char ChecksumAlgorithm::ReduceChecksumLength(unsigned char uiLength)
{
    m_uiChecksumLength = uiLength;
    if (m_uiChecksumLength == 0)
        m_uiChecksumLength = GetChecksumLength();

    if (m_uiChecksumLength > GetChecksumLength())
        m_uiChecksumLength = GetChecksumLength();

    return m_uiChecksumLength;
}

void ChecksumAlgorithm::CompressChecksum(Buffer* pChecksum)
{
    unsigned int uiLength = m_uiChecksumLength;
    unsigned int uiFullLength = GetChecksumLength();
    for (unsigned int i = uiLength; i < uiFullLength; ++i)
        (*pChecksum)[m_uiChecksumLength - 1] ^= (*pChecksum)[i];

    pChecksum->SetLength(m_uiChecksumLength);
}

bool ChecksumAlgorithm::AppendChecksum(Buffer* pBuffer)
{
    if (!IsReady())
        return false;

    if (m_uiChecksumLength == 0)
        m_uiChecksumLength = GetChecksumLength();

    Buffer checksum(GetChecksumLength(), 0);
    ComputeChecksum(*pBuffer, &checksum);
    if (m_uiChecksumLength < GetChecksumLength())
        CompressChecksum(&checksum);

    pBuffer->AppendData(checksum.GetData(), checksum.GetLength(), APPEND_AT_END);
    return true;
}

bool ChecksumAlgorithm::RemoveChecksum(Buffer* pBuffer)
{
    if (!IsReady())
        return false;

    if (m_uiChecksumLength == 0)
        m_uiChecksumLength = GetChecksumLength();

    if (pBuffer->GetLength() < m_uiChecksumLength)
        return false;

    Buffer received;
    pBuffer->SetLength(pBuffer->GetLength() - m_uiChecksumLength);
    received.AppendData(pBuffer->GetData() + pBuffer->GetLength(), m_uiChecksumLength, APPEND_AT_END);

    Buffer computed(GetChecksumLength(), 0);
    ComputeChecksum(*pBuffer, &computed);
    if (m_uiChecksumLength < GetChecksumLength())
        CompressChecksum(&computed);

    return received == computed;
}

Key ChecksumAlgorithm::DeriveKey(const Buffer& rBuffer, unsigned int uiIterations)
{
    Buffer current(rBuffer);
    unsigned int uiCount = 0;
    Buffer derived(GetChecksumLength(), 0);
    for (; uiCount < uiIterations; ++uiCount)
    {
        derived.Clear();
        ComputeChecksum(current, &derived);
        current = derived;
    }

    return Key(current.GetData(), current.GetLength());
}

Key ChecksumAlgorithm::DeriveKey(const char* pString, unsigned int uiIterations)
{
    Buffer buffer;
    buffer.AppendData(pString, static_cast<unsigned int>(strlen(pString)), APPEND_AT_END);
    return DeriveKey(buffer, uiIterations);
}

bool ChecksumAlgorithm::DeriveKey(CallContext* pCallContext, ChecksumAlgorithm* pChecksumAlgorithm,
                                  const Buffer& rBuffer, unsigned int uiIterations, Key* pKey, KeyCache* pKeyCache,
                                  Job::JobType jobType)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    Buffer buffer(rBuffer);
    JobDeriveKey* pJob = new JobDeriveKey(jobType, pCallContext->m_uiCallId, pChecksumAlgorithm, &buffer, uiIterations,
                                          pKey);
    pJob->m_pKeyCache = pKeyCache;
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool ChecksumAlgorithm::DeriveKey(CallContext* pCallContext, ChecksumAlgorithm* pChecksumAlgorithm, const char* pString,
                                  unsigned int uiIterations, Key* pKey, KeyCache* pKeyCache, Job::JobType jobType)
{
    Buffer buffer;
    buffer.AppendData(pString, static_cast<unsigned int>(strlen(pString)), APPEND_AT_END);
    return DeriveKey(pCallContext, pChecksumAlgorithm, buffer, uiIterations, pKey, pKeyCache, jobType);
}

}
}
