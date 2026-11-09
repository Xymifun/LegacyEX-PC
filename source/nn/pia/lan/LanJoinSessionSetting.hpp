#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_JOIN_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_JOIN_SESSION_SETTING_HPP

#include "nn/pia/session/JoinSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanJoinSessionSetting : public session::JoinSessionSetting
{
public:
    LanJoinSessionSetting();
    virtual ~LanJoinSessionSetting();
    void Reset();
    void SetSessionId(unsigned int sessionId);

private:
};

}
}
}

#endif
