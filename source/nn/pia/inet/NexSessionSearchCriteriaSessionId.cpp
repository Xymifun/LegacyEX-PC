#include "nn/pia/inet/NexSessionSearchCriteriaSessionId.hpp"

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

NexSessionSearchCriteriaSessionId::~NexSessionSearchCriteriaSessionId()
{
}

unsigned int NexSessionSearchCriteriaSessionId::GetSessionId() const
{
    return m_sessionId;
}

void NexSessionSearchCriteriaSessionId::Reset()
{
    session::SessionSearchCriteria::Reset();
    m_resultRangeOffset = cResultRangeOffsetUnset;
    m_resultRangeSize = cResultRangeSizeMax;
    m_keyType = session::SessionSearchCriteria::cKeyTypeSessionId;
    m_sessionId = 0;
}

nn::pia::Result NexSessionSearchCriteriaSessionId::SetResultRange(unsigned int, unsigned int)
{
    return transport::cResultInvalidState;
}

}
}
}
