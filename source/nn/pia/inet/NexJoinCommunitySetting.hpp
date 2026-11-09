#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_JOIN_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_JOIN_COMMUNITY_SETTING_HPP

#include "nn/pia/String.hpp"
#include "nn/pia/session/JoinCommunitySetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexJoinCommunitySetting : public session::JoinCommunitySetting
{
public:
    NexJoinCommunitySetting();
    virtual void Reset();

    const common::String& GetCommunityUserPassword() const;
    void SetCommunityUserPassword(const common::String& password);
    void Trace(unsigned long long value) const;

private:
    common::FixedString<32> m_communityUserPassword;
};

}
}
}

#endif
