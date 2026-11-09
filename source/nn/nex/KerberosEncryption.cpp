#include "nn/nex/KerberosEncryption.hpp"

#include "nn/nex/BitStream.hpp"
#include "nn/nex/KeyCache.hpp"
#include "nn/nex/MD5Checksum.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int BITS_PER_BYTE = 8;

}

KeyDerivation* KerberosEncryption::s_pKeyDerivation = &s_defaultKeyDerivation;
KeyCache* KerberosEncryption::s_pKeyCache = 0;
MD5KeyDerivation KerberosEncryption::s_defaultKeyDerivation;

KerberosEncryption::KerberosEncryption() : m_encryption(), m_checksum(), m_key()
{
}

KerberosEncryption::~KerberosEncryption()
{
}

bool KerberosEncryption::Decrypt(const Buffer& rSource, Buffer* pDestination)
{
    Buffer buffer(rSource);
    if (!m_checksum.RemoveChecksum(&buffer))
        return false;
    return m_encryption.Decrypt(buffer, pDestination) == 0;
}

bool KerberosEncryption::Encrypt(const Buffer& rSource, Buffer* pDestination, const Key& rKey)
{
    RC4Encryption encryption;
    HMACChecksum checksum;
    if (!checksum.SetKey(rKey) || !encryption.SetKey(rKey) || encryption.Encrypt(rSource, pDestination) != 0)
    {
        return false;
    }
    return checksum.AppendChecksum(pDestination);
}

bool KerberosEncryption::Decrypt(const Buffer& rSource, Buffer* pDestination, const Key& rKey)
{
    HMACChecksum checksum;
    if (!checksum.SetKey(rKey))
        return false;
    Buffer buffer(rSource);
    if (!checksum.RemoveChecksum(&buffer))
        return false;
    RC4Encryption encryption;
    if (!encryption.SetKey(rKey))
        return false;
    return encryption.Decrypt(buffer, pDestination) == 0;
}

bool KerberosEncryption::CreateKey(CallContext* pCallContext, unsigned int uiPrincipalID, const char* pPassword,
                                   Key* pKey)
{
    return s_pKeyDerivation->CreateKey(pCallContext, uiPrincipalID, pPassword, pKey, s_pKeyCache);
}

Key KerberosEncryption::CreateKey(unsigned int uiPrincipalID, const char* pPassword)
{
    if (s_pKeyDerivation == 0)
        s_pKeyDerivation = &s_defaultKeyDerivation;
    if (s_pKeyCache != 0)
    {
        Key key;
        if (!s_pKeyCache->RetrieveKey(uiPrincipalID, pPassword, &key))
        {
            key = s_pKeyDerivation->CreateKey(uiPrincipalID, pPassword);
            s_pKeyCache->AddKey(uiPrincipalID, pPassword, key);
        }
        return key;
    }
    return s_pKeyDerivation->CreateKey(uiPrincipalID, pPassword);
}

bool KerberosEncryption::InitializeKey(CallContext* pCallContext, const char* pPassword, Key* pKey)
{
    if (s_pKeyDerivation == 0)
        s_pKeyDerivation = &s_defaultKeyDerivation;
    return s_pKeyDerivation->InitializeKey(pCallContext, pPassword, pKey);
}

Key KerberosEncryption::InitializeKey(const char* pPassword)
{
    if (s_pKeyDerivation == 0)
        s_pKeyDerivation = &s_defaultKeyDerivation;
    return s_pKeyDerivation->InitializeKey(pPassword);
}

bool KerberosEncryption::FinalizeKey(CallContext* pCallContext, unsigned int uiPrincipalID, const Key& rKey,
                                     Key* pFinalKey)
{
    if (s_pKeyDerivation == 0)
        s_pKeyDerivation = &s_defaultKeyDerivation;
    return s_pKeyDerivation->FinalizeKey(pCallContext, uiPrincipalID, rKey, pFinalKey);
}

Key KerberosEncryption::FinalizeKey(unsigned int uiPrincipalID, const Key& rKey)
{
    if (s_pKeyDerivation == 0)
        s_pKeyDerivation = &s_defaultKeyDerivation;
    return s_pKeyDerivation->FinalizeKey(uiPrincipalID, rKey);
}

void KerberosEncryption::SetKey(const Key& rKey)
{
    m_encryption.SetKey(rKey);
    m_checksum.SetKey(rKey);
    m_key = rKey;
}

Key KerberosEncryption::GenerateConcatenateKey(const Key& rFirstKey, const Key& rSecondKey)
{
    BitStream stream;
    stream.AppendRaw(rFirstKey.GetContentPtr(), rFirstKey.GetLength() * BITS_PER_BYTE);
    stream.AppendRaw(rSecondKey.GetContentPtr(), rSecondKey.GetLength() * BITS_PER_BYTE);
    MD5Checksum checksum;
    Buffer buffer;
    checksum.ComputeChecksum(*stream.m_pBuffer, &buffer);
    return Key(buffer.GetData(), buffer.GetLength());
}

Key KerberosEncryption::GenerateConcatenateKey(const Key& rKey) const
{
    return GenerateConcatenateKey(m_key, rKey);
}

}
}
