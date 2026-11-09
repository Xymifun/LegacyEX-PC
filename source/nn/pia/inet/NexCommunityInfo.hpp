#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_HPP

#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/common/FixedString.hpp"
#include "nn/pia/session/ICommunityInfo.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunityInfo : public session::ICommunityInfo
{
public:
    NexCommunityInfo();
    virtual ~NexCommunityInfo();
    virtual unsigned int GetCommunityId() const;
    virtual bool IsOfficialCommunity() const;
    virtual bool IsRestrictedByPassword() const;
    virtual const common::DateTime& GetParticipationStartDate();
    virtual const common::DateTime& GetParticipationEndDate();
    virtual unsigned int GetRelatedSessionCount() const;
    virtual Result GetAttribute(unsigned char index, unsigned int* value) const;
    virtual unsigned long long GetOwnerPrincipalId() const;
    virtual unsigned short GetCurrentParticipantNum() const;
    virtual unsigned short GetMaxParticipantNum() const;
    virtual unsigned short GetMinParticipantNum() const;
    virtual const common::String& GetDescription() const;
    virtual Result GetApplicationData(void* buffer, unsigned int size);
    virtual Result GetApplicationDataSize(unsigned int* size);
    virtual void Clear();
    virtual void SetCommunityId(unsigned int communityId);
    virtual void SetOfficialCommunity(bool official);
    virtual void SetRestrictedByPassword(bool restricted);
    virtual void SetParticipationStartTime(const common::DateTime& time);
    virtual void SetParticipationEndTime(const common::DateTime& time);
    virtual void SetRelatedSessionCount(unsigned int count);
    virtual void SetAttribute(unsigned char index, unsigned int value);
    virtual void SetOwnerPrincipalId(unsigned long long principalId);
    virtual void SetCurrentParticipantNum(unsigned short count);
    virtual void SetMinParticipantNum(unsigned short count);
    virtual void SetMaxParticipantNum(unsigned short count);
    virtual void SetDescription(const common::String& description);
    virtual void SetApplicationData(const void* data, unsigned int size);

private:
    unsigned int m_communityId;
    bool m_official;
    bool m_restricted;
    unsigned int m_attributes[6];
    unsigned int m_relatedSessionCount;
    unsigned short m_currentParticipantNum;
    unsigned long long m_ownerPrincipalId;
    unsigned short m_maxParticipantNum;
    unsigned short m_minParticipantNum;
    common::FixedString<256> m_description;
    unsigned char m_applicationData[512];
    unsigned int m_applicationDataSize;
    common::DateTime m_participationStartDate;
    common::DateTime m_participationEndDate;
};

}
}
}

#endif
