#include "nn/pia/inet/NexJoinCommunitySetting.hpp"
namespace nn
{
namespace pia
{
namespace inet
{

const common::String& NexJoinCommunitySetting::GetCommunityUserPassword() const
{
    return m_communityUserPassword;
}

void NexJoinCommunitySetting::Reset()
{
    session::JoinCommunitySetting::Reset();
    m_communityUserPassword.Clear();
}

}
}
}
