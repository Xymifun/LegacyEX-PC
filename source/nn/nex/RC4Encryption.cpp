#include "nn/nex/RC4Encryption.hpp"

#include <string.h>

namespace nn
{
namespace nex
{

RC4Encryption::RC4Encryption()
    : EncryptionAlgorithm(5, 0x40),
      m_bReinitEverytime(true),
      m_pCriticalSection(new CriticalSection())
{
    SetKey(GetDefaultKey());
}

RC4Encryption::~RC4Encryption()
{
    delete m_pCriticalSection;
}

Key RC4Encryption::GetDefaultKey()
{
    unsigned char key[5] = {'C', 'D', '&', 'M', 'L'};
    return Key(key, 5);
}

void RC4Encryption::PrepareEncryption()
{
    unsigned int i;
    for (i = 0; i < StateSize; ++i)
        m_initialState[i] = static_cast<unsigned char>(i);

    m_uiIndexJ = 0;
    m_uiIndexI = 0;
    unsigned char uiKeyIndex = 0;
    unsigned char j = 0;
    unsigned char* pKey = m_key.GetContentPtr();
    unsigned int uiKeyLength = m_key.GetLength();
    for (i = 0; i < StateSize; ++i)
    {
        unsigned char uiValue = m_initialState[i];
        j = (pKey[uiKeyIndex] + uiValue + j) % StateSize;
        m_initialState[i] = m_initialState[j];
        m_initialState[j] = uiValue;
        uiKeyIndex = (uiKeyIndex + 1) % uiKeyLength;
    }
}

void RC4Encryption::ReinitStateArray()
{
    m_uiIndexJ = 0;
    m_uiIndexI = 0;
    memcpy(m_state, m_initialState, StateSize);
}

void RC4Encryption::KeyHasChanged()
{
    PrepareEncryption();
    ReinitStateArray();
}

void RC4Encryption::EncryptDecrypt(unsigned char* pData, unsigned int uiLength)
{
    ScopedCS lock(m_pCriticalSection);
    if (m_key.GetLength() == 0)
        return;

    if (m_bReinitEverytime)
        ReinitStateArray();

    unsigned char i = m_uiIndexI;
    unsigned char j = m_uiIndexJ;
    for (unsigned int uiOffset = 0; uiOffset < uiLength; ++uiOffset)
    {
        i = (i + 1) % StateSize;
        unsigned char uiValue = m_state[i];
        j = (uiValue + j) % StateSize;
        m_state[i] = m_state[j];
        m_state[j] = uiValue;
        pData[uiOffset] ^= m_state[(m_state[i] + uiValue) % StateSize];
    }

    m_uiIndexI = i;
    m_uiIndexJ = j;
}

void RC4Encryption::Encrypt(unsigned char* pData, unsigned int uiLength)
{
    EncryptDecrypt(pData, uiLength);
}

int RC4Encryption::Encrypt(const Buffer& rSource, Buffer* pDestination)
{
    *pDestination = rSource;
    Encrypt(pDestination);
    return 0;
}

int RC4Encryption::Encrypt(Buffer* pBuffer)
{
    Encrypt(pBuffer->GetData(), pBuffer->GetLength());
    return 0;
}

void RC4Encryption::Decrypt(unsigned char* pData, unsigned int uiLength)
{
    EncryptDecrypt(pData, uiLength);
}

int RC4Encryption::Decrypt(const Buffer& rSource, Buffer* pDestination)
{
    *pDestination = rSource;
    Decrypt(pDestination);
    return 0;
}

int RC4Encryption::Decrypt(Buffer* pBuffer)
{
    Decrypt(pBuffer->GetData(), pBuffer->GetLength());
    return 0;
}

void RC4Encryption::SetReinitEverytime(bool bReinit)
{
    ScopedCS lock(m_pCriticalSection);
    m_bReinitEverytime = bReinit;
}

}
}
