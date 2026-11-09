#include "nn/pia/session/CreateCommunitySetting.hpp"

namespace nn
{
namespace pia
{
namespace session
{

CreateCommunitySetting::CreateCommunitySetting() : m_minParticipantNum(0), m_maxParticipantNum(0)
{
}

CreateCommunitySetting::~CreateCommunitySetting()
{
}

unsigned short CreateCommunitySetting::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}

unsigned short CreateCommunitySetting::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}

void CreateCommunitySetting::Reset()
{
    m_minParticipantNum = 0;
    m_maxParticipantNum = 0;
}

}
}
}
