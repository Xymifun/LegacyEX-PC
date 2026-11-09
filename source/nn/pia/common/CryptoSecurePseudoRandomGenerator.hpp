#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_SECURE_PSEUDO_RANDOM_GENERATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_SECURE_PSEUDO_RANDOM_GENERATOR_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace common
{

class CryptoSecurePseudoRandomGenerator
{
public:
    CryptoSecurePseudoRandomGenerator();
    ~CryptoSecurePseudoRandomGenerator();
    void UpdateSetting(unsigned long long first, unsigned long long second);
    unsigned int GetU32();

private:
    uint64_t m_counter;
    unsigned char m_unknown08[16];
    uint64_t m_seed;
    uint64_t m_setting0;
    uint64_t m_setting1;
};

}
}
}

#endif
