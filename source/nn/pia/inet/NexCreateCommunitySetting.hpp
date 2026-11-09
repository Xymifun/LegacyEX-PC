#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_COMMUNITY_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/session/CreateCommunitySetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCreateCommunitySetting : public session::CreateCommunitySetting
{
public:
    enum
    {
        cAttributeNum = 6,
        cApplicationDataSizeMax = 512
    };

    NexCreateCommunitySetting();
    NexCreateCommunitySetting(const NexCreateCommunitySetting& other);
    NexCreateCommunitySetting& operator=(const NexCreateCommunitySetting& other);
    void Copy(const NexCreateCommunitySetting& other);
    const common::DateTime* GetParticipaitonStartDate() const;
    const common::DateTime* GetParticipaitonEndDate() const;
    const common::String* GetDescription() const;
    unsigned int GetAttribute(unsigned int index) const;
    bool IsSetAttribute(unsigned int index) const;
    void CopyApplicationDataToVector(void* vector) const;
    const common::String* GetCommunityUserPassword() const;
    void SetParticipationStartDate(const common::DateTime& value);
    bool IsSetParticipationStartDate() const;
    void SetParticipationEndDate(const common::DateTime& value);
    bool IsSetParticipationEndDate() const;
    void SetDescription(const common::String& value);
    bool IsSetDescription() const;
    void SetAttribute(unsigned int index, unsigned int value);
    bool SetApplicationData(const void* data, unsigned int size);
    bool GetApplicationData(void* data, unsigned int size) const;
    unsigned int GetApplicationDataSize() const;
    bool IsSetApplicationData() const;
    void SetCommunityUserPassword(const common::String& value);
    bool IsSetCommunityUserPassword() const;
    void Reset();

private:
    common::DateTime m_participationStartDate;
    common::DateTime m_participationEndDate;
    common::FixedString<256> m_description;
    unsigned int m_attributes[cAttributeNum];
    unsigned char m_applicationData[cApplicationDataSizeMax];
    unsigned int m_applicationDataSize;
    common::FixedString<32> m_communityUserPassword;
    unsigned int m_stateFlags;
};

}
}
}

#endif
