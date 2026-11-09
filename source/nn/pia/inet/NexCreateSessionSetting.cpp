#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/nex/MemAllocator.hpp"
#include <coreinit/memory.h>

#include <vector>

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
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultBufferTooSmall = 0x10c04
};

enum
{
    cFlagGameMode = 0,
    cFlagSessionType = 1,
    cFlagAttributeFirst = 2,
    cFlagApplicationData = 8,
    cFlagOpenSession = 9,
    cFlagSessionUserPassword = 10,
    cFlagMatchmakeSessionOption = 11,
    cFlagSelectionPriority = 12,
    cFlagRatingValue = 15,
    cFlagDisconnectionRate = 16,
    cFlagViolationRate = 17,
    cFlagCountryCode = 18,
    cFlagUseGeoIp = 19,
    cFlagOverrideIpAddress = 20,
    cFlagCommunityId = 21,
    cFlagSessionMatchmakeKeyword = 22,
    cFlagScoreBasedParameterStringKey = 23,
    cFlagScoreBasedParameterValueKeyFirst = 24
};

enum
{
    cScoreBasedParameterBaseMask = (1u << cFlagRatingValue) | (1u << cFlagDisconnectionRate)
                                   | (1u << cFlagViolationRate) | (1u << cFlagCountryCode) | (1u << cFlagUseGeoIp)
};
}

NexCreateSessionSetting::NexCreateSessionSetting()
    : m_gameMode(0),
      m_sessionType(cSessionType_Normal),
      m_autoCloseSession(false),
      m_applicationDataSize(0),
      m_openSession(false),
      m_selectionPriority(0),
      m_ratingValue(0),
      m_violationRate(0),
      m_disconnectionRate(0),
      m_useGeoIp(false),
      m_extraParticipantNum(0),
      m_communityId(0),
      m_matchmakeSessionOption(0),
      m_stateFlags(0)
{
    m_overrideIpAddress.m_address = 0;
    m_overrideIpAddress.m_port = 0;
    m_sessionUserPassword.Clear();
    m_sessionMatchmakeKeyword.Clear();
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    for (unsigned int index = 0; index < cAdditionalScoreBasedParameterValueNum; ++index)
    {
        m_additionalScoreBasedParameterValueKey[index].Clear();
        m_additionalScoreBasedParameterValue[index] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey[0].Clear();
    m_additionalScoreBasedParameterString[0].Clear();
    m_countryCode = 0;
}

NexCreateSessionSetting::NexCreateSessionSetting(const NexCreateSessionSetting& other)
{
    Copy(other);
}

NexCreateSessionSetting::~NexCreateSessionSetting()
{
}

NexCreateSessionSetting& NexCreateSessionSetting::operator=(const NexCreateSessionSetting& other)
{
    Copy(other);
    return *this;
}

void NexCreateSessionSetting::Copy(const NexCreateSessionSetting& other)
{
    CreateSessionSetting::Copy(other);
    m_gameMode = other.m_gameMode;
    m_sessionType = other.m_sessionType;
    m_autoCloseSession = other.m_autoCloseSession;
    OSBlockMove(m_attributes, other.m_attributes, sizeof(m_attributes), FALSE);
    OSBlockMove(m_applicationData, other.m_applicationData, sizeof(m_applicationData), FALSE);
    m_applicationDataSize = other.m_applicationDataSize;
    m_openSession = other.m_openSession;
    m_selectionPriority = other.m_selectionPriority;
    m_ratingValue = other.m_ratingValue;
    m_violationRate = other.m_violationRate;
    m_disconnectionRate = other.m_disconnectionRate;
    m_useGeoIp = other.m_useGeoIp;
    m_overrideIpAddress = other.m_overrideIpAddress;
    m_extraParticipantNum = other.m_extraParticipantNum;
    m_communityId = other.m_communityId;
    m_sessionUserPassword.Copy(other.m_sessionUserPassword);
    m_sessionMatchmakeKeyword.Copy(other.m_sessionMatchmakeKeyword);
    m_matchmakeSessionOption = other.m_matchmakeSessionOption;
    m_stateFlags = other.m_stateFlags;
    for (unsigned int index = 0; index < cAdditionalScoreBasedParameterValueNum; ++index)
    {
        m_additionalScoreBasedParameterValueKey[index].Copy(other.m_additionalScoreBasedParameterValueKey[index]);
        m_additionalScoreBasedParameterValue[index] = other.m_additionalScoreBasedParameterValue[index];
    }
    for (unsigned int index = 0; index < cAdditionalScoreBasedParameterStringNum; ++index)
    {
        m_additionalScoreBasedParameterStringKey[index].Copy(other.m_additionalScoreBasedParameterStringKey[index]);
        m_additionalScoreBasedParameterString[index].Copy(other.m_additionalScoreBasedParameterString[index]);
    }
    m_countryCode = other.m_countryCode;
}

void NexCreateSessionSetting::Reset()
{
    CreateSessionSetting::Reset();
    m_sessionType = cSessionType_Normal;
    m_gameMode = 0;
    m_autoCloseSession = false;
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_violationRate = 0;
    m_selectionPriority = 0;
    m_matchmakeSessionOption = 0;
    m_overrideIpAddress.m_address = 0;
    m_communityId = 0;
    m_overrideIpAddress.m_port = 0;
    m_disconnectionRate = 0;
    m_applicationDataSize = 0;
    m_ratingValue = 0;
    m_stateFlags = 0;
    m_useGeoIp = false;
    m_extraParticipantNum = 0;
    m_openSession = false;
    m_sessionUserPassword.Clear();
    m_sessionMatchmakeKeyword.Clear();
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    for (unsigned int index = 0; index < cAdditionalScoreBasedParameterValueNum; ++index)
    {
        m_additionalScoreBasedParameterValueKey[index].Clear();
        m_additionalScoreBasedParameterValue[index] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey[0].Clear();
    m_additionalScoreBasedParameterString[0].Clear();
    m_countryCode = 0;
}

void NexCreateSessionSetting::SetGameMode(unsigned int value)
{
    m_gameMode = value;
    m_stateFlags |= 1u << cFlagGameMode;
}

unsigned int NexCreateSessionSetting::GetGameMode() const
{
    return m_gameMode;
}

bool NexCreateSessionSetting::IsSetGameMode() const
{
    return (m_stateFlags >> cFlagGameMode & 1) != 0;
}

void NexCreateSessionSetting::SetSessionType(SessionType value)
{
    m_sessionType = value;
    if (value == cSessionType_Normal)
        m_autoCloseSession = false;
    else if (value == cSessionType_Ranked)
        m_autoCloseSession = true;
    m_stateFlags |= 1u << cFlagSessionType;
}

SessionType NexCreateSessionSetting::GetSessionType() const
{
    return m_sessionType;
}

bool NexCreateSessionSetting::IsSetSessionType() const
{
    return (m_stateFlags >> cFlagSessionType & 1) != 0;
}

bool NexCreateSessionSetting::IsAutoCloseSession() const
{
    return m_autoCloseSession;
}

bool NexCreateSessionSetting::IsSetSessionMatchmakeKeyword() const
{
    return (m_stateFlags >> cFlagSessionMatchmakeKeyword & 1) != 0;
}

Result NexCreateSessionSetting::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cAttributeNum)
        return cResultInvalidArgument;
    if (IsSetSessionMatchmakeKeyword())
        return cResultInvalidState;
    m_attributes[index] = value;
    m_stateFlags |= 1u << (index + cFlagAttributeFirst);
    return 0;
}

unsigned int NexCreateSessionSetting::GetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return 0;
    return m_attributes[index];
}

bool NexCreateSessionSetting::IsSetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return false;
    return (m_stateFlags >> (index + cFlagAttributeFirst) & 1) != 0;
}

Result NexCreateSessionSetting::SetApplicationData(const void* data, unsigned int size)
{
    if (data == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (size > cApplicationDataSizeMax)
        return cResultInvalidArgument;
    OSBlockMove(m_applicationData, data, size, false);
    m_applicationDataSize = size;
    m_stateFlags |= 1u << cFlagApplicationData;
    return 0;
}

Result NexCreateSessionSetting::GetApplicationData(void* data, unsigned int size) const
{
    if (data == 0)
        return cResultInvalidArgument;
    if (m_applicationDataSize == 0)
        return cResultInvalidState;
    if (size < m_applicationDataSize)
        return cResultBufferTooSmall;
    OSBlockMove(data, m_applicationData, m_applicationDataSize, false);
    return 0;
}

unsigned int NexCreateSessionSetting::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}

bool NexCreateSessionSetting::IsSetApplicationData() const
{
    return (m_stateFlags >> cFlagApplicationData & 1) != 0;
}

void NexCreateSessionSetting::SetOpenSession(bool value)
{
    m_openSession = value;
    m_stateFlags |= 1u << cFlagOpenSession;
}

bool NexCreateSessionSetting::IsOpenSession() const
{
    return m_openSession;
}

bool NexCreateSessionSetting::IsSetOpenSession() const
{
    return (m_stateFlags >> cFlagOpenSession & 1) != 0;
}

const common::String& NexCreateSessionSetting::GetSessionUserPassword() const
{
    return m_sessionUserPassword;
}

bool NexCreateSessionSetting::IsSetSessionUserPassword() const
{
    return (m_stateFlags >> cFlagSessionUserPassword & 1) != 0;
}

void NexCreateSessionSetting::SetMatchmakeSessionOption(unsigned int value)
{
    m_matchmakeSessionOption = value;
    m_stateFlags |= 1u << cFlagMatchmakeSessionOption;
}

unsigned int NexCreateSessionSetting::GetMatchmakeSessionOption() const
{
    return m_matchmakeSessionOption;
}

bool NexCreateSessionSetting::IsSetMatchmakeSessionOption() const
{
    return (m_stateFlags >> cFlagMatchmakeSessionOption & 1) != 0;
}

unsigned char NexCreateSessionSetting::GetSelectionPriority() const
{
    return m_selectionPriority;
}

bool NexCreateSessionSetting::IsSetSelectionPriority() const
{
    return (m_stateFlags >> cFlagSelectionPriority & 1) != 0;
}

void NexCreateSessionSetting::SetRatingValue(unsigned int value)
{
    m_ratingValue = value;
    m_stateFlags |= 1u << cFlagRatingValue;
}

unsigned int NexCreateSessionSetting::GetRatingValue() const
{
    return m_ratingValue;
}

bool NexCreateSessionSetting::IsSetRatingValue() const
{
    return (m_stateFlags >> cFlagRatingValue & 1) != 0;
}

void NexCreateSessionSetting::SetDisconnectionRate(unsigned int value)
{
    m_disconnectionRate = value;
    m_stateFlags |= 1u << cFlagDisconnectionRate;
}

unsigned int NexCreateSessionSetting::GetDisconnectionRate() const
{
    return m_disconnectionRate;
}

bool NexCreateSessionSetting::IsSetDisconnectionRate() const
{
    return (m_stateFlags >> cFlagDisconnectionRate & 1) != 0;
}

void NexCreateSessionSetting::SetViolationRate(unsigned int value)
{
    m_violationRate = value;
    m_stateFlags |= 1u << cFlagViolationRate;
}

unsigned int NexCreateSessionSetting::GetViolationRate() const
{
    return m_violationRate;
}

bool NexCreateSessionSetting::IsSetViolationRate() const
{
    return (m_stateFlags >> cFlagViolationRate & 1) != 0;
}

void NexCreateSessionSetting::SetUseGeoIp(bool value)
{
    m_useGeoIp = value;
    m_stateFlags |= 1u << cFlagUseGeoIp;
}

bool NexCreateSessionSetting::GetUseGeoIp() const
{
    return m_useGeoIp;
}

bool NexCreateSessionSetting::IsSetUseGeoIp() const
{
    return (m_stateFlags >> cFlagUseGeoIp & 1) != 0;
}

void NexCreateSessionSetting::SetCountryCode(unsigned int value)
{
    m_countryCode = value;
    m_stateFlags |= 1u << cFlagCountryCode;
}

unsigned int NexCreateSessionSetting::GetCountryCode() const
{
    return m_countryCode;
}

bool NexCreateSessionSetting::IsSetCountryCode() const
{
    return (m_stateFlags >> cFlagCountryCode & 1) != 0;
}

const common::InetAddress& NexCreateSessionSetting::GetOverrideIpAddress() const
{
    return m_overrideIpAddress;
}

bool NexCreateSessionSetting::IsSetOverrideIpAddress() const
{
    return (m_stateFlags >> cFlagOverrideIpAddress & 1) != 0;
}

bool NexCreateSessionSetting::IsSetScoreBasedParameter() const
{
    unsigned int mask = cScoreBasedParameterBaseMask;
    for (unsigned int index = 0; index < cAdditionalScoreBasedParameterValueNum; ++index)
    {
        if (m_additionalScoreBasedParameterValueKey[index].GetStringLength() > 0)
            mask |= 1u << (index + cFlagScoreBasedParameterValueKeyFirst);
    }
    if (m_additionalScoreBasedParameterStringKey[0].GetStringLength() > 0)
        mask |= 1u << cFlagScoreBasedParameterStringKey;
    return (m_stateFlags & mask) != 0;
}

const common::String* NexCreateSessionSetting::GetAdditionalScoreBasedParameterValueKey(unsigned int index) const
{
    if (index >= cAdditionalScoreBasedParameterValueNum)
        return 0;
    return &m_additionalScoreBasedParameterValueKey[index];
}

const ParamVariant* NexCreateSessionSetting::GetAdditionalScoreBasedParameterValue(unsigned int index) const
{
    return &m_additionalScoreBasedParameterValue[index];
}

const common::String* NexCreateSessionSetting::GetAdditionalScoreBasedParameterStringKey(unsigned int index) const
{
    if (index != 0)
        return 0;
    return &m_additionalScoreBasedParameterStringKey[0];
}

const common::String* NexCreateSessionSetting::GetAdditionalScoreBasedParameterString(unsigned int index) const
{
    if (index != 0)
        return 0;
    return &m_additionalScoreBasedParameterString[0];
}

void NexCreateSessionSetting::CopyApplicationDataToVector(void* vector) const
{
    typedef std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > ByteVector;
    ByteVector* bytes = static_cast<ByteVector*>(vector);
    bytes->clear();
    bytes->reserve(m_applicationDataSize);
    for (unsigned int index = 0; index < m_applicationDataSize; ++index)
        bytes->push_back(m_applicationData[index]);
}

void NexCreateSessionSetting::Trace(unsigned long long value) const
{
}

unsigned short NexCreateSessionSetting::GetExtraParticipantNum() const
{
    return m_extraParticipantNum;
}

void NexCreateSessionSetting::SetCommunityId(unsigned int value)
{
    m_communityId = value;
    m_stateFlags |= 1u << cFlagCommunityId;
}

unsigned int NexCreateSessionSetting::GetCommunityId() const
{
    return m_communityId;
}

bool NexCreateSessionSetting::IsSetCommunityId() const
{
    return (m_stateFlags >> cFlagCommunityId & 1) != 0;
}

}
}
}
