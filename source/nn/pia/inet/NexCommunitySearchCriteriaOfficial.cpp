#include "nn/pia/inet/NexCommunitySearchCriteriaOfficial.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexCommunitySearchCriteriaOfficial::~NexCommunitySearchCriteriaOfficial()
{
}

Result NexCommunitySearchCriteriaOfficial::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > 100)
        return transport::cResultInvalidArgument;
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

bool NexCommunitySearchCriteriaOfficial::IsAvailableCommunityOnly() const
{
    return m_availableCommunityOnly;
}

void NexCommunitySearchCriteriaOfficial::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_resultRangeOffset = 0xffffffff;
    m_resultRangeSize = 100;
    m_keyType = cKeyTypeOfficial;
    m_availableCommunityOnly = true;
}

}
}
}
