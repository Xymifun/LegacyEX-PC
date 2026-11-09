#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_UPDATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_UPDATE_SESSION_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/String.hpp"
#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/inet/ParamVariant.hpp"
#include "nn/pia/session/UpdateSessionSetting.hpp"
#include "nn/pia/inet/SessionType.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexUpdateSessionSetting : public session::UpdateSessionSetting
{
public:
    enum
    {
        cAttributeNum = 6
    };

    NexUpdateSessionSetting();
    virtual ~NexUpdateSessionSetting();

    virtual void Reset();
    virtual void Trace(unsigned long long value) const;

    void AddNexModificationFlag(unsigned int flag);
    bool IsNexModificationFlagSet(unsigned int flag) const;
    unsigned int GetNexModificationFlag() const;

    void SetMinParticipantNum(unsigned short participantNum);
    void SetMaxParticipantNum(unsigned short participantNum);
    unsigned short GetMinParticipantNum() const;
    unsigned short GetMaxParticipantNum() const;
    bool IsSetMinParticipantNum() const;
    bool IsSetMaxParticipantNum() const;

    void SetSessionType(SessionType sessionType, bool autoCloseSession);
    SessionType GetSessionType() const;
    bool IsAutoCloseSession() const;
    bool IsSetSessionType() const;

    Result SetAttributes(const unsigned int* attributes);
    Result SetAttribute(unsigned int index, unsigned int value);
    unsigned int GetAttribute(unsigned int index) const;
    bool IsAttributeUpdated(unsigned int index) const;
    bool IsOnlyAttributeUpdated() const;
    bool IsOnlyAllAttributesUpdated() const;
    bool IsOnlyApplicationDataUpdated() const;

    Result SetApplicationData(const void* data, unsigned int size);
    Result GetApplicationData(void* buffer, unsigned int bufferSize) const;
    unsigned int GetApplicationDataSize() const;
    void CopyApplicationDataToVector(void* vector) const;

    void SetSessionUserPassword(const common::String& password);
    const common::String& GetSessionUserPassword() const;
    bool IsSetSessionUserPassword() const;

    void SetSessionMatchmakeKeyword(const common::String& keyword);
    const common::String& GetSessionMatchmakeKeyword() const;
    const common::DateTime& GetStartedTime() const;

    void SetRatingValue(unsigned int ratingValue);
    unsigned int GetRatingValue() const;
    bool IsSetRatingValue() const;

    void SetDisconnectionRate(unsigned int disconnectionRate);
    unsigned int GetDisconnectionRate() const;
    bool IsSetDisconnectionRate() const;

    void SetViolationRate(unsigned int violationRate);
    unsigned int GetViolationRate() const;
    bool IsSetViolationRate() const;

    void SetCountryCode(const common::String& countryCode);
    void SetCountryCode(unsigned int countryCode);
    unsigned int GetCountryCode() const;
    bool IsSetCountryCode() const;

    void SetUpdateGeoIp(bool updateGeoIp);
    bool GetUpdateGeoIp() const;
    bool IsSetUpdateGeoIp() const;

    void SetSelectionPriority(unsigned char selectionPriority);
    unsigned char GetSelectionPriority() const;

    bool IsSetScoreBaseParam() const;
    bool IsValid() const;

    const common::String* GetAdditionalScoreBasedParameterValueKey(unsigned int index) const;
    const ParamVariant* GetAdditionalScoreBasedParameterValue(unsigned int index) const;
    const common::String* GetAdditionalScoreBasedParameterStringKey(unsigned int index) const;
    const common::String* GetAdditionalScoreBasedParameterString(unsigned int index) const;

    unsigned short m_minParticipantNum;
    unsigned short m_maxParticipantNum;
    SessionType m_sessionType;
    bool m_autoCloseSession;
    unsigned int m_attribute[cAttributeNum];
    bool m_attributeUpdated[cAttributeNum];
    unsigned char m_applicationData[512];
    unsigned int m_applicationDataSize;
    unsigned char m_selectionPriority;
    common::FixedString<32> m_sessionUserPassword;
    common::FixedString<32> m_sessionMatchmakeKeyword;
    common::DateTime m_startedTime;
    unsigned int m_ratingValue;
    bool m_isSetRatingValue;
    unsigned int m_disconnectionRate;
    bool m_isSetDisconnectionRate;
    unsigned int m_violationRate;
    bool m_isSetViolationRate;
    unsigned int m_countryCode;
    bool m_isSetCountryCode;
    bool m_updateGeoIp;
    bool m_isSetUpdateGeoIp;
    common::FixedString<8> m_additionalScoreBasedParameterValueKey[3];
    ParamVariant m_additionalScoreBasedParameterValue[3];
    common::FixedString<8> m_additionalScoreBasedParameterStringKey;
    common::FixedString<64> m_additionalScoreBasedParameterString;
    unsigned int m_nexModificationFlag;
};

}
}
}

#endif
