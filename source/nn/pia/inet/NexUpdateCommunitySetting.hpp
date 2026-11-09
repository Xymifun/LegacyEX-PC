#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_UPDATE_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_UPDATE_COMMUNITY_SETTING_HPP

#include "nn/pia/String.hpp"
#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/session/UpdateCommunitySetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexUpdateCommunitySetting : public session::UpdateCommunitySetting
{
public:
    NexUpdateCommunitySetting();
    virtual ~NexUpdateCommunitySetting();
    void Reset();
    void Trace(unsigned long long value) const;
    unsigned int GetCommunityId() const;
    unsigned short GetMinParticipantNum() const;
    unsigned short GetMaxParticipantNum() const;
    unsigned int GetAttribute(unsigned int index) const;
    const common::String& GetDescription() const;
    const common::String& GetCommunityUserPassword() const;
    bool IsSetCommunityUserPassword() const;
    const common::DateTime& GetParticipaitonStartDate() const;
    const common::DateTime& GetParticipaitonEndDate() const;
    void CopyApplicationDataToVector(void* output) const;
    bool IsValid() const;
    void SetMinParticipantNum(unsigned short value);
    void SetMaxParticipantNum(unsigned short value);
    void SetAttributes(const unsigned int* values);
    void SetCommunityUserPassword(const common::String& value);
    void SetApplicationData(const void* data, unsigned int size);
    void SetParticipationStartDate(const common::DateTime& value);
    void SetParticipationEndDate(const common::DateTime& value);
    void SetDescription(const common::String& value);
    void SetCommunityId(unsigned int value);
    void Copy(const NexUpdateCommunitySetting& other);
    NexUpdateCommunitySetting& operator=(const NexUpdateCommunitySetting& other);
    bool IsSetCommunityId() const;
    bool IsSetMinParticipantNum() const;
    bool IsSetMaxParticipantNum() const;
    void GetApplicationData(void* output, unsigned int size) const;
    bool IsSetApplicationData() const;
    unsigned int GetApplicationDataSize() const;
    bool IsSetAttribute() const;
    bool IsSetDescription() const;
    bool IsSetParticipationStartDate() const;
    bool IsSetParticipationEndDate() const;

private:
    unsigned int communityId;
    unsigned short minParticipantNum;
    unsigned short maxParticipantNum;
    unsigned int attributes[6];
    unsigned char applicationData[512];
    unsigned int applicationDataSize;
    common::FixedString<256> description;
    common::FixedString<32> communityUserPassword;
    common::DateTime participationStartDate;
    common::DateTime participationEndDate;
    unsigned int communityIdSet : 1;
    unsigned int minParticipantNumSet : 1;
    unsigned int maxParticipantNumSet : 1;
    unsigned int applicationDataSet : 1;
    unsigned int attributesSet : 1;
    unsigned int descriptionSet : 1;
    unsigned int : 18;
    unsigned int communityUserPasswordSet : 1;
    unsigned int participationStartDateSet : 1;
    unsigned int participationEndDateSet : 1;
};

}
}
}

#endif
