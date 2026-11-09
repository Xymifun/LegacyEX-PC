#include "nn/nex/MD5KeyDerivation.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ChecksumAlgorithm.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/MD5Checksum.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int DEFAULT_CLIENT_SIDE_ITERATIONS = 65000;
const unsigned int DEFAULT_PRINCIPAL_ID_MODULO = 1024;

}

MD5KeyDerivation::MD5KeyDerivation()
    : m_uiClientSideIterations(DEFAULT_CLIENT_SIDE_ITERATIONS),
      m_uiPrincipalIdModulo(DEFAULT_PRINCIPAL_ID_MODULO)
{
}

MD5KeyDerivation::~MD5KeyDerivation()
{
}

Key MD5KeyDerivation::CreateKey(unsigned int uiPrincipalID, const char* pPassword)
{
    const unsigned int uiIterations = m_uiClientSideIterations + uiPrincipalID % m_uiPrincipalIdModulo;
    MD5Checksum checksum;
    return checksum.DeriveKey(pPassword, uiIterations);
}

Key MD5KeyDerivation::InitializeKey(const char* pPassword)
{
    const unsigned int uiIterations = m_uiClientSideIterations;
    MD5Checksum checksum;
    return checksum.DeriveKey(pPassword, uiIterations);
}

Key MD5KeyDerivation::FinalizeKey(unsigned int uiPrincipalID, const Key& rKey)
{
    Buffer buffer;
    buffer.AppendData(rKey.GetContentPtr(), rKey.GetLength(), 0xFFFFFFFF);
    const unsigned int uiIterations = uiPrincipalID % m_uiPrincipalIdModulo;
    MD5Checksum checksum;
    return checksum.DeriveKey(buffer, uiIterations);
}

bool MD5KeyDerivation::CreateKey(CallContext* pCallContext, unsigned int uiPrincipalID, const char* pPassword,
                                 Key* pKey, KeyCache* pKeyCache)
{
    const unsigned int uiIterations = m_uiClientSideIterations + uiPrincipalID % m_uiPrincipalIdModulo;
    return ChecksumAlgorithm::DeriveKey(pCallContext, new MD5Checksum, pPassword, uiIterations, pKey, pKeyCache,
                                        Job::JOB_TYPE_2);
}

bool MD5KeyDerivation::InitializeKey(CallContext* pCallContext, const char* pPassword, Key* pKey)
{
    const unsigned int uiIterations = m_uiClientSideIterations;
    return ChecksumAlgorithm::DeriveKey(pCallContext, new MD5Checksum, pPassword, uiIterations, pKey, 0,
                                        Job::JOB_TYPE_2);
}

bool MD5KeyDerivation::FinalizeKey(CallContext* pCallContext, unsigned int uiPrincipalID, const Key& rKey,
                                   Key* pFinalKey)
{
    Buffer buffer;
    buffer.AppendData(rKey.GetContentPtr(), rKey.GetLength(), 0xFFFFFFFF);
    const unsigned int uiIterations = uiPrincipalID % m_uiPrincipalIdModulo;
    return ChecksumAlgorithm::DeriveKey(pCallContext, new MD5Checksum, buffer, uiIterations, pFinalKey, 0,
                                        Job::JOB_TYPE_2);
}

}
}
