#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace session
{

SessionSearchCriteria::SessionSearchCriteria()
    : m_resultRangeOffset(0),
      m_resultRangeSize(20),
      m_keyType(cKeyTypeDefault)
{
}

SessionSearchCriteria::~SessionSearchCriteria()
{
}

nn::pia::Result SessionSearchCriteria::SetResultRange(unsigned int offset, unsigned int size)
{
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

void SessionSearchCriteria::Reset()
{
    m_keyType = cKeyTypeDefault;
    m_resultRangeSize = 20;
    m_resultRangeOffset = 0;
}

void SessionSearchCriteria::Copy(const SessionSearchCriteria& other)
{
    m_keyType = other.m_keyType;
    m_resultRangeOffset = other.m_resultRangeOffset;
    m_resultRangeSize = other.m_resultRangeSize;
}

void SessionSearchCriteria::Trace(unsigned long long) const
{
}

unsigned int SessionSearchCriteria::GetResultRangeOffset() const
{
    return m_resultRangeOffset;
}

unsigned int SessionSearchCriteria::GetResultRangeSize() const
{
    return m_resultRangeSize;
}

SessionSearchCriteria::SearchCriteriaKey SessionSearchCriteria::GetKeyType() const
{
    return m_keyType;
}

void SessionSearchCriteria::SetKeyType(SearchCriteriaKey keyType)
{
    m_keyType = keyType;
}

}
}
}
