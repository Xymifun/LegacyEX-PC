#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KERBEROSENCRYPTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KERBEROSENCRYPTION_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/HMACChecksum.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/MD5KeyDerivation.hpp"
#include "nn/nex/RC4Encryption.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class KeyCache;

class KerberosEncryption : public RootObject
{
public:
    KerberosEncryption();
    ~KerberosEncryption();

    bool Encrypt(const Buffer& rSource, Buffer* pDestination);
    bool Decrypt(const Buffer& rSource, Buffer* pDestination);
    bool Encrypt(const Buffer& rSource, Buffer* pDestination, const Key& rKey);
    bool Decrypt(const Buffer& rSource, Buffer* pDestination, const Key& rKey);
    void SetKey(const Key& rKey);
    Key GenerateConcatenateKey(const Key& rKey) const;

    static bool CreateKey(CallContext* pCallContext, unsigned int uiPrincipalID, const char* pPassword, Key* pKey);
    static Key CreateKey(unsigned int uiPrincipalID, const char* pPassword);
    static bool InitializeKey(CallContext* pCallContext, const char* pPassword, Key* pKey);
    static Key InitializeKey(const char* pPassword);
    static bool FinalizeKey(CallContext* pCallContext, unsigned int uiPrincipalID, const Key& rKey, Key* pFinalKey);
    static Key FinalizeKey(unsigned int uiPrincipalID, const Key& rKey);
    static void SetKeyDerivation(KeyDerivation* pKeyDerivation);
    static void EnablePasswordCache();
    static void DisablePasswordCache();
    static Key GenerateConcatenateKey(const Key& rFirstKey, const Key& rSecondKey);

private:
    RC4Encryption m_encryption;
    HMACChecksum m_checksum;
    Key m_key;

    static KeyDerivation* s_pKeyDerivation;
    static KeyCache* s_pKeyCache;
    static MD5KeyDerivation s_defaultKeyDerivation;
};

}
}

#endif
