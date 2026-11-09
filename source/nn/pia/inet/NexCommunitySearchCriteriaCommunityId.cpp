#include "nn/pia/inet/NexCommunitySearchCriteriaCommunityId.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexCommunitySearchCriteriaCommunityId::~NexCommunitySearchCriteriaCommunityId()
{
}

Result NexCommunitySearchCriteriaCommunityId::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > 100)
        return transport::cResultInvalidArgument;
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

const unsigned int* NexCommunitySearchCriteriaCommunityId::GetCommunityIdList() const
{
    return m_communityIds;
}

unsigned int NexCommunitySearchCriteriaCommunityId::GetCommunityIdListSize() const
{
    return m_communityIdCount;
}

void NexCommunitySearchCriteriaCommunityId::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_resultRangeOffset = 0xffffffff;
    m_resultRangeSize = 100;
    m_keyType = cKeyTypeCommunityId;
    for (unsigned int index = 0; index < 100; ++index)
        m_communityIds[index] = 0;
    m_communityIdCount = 0;
}

}
}
}
