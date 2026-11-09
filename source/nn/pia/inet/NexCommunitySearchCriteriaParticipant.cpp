#include "nn/pia/inet/NexCommunitySearchCriteriaParticipant.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexCommunitySearchCriteriaParticipant::~NexCommunitySearchCriteriaParticipant()
{
}

unsigned long long NexCommunitySearchCriteriaParticipant::GetParticipantPrincipalId() const
{
    return m_participantPrincipalId;
}

Result NexCommunitySearchCriteriaParticipant::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > 100)
        return transport::cResultInvalidArgument;
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

void NexCommunitySearchCriteriaParticipant::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_resultRangeOffset = 0xffffffff;
    m_resultRangeSize = 100;
    m_keyType = cKeyTypeParticipant;
    m_participantPrincipalId = 0;
}

}
}
}
