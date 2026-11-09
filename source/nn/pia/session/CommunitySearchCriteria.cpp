#include "nn/pia/session/CommunitySearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace session
{

CommunitySearchCriteria::~CommunitySearchCriteria()
{
}

Result CommunitySearchCriteria::SetResultRange(unsigned int offset, unsigned int size)
{
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

CommunitySearchCriteria::SearchCriteriaKey CommunitySearchCriteria::GetKeyType() const
{
    return m_keyType;
}

unsigned int CommunitySearchCriteria::GetResultRangeOffset() const
{
    return m_resultRangeOffset;
}

unsigned int CommunitySearchCriteria::GetResultRangeSize() const
{
    return m_resultRangeSize;
}

void CommunitySearchCriteria::Reset()
{
    m_keyType = cKeyTypeDefault;
    m_resultRangeOffset = 0;
    m_resultRangeSize = 20;
}

}
}
}
