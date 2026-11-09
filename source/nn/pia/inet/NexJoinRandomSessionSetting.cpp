#include "nn/pia/inet/NexJoinRandomSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
const int kInvalidParameterResult = 0x10c07;
}

NexJoinRandomSessionSetting::NexJoinRandomSessionSetting()
    : m_sessionSearchCriteriaSize(0),
      m_myBlockListUsed(true),
      m_othersBlockListUsed(true),
      m_matchmakeUniqueKeyword(false)
{
    m_createSessionSetting.Reset();
    for (unsigned int i = 0; i < 2; ++i)
        m_sessionSearchCriteria[i].Reset();
}

NexJoinRandomSessionSetting::~NexJoinRandomSessionSetting()
{
}

session::CreateSessionSetting* NexJoinRandomSessionSetting::GetCreateSessionSetting()
{
    return &m_createSessionSetting;
}
const session::CreateSessionSetting* NexJoinRandomSessionSetting::GetCreateSessionSetting() const
{
    return &m_createSessionSetting;
}
session::SessionSearchCriteria* NexJoinRandomSessionSetting::GetSessionSearchCriteria(unsigned char index)
{
    if (index < 2)
        return &m_sessionSearchCriteria[index];
    return 0;
}
const session::SessionSearchCriteria* NexJoinRandomSessionSetting::GetSessionSearchCriteria(unsigned char index) const
{
    if (index < 2)
        return &m_sessionSearchCriteria[index];
    return 0;
}
unsigned int NexJoinRandomSessionSetting::GetSessionSearchCriteriaSize() const
{
    return m_sessionSearchCriteriaSize;
}
void NexJoinRandomSessionSetting::Reset()
{
    m_createSessionSetting.Reset();
    for (unsigned int i = 0; i < 2; ++i)
        m_sessionSearchCriteria[i].Reset();
    m_sessionSearchCriteriaSize = 0;
    m_myBlockListUsed = true;
    m_othersBlockListUsed = true;
    m_matchmakeUniqueKeyword = false;
}
Result NexJoinRandomSessionSetting::SetCreateSessionSetting(const session::CreateSessionSetting* setting)
{
    if (setting == 0)
        return kInvalidParameterResult;
    m_createSessionSetting.Copy(static_cast<const NexCreateSessionSetting&>(*setting));
    return 0;
}
Result NexJoinRandomSessionSetting::SetSessionSearchCriteria(const session::SessionSearchCriteria* criteria,
                                                             unsigned int count)
{
    if (count > 2)
        return kInvalidParameterResult;
    if (criteria == 0)
        return kInvalidParameterResult;
    const NexSessionSearchCriteria* nexCriteria = static_cast<const NexSessionSearchCriteria*>(criteria);
    for (unsigned int i = 0; i < count; ++i)
    {
        if (nexCriteria[i].GetKeyType() != 0)
            return kInvalidParameterResult;
    }
    for (unsigned int i = 0; i < count; ++i)
    {
        m_sessionSearchCriteria[i].Copy(nexCriteria[i]);
    }
    m_sessionSearchCriteriaSize = count;
    return 0;
}
void NexJoinRandomSessionSetting::Trace(unsigned long long p0) const
{
    (void)p0;
}

}
}
}
