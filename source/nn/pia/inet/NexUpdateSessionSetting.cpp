#include "nn/pia/inet/NexUpdateSessionSetting.hpp"

#include <coreinit/memory.h>
#include <vector>
#include "nn/nex/MemAllocator.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

NexUpdateSessionSetting::NexUpdateSessionSetting()
    : session::UpdateSessionSetting(),
      m_minParticipantNum(0xffff),
      m_maxParticipantNum(0xffff),
      m_sessionType(cSessionType_Normal),
      m_autoCloseSession(false),
      m_applicationDataSize(0),
      m_selectionPriority(0),
      m_ratingValue(0),
      m_isSetRatingValue(false),
      m_disconnectionRate(0),
      m_isSetDisconnectionRate(false),
      m_violationRate(0),
      m_isSetViolationRate(false),
      m_isSetCountryCode(false),
      m_updateGeoIp(false),
      m_isSetUpdateGeoIp(false)
{
    m_nexModificationFlag = 0;
    m_sessionUserPassword.Clear();
    m_sessionMatchmakeKeyword.Clear();
    m_startedTime.Clear();
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_attributeUpdated, 0, sizeof(m_attributeUpdated));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    for (unsigned int index = 0; index < 3; ++index)
    {
        m_additionalScoreBasedParameterValueKey[index].Clear();
        m_additionalScoreBasedParameterValue[index] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey.Clear();
    m_additionalScoreBasedParameterString.Clear();
    m_countryCode = 0;
}

NexUpdateSessionSetting::~NexUpdateSessionSetting()
{
}

void NexUpdateSessionSetting::Reset()
{
    session::UpdateSessionSetting::Reset();
    m_nexModificationFlag = 0;
    m_sessionType = cSessionType_Normal;
    m_minParticipantNum = 0xffff;
    m_maxParticipantNum = 0xffff;
    m_autoCloseSession = false;
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_attributeUpdated, 0, sizeof(m_attributeUpdated));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
    m_selectionPriority = 0;
    m_sessionUserPassword.Clear();
    m_sessionMatchmakeKeyword.Clear();
    m_startedTime.Clear();
    m_ratingValue = 0;
    m_isSetRatingValue = false;
    m_disconnectionRate = 0;
    m_isSetDisconnectionRate = false;
    m_violationRate = 0;
    m_isSetViolationRate = false;
    m_isSetCountryCode = false;
    m_updateGeoIp = false;
    m_isSetUpdateGeoIp = false;
    for (unsigned int index = 0; index < 3; ++index)
    {
        m_additionalScoreBasedParameterValueKey[index].Clear();
        m_additionalScoreBasedParameterValue[index] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey.Clear();
    m_additionalScoreBasedParameterString.Clear();
    m_countryCode = 0;
}

void NexUpdateSessionSetting::Trace(unsigned long long value) const
{
}

void NexUpdateSessionSetting::AddNexModificationFlag(unsigned int flag)
{
    m_nexModificationFlag |= flag;
}

unsigned int NexUpdateSessionSetting::GetNexModificationFlag() const
{
    if ((m_modificationFlag & 2) == 0)
        return m_nexModificationFlag;
    return m_nexModificationFlag | 4;
}

unsigned short NexUpdateSessionSetting::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}

unsigned short NexUpdateSessionSetting::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}

SessionType NexUpdateSessionSetting::GetSessionType() const
{
    return m_sessionType;
}

bool NexUpdateSessionSetting::IsAutoCloseSession() const
{
    return m_autoCloseSession;
}

Result NexUpdateSessionSetting::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cAttributeNum)
        return 0x10c07;
    if ((m_nexModificationFlag != 0 && m_nexModificationFlag != 1) || m_modificationFlag != 0)
        return 0x10c08;

    unsigned int i = 0;
    m_attribute[index] = value;
    m_attributeUpdated[index] = true;

    bool allUpdated = true;
    for (; i < cAttributeNum; ++i)
        allUpdated &= m_attributeUpdated[i];
    if (allUpdated)
        AddNexModificationFlag(1);

    return 0;
}

unsigned int NexUpdateSessionSetting::GetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return 0;
    return m_attribute[index];
}

bool NexUpdateSessionSetting::IsAttributeUpdated(unsigned int index) const
{
    return m_attributeUpdated[index];
}

Result NexUpdateSessionSetting::SetApplicationData(const void* data, unsigned int size)
{
    if (data == 0)
        return 0x10c07;
    if (size > sizeof(m_applicationData))
        return 0x10c07;

    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    OSBlockMove(m_applicationData, data, size, FALSE);
    m_applicationDataSize = size;
    m_modificationFlag |= 2;
    return 0;
}

Result NexUpdateSessionSetting::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == 0)
    {
        return 0x10c07;
    }
    else if (m_applicationDataSize == 0)
    {
        return 0x10c08;
    }
    else if (bufferSize < m_applicationDataSize)
    {
        return 0x10c04;
    }
    else
    {
        OSBlockMove(buffer, m_applicationData, m_applicationDataSize, FALSE);
        return 0;
    }
}

unsigned int NexUpdateSessionSetting::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}

void NexUpdateSessionSetting::CopyApplicationDataToVector(void* vector) const
{
    typedef std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > ByteVector;
    ByteVector* bytes = static_cast<ByteVector*>(vector);
    bytes->clear();
    bytes->reserve(m_applicationDataSize);
    for (unsigned int index = 0; index < m_applicationDataSize; ++index)
        bytes->push_back(m_applicationData[index]);
}

const common::String& NexUpdateSessionSetting::GetSessionUserPassword() const
{
    return m_sessionUserPassword;
}

const common::DateTime& NexUpdateSessionSetting::GetStartedTime() const
{
    return m_startedTime;
}

const common::String* NexUpdateSessionSetting::GetAdditionalScoreBasedParameterValueKey(unsigned int index) const
{
    if (index >= 3)
        return 0;
    return &m_additionalScoreBasedParameterValueKey[index];
}

const ParamVariant* NexUpdateSessionSetting::GetAdditionalScoreBasedParameterValue(unsigned int index) const
{
    return &m_additionalScoreBasedParameterValue[index];
}

const common::String* NexUpdateSessionSetting::GetAdditionalScoreBasedParameterStringKey(unsigned int index) const
{
    if (index != 0)
        return 0;
    return &m_additionalScoreBasedParameterStringKey;
}

const common::String* NexUpdateSessionSetting::GetAdditionalScoreBasedParameterString(unsigned int index) const
{
    if (index != 0)
        return 0;
    return &m_additionalScoreBasedParameterString;
}

bool NexUpdateSessionSetting::IsSetScoreBaseParam() const
{
    bool isSet = m_isSetRatingValue | m_isSetDisconnectionRate | m_isSetViolationRate | m_isSetCountryCode
                 | m_isSetUpdateGeoIp;
    for (unsigned int index = 0; index < 3; ++index)
    {
        if (m_additionalScoreBasedParameterValueKey[index].GetStringLength() > 0)
            isSet = true;
    }
    if (m_additionalScoreBasedParameterStringKey.GetStringLength() > 0)
        isSet = true;
    return isSet;
}

bool NexUpdateSessionSetting::IsOnlyAttributeUpdated() const
{
    if (m_nexModificationFlag == 0 && !IsModificationFlagSet(cModificationFlag_ApplicationData))
    {
        bool updated = false;
        for (unsigned int index = 0; index < cAttributeNum; ++index)
            updated |= m_attributeUpdated[index];
        return updated;
    }
    return false;
}

bool NexUpdateSessionSetting::IsOnlyAllAttributesUpdated() const
{
    if (m_nexModificationFlag == 1 && !IsModificationFlagSet(cModificationFlag_ApplicationData))
        return true;
    return false;
}

bool NexUpdateSessionSetting::IsOnlyApplicationDataUpdated() const
{
    if (m_nexModificationFlag == 0 && IsModificationFlagSet(cModificationFlag_ApplicationData))
        return true;
    return false;
}

unsigned int NexUpdateSessionSetting::GetRatingValue() const
{
    return m_ratingValue;
}

bool NexUpdateSessionSetting::IsSetRatingValue() const
{
    return m_isSetRatingValue;
}

unsigned int NexUpdateSessionSetting::GetDisconnectionRate() const
{
    return m_disconnectionRate;
}

bool NexUpdateSessionSetting::IsSetDisconnectionRate() const
{
    return m_isSetDisconnectionRate;
}

unsigned int NexUpdateSessionSetting::GetViolationRate() const
{
    return m_violationRate;
}

bool NexUpdateSessionSetting::IsSetViolationRate() const
{
    return m_isSetViolationRate;
}

unsigned int NexUpdateSessionSetting::GetCountryCode() const
{
    return m_countryCode;
}

bool NexUpdateSessionSetting::IsSetCountryCode() const
{
    return m_isSetCountryCode;
}

bool NexUpdateSessionSetting::GetUpdateGeoIp() const
{
    return m_updateGeoIp;
}

unsigned char NexUpdateSessionSetting::GetSelectionPriority() const
{
    return m_selectionPriority;
}

bool NexUpdateSessionSetting::IsValid() const
{
    bool updated = false;
    for (unsigned int index = 0; index < cAttributeNum; ++index)
        updated |= m_attributeUpdated[index];
    if (updated && (m_nexModificationFlag & 1) == 0)
    {
        if (IsModificationFlagSet(cModificationFlag_ApplicationData))
            return false;
        if (m_nexModificationFlag != 0)
            return false;
    }
    else
    {
        if (m_nexModificationFlag != 0)
            return true;
    }
    if (!IsModificationFlagSet(cModificationFlag_ApplicationData) && !updated)
        return false;
    return true;
}

}
}
}
