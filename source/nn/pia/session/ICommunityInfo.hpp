#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ICOMMUNITYINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ICOMMUNITYINFO_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class DateTime;
class String;

}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class ICommunityInfo
{
public:
    virtual ~ICommunityInfo();
    virtual unsigned int GetCommunityId() const = 0;
    virtual bool IsOfficialCommunity() const = 0;
    virtual bool IsRestrictedByPassword() const = 0;
    virtual const common::DateTime& GetParticipationStartDate() = 0;
    virtual const common::DateTime& GetParticipationEndDate() = 0;
    virtual unsigned int GetRelatedSessionCount() const = 0;
    virtual Result GetAttribute(unsigned char index, unsigned int* value) const = 0;
    virtual unsigned long long GetOwnerPrincipalId() const = 0;
    virtual unsigned short GetCurrentParticipantNum() const = 0;
    virtual unsigned short GetMaxParticipantNum() const = 0;
    virtual unsigned short GetMinParticipantNum() const = 0;
    virtual const common::String& GetDescription() const = 0;
    virtual Result GetApplicationData(void* buffer, unsigned int size) = 0;
    virtual Result GetApplicationDataSize(unsigned int* size) = 0;
    virtual void Clear() = 0;
};

}
}
}

#endif
