#include "nn/pia/inet/NexCommunitySearchCriteriaOwner.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexCommunitySearchCriteriaOwner::~NexCommunitySearchCriteriaOwner()
{
}

unsigned long long NexCommunitySearchCriteriaOwner::GetOwnerPrincipalId() const
{
    return m_ownerPrincipalId;
}

Result NexCommunitySearchCriteriaOwner::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > 100)
        return transport::cResultInvalidArgument;
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

void NexCommunitySearchCriteriaOwner::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_resultRangeOffset = 0xffffffff;
    m_resultRangeSize = 100;
    m_keyType = cKeyTypeOwner;
    m_ownerPrincipalId = 0;
}

}
}
}
