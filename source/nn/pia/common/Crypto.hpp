#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Crypto
{
public:
    enum Mode
    {
        cModeNone = 0,
        cModeAesGcm = 1
    };

    class Setting
    {
    public:
        Mode m_mode;
        const void* m_iv;
        unsigned int m_ivLength;
        const void* m_key;
        unsigned int m_keyLength;
    };

    static Result Encrypt(void* destination, const void* source, unsigned int size, void* signature,
                          unsigned int signatureSize, const Setting& setting);
    static Result Decrypt(void* destination, const void* source, unsigned int size, void* signature,
                          unsigned int signatureSize, const Setting& setting);
    static unsigned int GetBlockSize(Mode mode);

private:
    enum
    {
        cKeySize = 16,
        cBlockSize = 16,
        cResultInvalidArgument = 0x10c07,
        cResultMacMismatch = 0x2c03
    };
};

}
}
}

namespace nn
{
namespace pia
{
namespace common
{
namespace crypto
{

void ClearMemory(void* destination, unsigned int size);
unsigned long long GenerateRandomSeed();
void GenerateRandomParameter(void* destination, unsigned int size);

}
}
}
}

#endif
