#include "nn/pia/session/JoinSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace session
{

JoinSessionSetting::JoinSessionSetting() : m_sessionInfo(0), m_sessionId(0), m_maxParticipantNum(0)
{
}

JoinSessionSetting::~JoinSessionSetting()
{
}

void JoinSessionSetting::Reset()
{
    m_sessionId = 0;
    m_maxParticipantNum = 0;
    m_sessionInfo = 0;
}

const ISessionInfo* JoinSessionSetting::GetSessionInfoPtr() const
{
    return m_sessionInfo;
}

void JoinSessionSetting::SetSessionInfoPtr(const ISessionInfo* sessionInfo)
{
    m_sessionInfo = sessionInfo;
}

void JoinSessionSetting::SetSessionId(unsigned int sessionId)
{
    m_sessionId = sessionId;
}

unsigned int JoinSessionSetting::GetSessionId() const
{
    if (m_sessionId != 0)
        return m_sessionId;
    if (m_sessionInfo != 0)
        return m_sessionInfo->GetSessionId();
    return 0;
}

void JoinSessionSetting::SetMaxParticipantNum(unsigned short participantNum)
{
    m_maxParticipantNum = participantNum;
}

unsigned int JoinSessionSetting::GetMaxParticipantNum() const
{
    if (m_sessionInfo != 0)
        return m_sessionInfo->GetMaxParticipantNum();
    return m_maxParticipantNum;
}

}
}
}
