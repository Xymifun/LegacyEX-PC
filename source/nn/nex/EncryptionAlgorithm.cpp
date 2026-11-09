#include "nn/nex/EncryptionAlgorithm.hpp"

#include <wchar.h>

namespace nn
{
namespace nex
{

namespace
{

const int ENCRYPTION_ERROR_NULL_BUFFER = 3;

}

EncryptionAlgorithm::EncryptionAlgorithm(unsigned int uiMinKeyLength, unsigned int uiMaxKeyLength)
    : m_uiMinKeyLength(uiMinKeyLength),
      m_uiMaxKeyLength(uiMaxKeyLength)
{
}

EncryptionAlgorithm::~EncryptionAlgorithm()
{
}

int EncryptionAlgorithm::Encrypt(Buffer* pBuffer)
{
    Buffer encrypted;
    int iResult = Encrypt(*pBuffer, &encrypted);
    if (iResult == 0)
        *pBuffer = encrypted;

    return iResult;
}

int EncryptionAlgorithm::Decrypt(Buffer* pBuffer)
{
    if (pBuffer == 0)
        return ENCRYPTION_ERROR_NULL_BUFFER;

    Buffer decrypted;
    int iResult = Decrypt(*pBuffer, &decrypted);
    if (iResult == 0)
        *pBuffer = decrypted;

    return iResult;
}

bool EncryptionAlgorithm::GetErrorString(unsigned int uiError, wchar_t* pString, unsigned int uiLength)
{
    if (wcslen(L"Encryption Error") >= uiLength)
        return false;

    wcscpy(pString, L"Encryption Error");
    return true;
}

bool EncryptionAlgorithm::SetKey(const Key& rKey)
{
    unsigned int uiLength = rKey.GetLength();
    if (uiLength >= m_uiMinKeyLength && uiLength <= m_uiMaxKeyLength)
    {
        m_key = rKey;
        KeyHasChanged();
        return true;
    }

    return false;
}

}
}
