#include "nn/nex/JoinMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

void JoinMatchmakeSessionParam::Reset()
{
    m_uiGatheringID = 0;
    m_AdditionalParticipants = qList<unsigned int>();
    m_uiJoinMatchmakeSessionOption = 0;
    m_uiGatheringIDForParticipationCheck = 0;
    m_uiJoinMatchmakeSessionBehavior = 0;
    m_strUserPassword = String(L"");
    m_strSystemPassword = String(L"");
    m_strJoinMessage = String();
    m_uiExtraParticipants = 0;
    m_uiParticipationCount = DEFAULT_PARTICIPATION_COUNT;
}

}
}
