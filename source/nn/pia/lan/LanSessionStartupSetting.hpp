#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP

#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanSessionStartupSetting : public session::Session::StartupSetting
{
public:
    LanSessionStartupSetting() : m_playersCountedAsParticipants(true)
    {
        m_cryptoSetting.m_mode = common::Crypto::cModeAesGcm;
        m_field04 = cDefaultField04;
        m_field00 = cDefaultField00;
        for (unsigned int index = 0; index < cCryptoKeySize; ++index)
            m_cryptoSetting.m_key[index] = 0;
    }

    unsigned int m_payloadWord;
    transport::Station::PlayerInfo m_playerInfo[1];
    bool m_playersCountedAsParticipants;
    common::CryptoSetting m_cryptoSetting;

private:
    enum
    {
        cDefaultField00 = 4000,
        cDefaultField04 = 400,
        cCryptoKeySize = 0x10
    };
};

}
}
}

#endif
