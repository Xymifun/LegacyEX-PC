#include "nn/pia/inet/NexCommunityInfo.hpp"
#include <coreinit/memory.h>

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
    cResultInvalidArgument = 0x10c07
};
}

NexCommunityInfo::NexCommunityInfo()
{
    Clear();
}

NexCommunityInfo::~NexCommunityInfo()
{
}

void NexCommunityInfo::Clear()
{
    m_communityId = 0;
    m_official = false;
    m_restricted = false;
    m_relatedSessionCount = 0;
    m_currentParticipantNum = 0;
    m_ownerPrincipalId = 0;
    m_maxParticipantNum = 0;
    m_minParticipantNum = 0;
    m_applicationDataSize = 0;
    m_participationStartDate.Clear();
    m_participationEndDate.Clear();
    m_description.Clear();
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
}

unsigned int NexCommunityInfo::GetCommunityId() const
{
    return m_communityId;
}
bool NexCommunityInfo::IsOfficialCommunity() const
{
    return m_official;
}
bool NexCommunityInfo::IsRestrictedByPassword() const
{
    return m_restricted;
}
const common::DateTime& NexCommunityInfo::GetParticipationStartDate()
{
    return m_participationStartDate;
}
const common::DateTime& NexCommunityInfo::GetParticipationEndDate()
{
    return m_participationEndDate;
}
unsigned int NexCommunityInfo::GetRelatedSessionCount() const
{
    return m_relatedSessionCount;
}

Result NexCommunityInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (value == 0)
    {
        return Result(cResultInvalidArgument);
    }
    else if (index >= 6)
    {
        Result result(cResultInvalidArgument);
        return result;
    }
    else
    {
        *value = m_attributes[index];
        return Result(0);
    }
}

unsigned long long NexCommunityInfo::GetOwnerPrincipalId() const
{
    return m_ownerPrincipalId;
}
unsigned short NexCommunityInfo::GetCurrentParticipantNum() const
{
    return m_currentParticipantNum;
}
unsigned short NexCommunityInfo::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}
unsigned short NexCommunityInfo::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}
const common::String& NexCommunityInfo::GetDescription() const
{
    return m_description;
}

Result NexCommunityInfo::GetApplicationData(void* buffer, unsigned int size)
{
    if (buffer == 0 || size < m_applicationDataSize)
    {
        return Result(cResultInvalidArgument);
    }
    OSBlockMove(buffer, m_applicationData, size, false);
    return Result(0);
}

Result NexCommunityInfo::GetApplicationDataSize(unsigned int* size)
{
    if (size == 0)
    {
        return Result(cResultInvalidArgument);
    }
    *size = m_applicationDataSize;
    return Result(0);
}

void NexCommunityInfo::SetCommunityId(unsigned int communityId)
{
    m_communityId = communityId;
}
void NexCommunityInfo::SetOfficialCommunity(bool official)
{
    m_official = official;
}
void NexCommunityInfo::SetRestrictedByPassword(bool restricted)
{
    m_restricted = restricted;
}
void NexCommunityInfo::SetParticipationStartTime(const common::DateTime& time)
{
    m_participationStartDate = time;
}
void NexCommunityInfo::SetParticipationEndTime(const common::DateTime& time)
{
    m_participationEndDate = time;
}
void NexCommunityInfo::SetRelatedSessionCount(unsigned int count)
{
    m_relatedSessionCount = count;
}

void NexCommunityInfo::SetAttribute(unsigned char index, unsigned int value)
{
    m_attributes[index] = value;
}

void NexCommunityInfo::SetOwnerPrincipalId(unsigned long long principalId)
{
    m_ownerPrincipalId = principalId;
}
void NexCommunityInfo::SetCurrentParticipantNum(unsigned short count)
{
    m_currentParticipantNum = count;
}
void NexCommunityInfo::SetMinParticipantNum(unsigned short count)
{
    m_minParticipantNum = count;
}
void NexCommunityInfo::SetMaxParticipantNum(unsigned short count)
{
    m_maxParticipantNum = count;
}
void NexCommunityInfo::SetDescription(const common::String& description)
{
    m_description.Copy(description);
}

void NexCommunityInfo::SetApplicationData(const void* data, unsigned int size)
{
    OSBlockMove(m_applicationData, data, size, false);
    m_applicationDataSize = size;
}

}
}
}
