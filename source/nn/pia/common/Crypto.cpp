#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/common.hpp"


namespace nn
{
namespace pia
{
namespace common
{


unsigned int Crypto::GetBlockSize(Mode mode)
{
    if (mode == cModeAesGcm)
        return cBlockSize;
    return 0;
}

Result Crypto::Encrypt(void* destination, const void* source, unsigned int size, void* signature,
                       unsigned int signatureSize, const Setting& setting)
{
    if (setting.m_mode == cModeNone)
        return 0;
    if (setting.m_mode == cModeAesGcm)
    {
        {
            AesEncryptor<cKeySize> aes;
            GcmMode gcm;
            if (setting.m_keyLength == cKeySize)
            {
                aes.Initialize(setting.m_key, cKeySize);
                gcm.Initialize(aes);
                gcm.Reset(setting.m_iv, setting.m_ivLength);
                if ((size & (cBlockSize - 1)) == 0)
                {
                    gcm.EncryptUpdate(destination, size, source, size);
                    if (signature != 0 && signatureSize == cBlockSize)
                        gcm.GetMac(signature, cBlockSize);
                }
                else
                    return cResultInvalidArgument;
            }
            else
                return cResultInvalidArgument;
        }
        return 0;
    }
    return cResultInvalidArgument;
}

Result Crypto::Decrypt(void* destination, const void* source, unsigned int size, void* signature,
                       unsigned int signatureSize, const Setting& setting)
{
    if (setting.m_mode == cModeNone)
        return 0;
    if (setting.m_mode == cModeAesGcm)
    {
        {
            AesEncryptor<cKeySize> aes;
            GcmMode gcm;
            if (setting.m_keyLength == cKeySize)
            {
                aes.Initialize(setting.m_key, cKeySize);
                gcm.Initialize(aes);
                gcm.Reset(setting.m_iv, setting.m_ivLength);
                if ((size & (cBlockSize - 1)) == 0)
                {
                    gcm.DecryptUpdate(destination, size, source, size);
                    if (signature != 0 && signatureSize == cBlockSize)
                    {
                        unsigned char mac[cBlockSize];
                        gcm.GetMac(mac, cBlockSize);
                        for (unsigned int index = 0; index < cBlockSize; ++index)
                        {
                            if (static_cast<const unsigned char*>(signature)[index] != mac[index])
                                return cResultMacMismatch;
                        }
                    }
                }
                else
                    return cResultInvalidArgument;
            }
            else
                return cResultInvalidArgument;
        }
        return 0;
    }
    return cResultInvalidArgument;
}

}
}
}
