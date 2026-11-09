#include "nn/pia/inet/NexSessionSearchCriteriaOwner.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cResultRangeOffsetUnset = 0xffffffff,
    cResultRangeSizeMax = 100
};
}

NexSessionSearchCriteriaOwner::~NexSessionSearchCriteriaOwner()
{
}

nn::pia::Result NexSessionSearchCriteriaOwner::SetResultRange(uint32_t offset, uint32_t size)
{
    if (size > cResultRangeSizeMax)
    {
        return transport::cResultInvalidArgument;
    }
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

uint64_t NexSessionSearchCriteriaOwner::GetOwnerPrincipalId() const
{
    return owner_principal_id_;
}

void NexSessionSearchCriteriaOwner::Reset()
{
    nn::pia::session::SessionSearchCriteria::Reset();
    m_resultRangeOffset = cResultRangeOffsetUnset;
    m_resultRangeSize = cResultRangeSizeMax;
    m_keyType = nn::pia::session::SessionSearchCriteria::cKeyTypeOwner;
    owner_principal_id_ = 0;
}

}
}
}
