#include "nn/nex/MD5.hpp"

#include <string.h>

namespace nn
{
namespace nex
{

MD5::MD5()
{
    init();
}

void MD5::init()
{
    m_finalized = false;
    Md5Context::Initialize();
}

void MD5::update(const unsigned char* pInput, unsigned int uiLength)
{
    if (m_finalized)
        return;

    Md5Context::Update(pInput, uiLength);
}

void MD5::finalize()
{
    if (m_finalized)
        return;

    Md5Context::GetHash(m_digest);
    m_finalized = true;
}

void MD5::raw_digest(unsigned char* pDigest)
{
    if (!m_finalized)
        return;

    if (pDigest == 0)
        return;

    memcpy(pDigest, m_digest, DigestSize);
}

}
}
