#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_COMMUNITY_SETTING_HPP

namespace nn
{
namespace pia
{
namespace session
{

class JoinCommunitySetting
{
public:
    JoinCommunitySetting();
    virtual ~JoinCommunitySetting();
    unsigned int GetCommunityId() const;
    void Reset();
    void SetCommunityId(unsigned int communityId);

private:
    unsigned int m_communityId;
};

}
}
}

#endif
