#include "nn/nex/MatchmakeSession.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ATTRIBUTE_COUNT = 6;
const unsigned char MAX_PROGRESS_SCORE = 100;
const unsigned int PARTICIPATION_POLICY_95 = 95;
const unsigned int PARTICIPATION_POLICY_96 = 96;
const unsigned int PARTICIPATION_POLICY_98 = 98;

}

MatchmakeSession::MatchmakeSession()
{
    Reset();
}

Gathering* _DDL_MatchmakeSession::Clone() const
{
    return new MatchmakeSession;
}

void MatchmakeSession::SetMatchmakeSystemType(MatchmakeSystemType type, unsigned int uiPolicyArgument)
{
    m_uiMatchmakeSystemType = type;
    switch (type)
    {
    case cMatchmakeSystemType_2:
    case cMatchmakeSystemType_3:
        SetParticipationPolicy(PARTICIPATION_POLICY_98, uiPolicyArgument);
        break;
    case cMatchmakeSystemType_1:
        SetParticipationPolicy(PARTICIPATION_POLICY_95, uiPolicyArgument);
        break;
    default:
        SetParticipationPolicy(PARTICIPATION_POLICY_96, 0);
        break;
    }
}

void MatchmakeSession::Reset()
{
    Gathering::Reset();
    m_uiGameMode = 0;
    m_uiMatchmakeSystemType = 0;
    m_Attributes.assign(ATTRIBUTE_COUNT, 0);
    m_bOpenParticipation = true;
    m_ApplicationBuffer.clear();
    m_uiProgressScore = MAX_PROGRESS_SCORE;
    m_uiParticipationCount = 0;
    m_uiOption = 0;
    m_StartedTime = DateTime::s_never;
    m_MatchmakeParam = MatchmakeParam();
    SetMatchmakeSystemType(cMatchmakeSystemType_0, 0);
    m_uiReferGatheringID = 0;
}

void MatchmakeSession::SetGameMode(unsigned int uiGameMode)
{
    m_uiGameMode = uiGameMode;
}

void MatchmakeSession::SetAttribute(unsigned int uiIndex, unsigned int uiValue)
{
    m_Attributes[uiIndex] = uiValue;
}

void MatchmakeSession::SetApplicationBuffer(const qVector<unsigned char>& rApplicationBuffer)
{
    m_ApplicationBuffer = rApplicationBuffer;
}

void MatchmakeSession::SetOpenParticipation(bool bOpenParticipation)
{
    m_bOpenParticipation = bOpenParticipation;
}

unsigned int MatchmakeSession::GetAttribute(unsigned int uiIndex) const
{
    return m_Attributes[uiIndex];
}

bool MatchmakeSession::SetProgressScore(unsigned char uiProgressScore)
{
    if (uiProgressScore > MAX_PROGRESS_SCORE)
        return false;
    m_uiProgressScore = uiProgressScore;
    return true;
}

}
}
