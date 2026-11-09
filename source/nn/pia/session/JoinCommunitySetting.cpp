#include "nn/pia/session/JoinCommunitySetting.hpp"

namespace nn
{
namespace pia
{
namespace session
{

JoinCommunitySetting::~JoinCommunitySetting()
{
}

unsigned int JoinCommunitySetting::GetCommunityId() const
{
    return m_communityId;
}

void JoinCommunitySetting::Reset()
{
    m_communityId = 0;
}

}
}
}
