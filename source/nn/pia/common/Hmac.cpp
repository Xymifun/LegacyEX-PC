#include "nn/pia/common/Hmac.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

void Hmac::Initialize(HashContextBase* context, const void* key, unsigned int keySize)
{
    if (context == 0 || keySize == 0)
    {
        m_context = 0;
        return;
    }

    m_context = context;
    unsigned int block[16];
    if (keySize > sizeof(block))
    {
        m_context->Initialize();
        m_context->Update(key, keySize);
        m_context->GetHash(block);
        keySize = m_context->GetHashSize();
    }
    else
    {
        OSBlockMove(block, key, keySize, FALSE);
    }
    const int padSize = sizeof(block) - keySize;
    if (padSize > 0)
        OSBlockSet(reinterpret_cast<unsigned char*>(block) + keySize, 0, padSize);
    for (unsigned int index = 0; index < 16; ++index)
    {
        m_innerPad[index] = block[index] ^ 0x36363636;
        m_outerPad[index] = block[index] ^ 0x5c5c5c5c;
    }
}

Hmac::Hmac()
{
    Initialize(0, 0, 0);
}

void Hmac::Calc(void* hash, const void* data, unsigned int size)
{
    if (m_context == 0)
        return;

    unsigned char inner[36];
    m_context->Initialize();
    m_context->Update(m_innerPad, sizeof(m_innerPad));
    m_context->Update(data, size);
    m_context->GetHash(inner);
    m_context->Initialize();
    m_context->Update(m_outerPad, sizeof(m_outerPad));
    m_context->Update(inner, m_context->GetHashSize());
    m_context->GetHash(hash);
}

}
}
}
