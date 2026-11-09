#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_SESSION_SETTING_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/FixedString.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/inet/ParamVariant.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexCreateSessionSetting : public session::CreateSessionSetting
{
public:
    enum
    {
        cAttributeNum = 6,
        cApplicationDataSizeMax = 512,
        cAdditionalScoreBasedParameterValueNum = 3,
        cAdditionalScoreBasedParameterStringNum = 1
    };

    NexCreateSessionSetting();
    NexCreateSessionSetting(const NexCreateSessionSetting& other);
    virtual ~NexCreateSessionSetting();
    NexCreateSessionSetting& operator=(const NexCreateSessionSetting& other);
    void Copy(const NexCreateSessionSetting& other);
    virtual void Reset();
    void SetGameMode(unsigned int value);
    unsigned int GetGameMode() const;
    bool IsSetGameMode() const;
    void SetSessionType(SessionType value);
    SessionType GetSessionType() const;
    bool IsSetSessionType() const;
    bool IsAutoCloseSession() const;
    bool IsSetSessionMatchmakeKeyword() const;
    Result SetAttribute(unsigned int index, unsigned int value);
    unsigned int GetAttribute(unsigned int index) const;
    bool IsSetAttribute(unsigned int index) const;
    virtual Result SetApplicationData(const void* data, unsigned int size);
    virtual Result GetApplicationData(void* data, unsigned int size) const;
    virtual unsigned int GetApplicationDataSize() const;
    virtual bool IsSetApplicationData() const;
    void SetOpenSession(bool value);
    bool IsOpenSession() const;
    bool IsSetOpenSession() const;
    const common::String& GetSessionUserPassword() const;
    bool IsSetSessionUserPassword() const;
    void SetMatchmakeSessionOption(unsigned int value);
    unsigned int GetMatchmakeSessionOption() const;
    bool IsSetMatchmakeSessionOption() const;
    unsigned char GetSelectionPriority() const;
    bool IsSetSelectionPriority() const;
    void SetRatingValue(unsigned int value);
    unsigned int GetRatingValue() const;
    bool IsSetRatingValue() const;
    void SetDisconnectionRate(unsigned int value);
    unsigned int GetDisconnectionRate() const;
    bool IsSetDisconnectionRate() const;
    void SetViolationRate(unsigned int value);
    unsigned int GetViolationRate() const;
    bool IsSetViolationRate() const;
    void SetUseGeoIp(bool value);
    bool GetUseGeoIp() const;
    bool IsSetUseGeoIp() const;
    void SetCountryCode(unsigned int value);
    unsigned int GetCountryCode() const;
    bool IsSetCountryCode() const;
    const common::InetAddress& GetOverrideIpAddress() const;
    bool IsSetOverrideIpAddress() const;
    bool IsSetScoreBasedParameter() const;
    const common::String* GetAdditionalScoreBasedParameterValueKey(unsigned int index) const;
    const ParamVariant* GetAdditionalScoreBasedParameterValue(unsigned int index) const;
    const common::String* GetAdditionalScoreBasedParameterStringKey(unsigned int index) const;
    const common::String* GetAdditionalScoreBasedParameterString(unsigned int index) const;
    void CopyApplicationDataToVector(void* vector) const;
    virtual void Trace(unsigned long long value) const;
    unsigned short GetExtraParticipantNum() const;
    void SetCommunityId(unsigned int value);
    unsigned int GetCommunityId() const;
    bool IsSetCommunityId() const;

private:
    unsigned int m_gameMode;
    SessionType m_sessionType;
    bool m_autoCloseSession;
    unsigned int m_attributes[cAttributeNum];
    unsigned char m_applicationData[cApplicationDataSizeMax];
    unsigned int m_applicationDataSize;
    bool m_openSession;
    unsigned char m_selectionPriority;
    unsigned int m_ratingValue;
    unsigned int m_violationRate;
    unsigned int m_disconnectionRate;
    bool m_useGeoIp;
    unsigned int m_countryCode;
    common::InetAddress m_overrideIpAddress;
    unsigned short m_extraParticipantNum;
    unsigned int m_communityId;
    common::FixedString<32> m_sessionUserPassword;
    unsigned int m_matchmakeSessionOption;
    common::FixedString<8> m_additionalScoreBasedParameterValueKey[cAdditionalScoreBasedParameterValueNum];
    ParamVariant m_additionalScoreBasedParameterValue[cAdditionalScoreBasedParameterValueNum];
    common::FixedString<8> m_additionalScoreBasedParameterStringKey[cAdditionalScoreBasedParameterStringNum];
    common::FixedString<64> m_additionalScoreBasedParameterString[cAdditionalScoreBasedParameterStringNum];
    common::FixedString<32> m_sessionMatchmakeKeyword;
    unsigned int m_stateFlags;
};

}
}
}

#endif
