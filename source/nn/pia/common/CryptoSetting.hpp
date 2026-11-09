#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_SETTING_HPP

namespace nn
{
namespace pia
{
namespace common
{

class CryptoSetting
{
public:
    CryptoSetting();

    unsigned int m_mode;
    unsigned char m_key[16];
};

}
}
}

#endif
