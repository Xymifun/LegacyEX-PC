#include "nn/pia/inet/NexSessionSearchCriteria.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Variant.hpp"
#include "nn/nex/qVector.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"

#include <cstring>

namespace nn
{
namespace pia
{
namespace inet
{

const wchar_t* s_scoreSettingIndexKey = L"@SI";
const wchar_t* s_countryCodeKey = L"@NCC";
const wchar_t* s_overrideIpAddressKey = L"@OIA";
const wchar_t* s_useGeoIpKey = L"@UsGI";
const wchar_t* s_ratingValueKey = L"@RV";
const wchar_t* s_disconnectionRateKey = L"@DR";
const wchar_t* s_violationRateKey = L"@VR";
const wchar_t* s_totalScoreKey = L"@TS";
const wchar_t* s_updateGeoIpKey = L"@UpGI";

namespace
{

enum
{
    cConditionMinParticipants = 1u << 0,
    cConditionMaxParticipants = 1u << 1,
    cConditionExcludeLocked = 1u << 2,
    cConditionVacantOnly = 1u << 3,
    cConditionGameMode = 1u << 4,
    cConditionAttributeShift = 6,
    cConditionExcludeNonHostPrincipalId = 1u << 12,
    cConditionExcludeUserPasswordSet = 1u << 13,
    cConditionRandomSessionSelectionMethod = 1u << 14,
    cConditionScoreBasedSettingIndex = 1u << 15,
    cConditionScoreBasedRatingValue = 1u << 16,
    cConditionScoreBasedDisconnectionRate = 1u << 17,
    cConditionScoreBasedViolationRate = 1u << 18,
    cConditionCountryCode = 1u << 19,
    cConditionScoreBasedGeoIp = 1u << 20,
    cConditionOverrideIpAddress = 1u << 21,
    cConditionCommunityId = 1u << 22,
    cConditionAdditionalString = 1u << 24,
    cConditionAdditionalValueShift = 25
};

enum
{
    cMonitoringFlagExcludeUserPasswordSet = 1 << 0,
    cMonitoringFlagExcludeNonHostPrincipalId = 1 << 1,
    cMonitoringFlagExcludeLocked = 1 << 2,
    cMonitoringFlagVacantOnly = 1 << 3
};

enum
{
    cMonitoringMethodFirst = 0,
    cMonitoringMethodSecond = 1
};

enum
{
    cResultRangeOffsetUnset = 0xffffffff,
    cResultRangeSizeMax = 100
};

}

NexSessionSearchCriteria::NexSessionSearchCriteria()
    : session::SessionSearchCriteria(),
      m_gameMode(0),
      m_sessionType(cSessionType_Normal),
      m_excludeNonHostPrincipalId(true),
      m_excludeUserPasswordSet(true),
      m_randomSessionSelectionMethod(cSelectionMethod_0),
      m_scoreBasedSettingIndex(0),
      m_scoreBasedRatingValue(0),
      m_scoreBasedDisconnectionRate(0),
      m_scoreBasedViolationRate(0),
      m_scoreBasedGeoIpUsed(false),
      m_overrideIpAddress(),
      m_communityId(0),
      m_minParticipantRangeMax(cParticipantNumUnset),
      m_minParticipantRangeMin(cParticipantNumUnset),
      m_maxParticipantRangeMax(cParticipantNumUnset),
      m_maxParticipantRangeMin(cParticipantNumUnset),
      m_openedOnly(true),
      m_vacantOnly(true),
      m_additionalScoreBasedParameterValueKey(),
      m_additionalScoreBasedParameterValues(),
      m_additionalScoreBasedParameterStringKey(),
      m_additionalScoreBasedParameterString(),
      m_sessionMatchmakeKeyword(),
      m_conditionMask(0)
{
    m_overrideIpAddress.m_address = 0;
    m_overrideIpAddress.m_port = 0;
    m_sessionMatchmakeKeyword.Clear();
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValueKey[i].Clear();
        m_additionalScoreBasedParameterValues[i] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey[0].Clear();
    m_additionalScoreBasedParameterString[0].Clear();
    for (unsigned int i = 0; i < cAttributeNum; ++i)
    {
        std::memset(m_attributeList[i], 0, sizeof(m_attributeList[i]));
        m_attributeListSize[i] = 0;
        m_attributeRangeMin[i] = 0;
        m_attributeRangeMax[i] = 0;
        m_attributeRangeUsed[i] = false;
    }
    m_countryCode = 0;
    m_resultRangeSize = cResultRangeSizeMax;
    m_resultRangeOffset = cResultRangeOffsetUnset;
}

NexSessionSearchCriteria::~NexSessionSearchCriteria()
{
}

nn::pia::Result NexSessionSearchCriteria::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > cResultRangeSizeMax)
        return transport::cResultInvalidArgument;
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

void NexSessionSearchCriteria::SetMinParticipantNum(unsigned short value)
{
    m_minParticipantRangeMin = value;
    m_minParticipantRangeMax = cParticipantNumUnset;
    m_conditionMask |= cConditionMinParticipants;
}

void NexSessionSearchCriteria::SetMaxParticipantNum(unsigned short value)
{
    m_maxParticipantRangeMin = value;
    m_maxParticipantRangeMax = cParticipantNumUnset;
    m_conditionMask |= cConditionMaxParticipants;
}

void NexSessionSearchCriteria::SetGameMode(unsigned int value)
{
    m_gameMode = value;
    m_conditionMask |= cConditionGameMode;
}

unsigned int NexSessionSearchCriteria::IsSetSessionMatchmakeKeyword() const
{
    return (m_conditionMask >> 23) & 1;
}

nn::pia::Result NexSessionSearchCriteria::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cAttributeNum)
        return transport::cResultInvalidArgument;
    if (IsSetSessionMatchmakeKeyword() != 0)
        return transport::cResultInvalidState;

    m_attributeList[index][0] = value;
    m_attributeListSize[index] = 1;
    m_attributeRangeUsed[index] = false;
    m_conditionMask |= 1u << (index + cConditionAttributeShift);
    return 0;
}

nn::pia::Result NexSessionSearchCriteria::SetAttributeRange(unsigned int index, unsigned int minimum,
                                                            unsigned int maximum)
{
    if (index >= cAttributeNum)
        return transport::cResultInvalidArgument;
    if (minimum > maximum)
        return transport::cResultInvalidArgument;
    if (IsSetSessionMatchmakeKeyword() != 0)
        return transport::cResultInvalidState;

    m_attributeRangeMin[index] = minimum;
    m_attributeRangeMax[index] = maximum;
    m_attributeRangeUsed[index] = true;
    m_conditionMask |= 1u << (index + cConditionAttributeShift);
    return 0;
}

void NexSessionSearchCriteria::SetRandomSessionSelectionMethod(SelectionMethod method)
{
    m_randomSessionSelectionMethod = method;
    m_conditionMask |= cConditionRandomSessionSelectionMethod;
}

void NexSessionSearchCriteria::Copy(const NexSessionSearchCriteria& other)
{
    session::SessionSearchCriteria::Copy(other);
    m_gameMode = other.m_gameMode;
    m_sessionType = other.m_sessionType;
    for (unsigned int i = 0; i < cAttributeNum; ++i)
    {
        for (unsigned int j = 0; j < cAttributeListSize; ++j)
            m_attributeList[i][j] = other.m_attributeList[i][j];
        m_attributeListSize[i] = other.m_attributeListSize[i];
        m_attributeRangeMin[i] = other.m_attributeRangeMin[i];
        m_attributeRangeMax[i] = other.m_attributeRangeMax[i];
        m_attributeRangeUsed[i] = other.m_attributeRangeUsed[i];
    }
    m_excludeNonHostPrincipalId = other.m_excludeNonHostPrincipalId;
    m_excludeUserPasswordSet = other.m_excludeUserPasswordSet;
    m_randomSessionSelectionMethod = other.m_randomSessionSelectionMethod;
    m_scoreBasedSettingIndex = other.m_scoreBasedSettingIndex;
    m_scoreBasedRatingValue = other.m_scoreBasedRatingValue;
    m_scoreBasedDisconnectionRate = other.m_scoreBasedDisconnectionRate;
    m_scoreBasedViolationRate = other.m_scoreBasedViolationRate;
    m_scoreBasedGeoIpUsed = other.m_scoreBasedGeoIpUsed;
    m_overrideIpAddress = other.m_overrideIpAddress;
    m_sessionMatchmakeKeyword = other.m_sessionMatchmakeKeyword;
    m_communityId = other.m_communityId;
    m_minParticipantRangeMax = other.m_minParticipantRangeMax;
    m_minParticipantRangeMin = other.m_minParticipantRangeMin;
    m_maxParticipantRangeMax = other.m_maxParticipantRangeMax;
    m_maxParticipantRangeMin = other.m_maxParticipantRangeMin;
    m_openedOnly = other.m_openedOnly;
    m_vacantOnly = other.m_vacantOnly;
    m_conditionMask = other.m_conditionMask;
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValueKey[i] = other.m_additionalScoreBasedParameterValueKey[i];
        m_additionalScoreBasedParameterValues[i] = other.m_additionalScoreBasedParameterValues[i];
    }
    m_additionalScoreBasedParameterStringKey[0].Copy(other.m_additionalScoreBasedParameterStringKey[0]);
    m_additionalScoreBasedParameterString[0].Copy(other.m_additionalScoreBasedParameterString[0]);
    m_countryCode = other.m_countryCode;
}

void NexSessionSearchCriteria::Trace(unsigned long long) const
{
}

void NexSessionSearchCriteria::Reset()
{
    session::SessionSearchCriteria::Reset();
    m_gameMode = 0;
    m_sessionType = cSessionType_Normal;
    m_excludeNonHostPrincipalId = true;
    m_excludeUserPasswordSet = true;
    m_randomSessionSelectionMethod = cSelectionMethod_0;
    m_scoreBasedSettingIndex = 0;
    m_scoreBasedRatingValue = 0;
    m_scoreBasedDisconnectionRate = 0;
    m_scoreBasedViolationRate = 0;
    m_scoreBasedGeoIpUsed = false;
    m_communityId = 0;
    m_minParticipantRangeMax = cParticipantNumUnset;
    m_minParticipantRangeMin = cParticipantNumUnset;
    m_maxParticipantRangeMax = cParticipantNumUnset;
    m_maxParticipantRangeMin = cParticipantNumUnset;
    m_openedOnly = true;
    m_vacantOnly = true;
    m_overrideIpAddress.m_address = 0;
    m_overrideIpAddress.m_port = 0;
    m_sessionMatchmakeKeyword.Clear();
    m_conditionMask = 0;
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValueKey[i].Clear();
        m_additionalScoreBasedParameterValues[i] = ParamVariant();
    }
    m_additionalScoreBasedParameterStringKey[0].Clear();
    m_additionalScoreBasedParameterString[0].Clear();
    for (unsigned int i = 0; i < cAttributeNum; ++i)
    {
        std::memset(m_attributeList[i], 0, sizeof(m_attributeList[i]));
        m_attributeListSize[i] = 0;
        m_attributeRangeMin[i] = 0;
        m_attributeRangeMax[i] = 0;
        m_attributeRangeUsed[i] = false;
    }
    m_countryCode = 0;
    m_resultRangeSize = cResultRangeSizeMax;
    m_resultRangeOffset = cResultRangeOffsetUnset;
}

nn::pia::Result NexSessionSearchCriteria::ConvertToNexMatchmakeSessionSearchCriteria(
    nn::nex::MatchmakeSessionSearchCriteria* criteria, nn::nex::InetAddress* address, nn::nex::MatchmakeParam* param,
    nn::nex::ResultRange* resultRange, unsigned char method) const
{
    if (criteria != 0)
    {
        criteria->Reset();
        if ((m_conditionMask & cConditionGameMode) != 0)
            criteria->SetGameMode(m_gameMode);
        criteria->SetMatchmakeSystemType(NexMatchmakeSession::ConvertSessionTypeToMatchmakeSystemType(m_sessionType));

        if ((m_conditionMask & cConditionMinParticipants) != 0)
        {
            if (m_minParticipantRangeMin != cParticipantNumUnset)
            {
                if (m_minParticipantRangeMax != cParticipantNumUnset)
                    criteria->SetMinParticipantsWithRange(m_minParticipantRangeMin, m_minParticipantRangeMax);
                else
                    criteria->SetMinParticipants(m_minParticipantRangeMin);
            }
            else if (m_minParticipantRangeMax != cParticipantNumUnset)
            {
                criteria->SetMinParticipants(m_minParticipantRangeMax);
            }
        }
        if ((m_conditionMask & cConditionMaxParticipants) != 0)
        {
            if (m_maxParticipantRangeMin != cParticipantNumUnset)
            {
                if (m_maxParticipantRangeMax != cParticipantNumUnset)
                    criteria->SetMaxParticipantsWithRange(m_maxParticipantRangeMin, m_maxParticipantRangeMax);
                else
                    criteria->SetMaxParticipants(m_maxParticipantRangeMin);
            }
            else if (m_maxParticipantRangeMax != cParticipantNumUnset)
            {
                criteria->SetMaxParticipants(m_maxParticipantRangeMax);
            }
        }
        if ((m_conditionMask & cConditionExcludeLocked) != 0)
            criteria->SetExcludeLocked(m_openedOnly);
        if ((m_conditionMask & cConditionVacantOnly) != 0)
            criteria->SetVacantOnly(m_vacantOnly);
        if ((m_conditionMask & cConditionExcludeNonHostPrincipalId) != 0)
            criteria->SetExcludeNonHostPid(m_excludeNonHostPrincipalId);
        if ((m_conditionMask & cConditionExcludeUserPasswordSet) != 0)
            criteria->SetExcludeUserPasswordSet(m_excludeUserPasswordSet);
        if ((m_conditionMask & cConditionCommunityId) != 0)
            criteria->m_uiReferGatheringID = m_communityId;

        if (m_randomSessionSelectionMethod == cSelectionMethod_2)
        {
            param->m_Parameters.clear();
            if ((m_conditionMask & cConditionScoreBasedSettingIndex) != 0)
                param->m_Parameters[nn::nex::String(s_scoreSettingIndexKey)] = nn::nex::Variant(
                    m_scoreBasedSettingIndex);
            if ((m_conditionMask & cConditionScoreBasedRatingValue) != 0)
                param->m_Parameters[nn::nex::String(s_ratingValueKey)] = nn::nex::Variant(m_scoreBasedRatingValue);
            if ((m_conditionMask & cConditionScoreBasedDisconnectionRate) != 0)
                param->m_Parameters[nn::nex::String(s_disconnectionRateKey)] = nn::nex::Variant(
                    m_scoreBasedDisconnectionRate);
            if ((m_conditionMask & cConditionScoreBasedViolationRate) != 0)
                param->m_Parameters[nn::nex::String(s_violationRateKey)] = nn::nex::Variant(m_scoreBasedViolationRate);
            if ((m_conditionMask & cConditionScoreBasedGeoIp) != 0)
                param->m_Parameters[nn::nex::String(s_useGeoIpKey)] = nn::nex::Variant(m_scoreBasedGeoIpUsed);
            if ((m_conditionMask & cConditionOverrideIpAddress) != 0)
            {
                NexFacade::ConvertInetAddressToNexInetAddress(m_overrideIpAddress, address);
                nn::nex::String addressString = address->GetAddressStr();
                param->m_Parameters[nn::nex::String(s_overrideIpAddressKey)] = nn::nex::Variant(addressString);
            }
            if ((m_conditionMask & cConditionCountryCode) != 0)
                param->m_Parameters[nn::nex::String(s_countryCodeKey)] = nn::nex::Variant(m_countryCode);

            for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
            {
                if ((m_conditionMask & (1u << (i + cConditionAdditionalValueShift))) == 0)
                    continue;
                const common::String* key = GetAdditionalScoreBasedParameterValueKey(i);
                const ParamVariant& value = GetAdditionalScoreBasedParameterValue(i);
                if (key == 0 || key->GetStringLength() <= 0)
                    continue;
                switch (value.m_type)
                {
                case ParamVariant::cType_UInt64:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_uint64Value);
                    break;
                case ParamVariant::cType_Int64:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_int64Value);
                    break;
                case ParamVariant::cType_UInt32:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_uint32Value);
                    break;
                case ParamVariant::cType_Int32:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_int32Value);
                    break;
                case ParamVariant::cType_Bool:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_boolValue);
                    break;
                case ParamVariant::cType_Double:
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(value.m_doubleValue);
                    break;
                default:
                    break;
                }
            }

            if ((m_conditionMask & cConditionAdditionalString) != 0)
            {
                const common::String* key = GetAdditionalScoreBasedParameterStringKey(0);
                const common::String* text = GetAdditionalScoreBasedParameterString(0);
                if (key != 0 && key->GetStringLength() > 0 && text != 0 && text->GetStringLength() > 0)
                {
                    param->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                        nn::nex::Variant(static_cast<const wchar_t*>(text->GetBuffer()));
                }
            }

            criteria->SetSelectionMethod(
                NexMatchmakeSession::ConvertSelectionMethodToMatchmakeSelectionMethod(m_randomSessionSelectionMethod),
                *param);
        }
        else
        {
            criteria->SetSelectionMethod(
                NexMatchmakeSession::ConvertSelectionMethodToMatchmakeSelectionMethod(m_randomSessionSelectionMethod));
        }

        unsigned char flags = 0;
        if (m_vacantOnly)
            flags |= cMonitoringFlagVacantOnly;
        if (m_openedOnly)
            flags |= cMonitoringFlagExcludeLocked;
        if (m_excludeNonHostPrincipalId)
            flags |= cMonitoringFlagExcludeNonHostPrincipalId;
        if (m_excludeUserPasswordSet)
            flags |= cMonitoringFlagExcludeUserPasswordSet;

        if (method == cMonitoringMethodFirst)
        {
            if ((m_conditionMask & cConditionGameMode) != 0)
                common::g_sessionBeginMonitoringContent.m_metric84 = m_gameMode;
            common::g_sessionBeginMonitoringContent.m_metric85 = static_cast<unsigned char>(m_minParticipantRangeMin);
            common::g_sessionBeginMonitoringContent.m_metric86 = static_cast<unsigned char>(m_minParticipantRangeMax);
            common::g_sessionBeginMonitoringContent.m_metric87 = static_cast<unsigned char>(m_maxParticipantRangeMin);
            common::g_sessionBeginMonitoringContent.m_metric88 = static_cast<unsigned char>(m_maxParticipantRangeMax);
            common::g_sessionBeginMonitoringContent.m_metric92 = static_cast<unsigned char>(m_sessionType);
            common::g_sessionBeginMonitoringContent.m_metric93 = flags;
            common::g_sessionBeginMonitoringContent.m_metric94 = static_cast<unsigned char>(
                m_randomSessionSelectionMethod);
            common::g_sessionBeginMonitoringContent.m_metric113 = m_scoreBasedGeoIpUsed;
            common::g_sessionBeginMonitoringContent.m_metric109 = m_scoreBasedRatingValue;
            common::g_sessionBeginMonitoringContent.m_metric111 = m_scoreBasedDisconnectionRate;
            common::g_sessionBeginMonitoringContent.m_metric110 = m_scoreBasedViolationRate;
            common::g_sessionBeginMonitoringContent.m_metric112 = static_cast<unsigned char>(m_countryCode);
        }
        else if (method == cMonitoringMethodSecond)
        {
            if ((m_conditionMask & cConditionGameMode) != 0)
                common::g_sessionBeginMonitoringContent.m_metric95 = m_gameMode;
            common::g_sessionBeginMonitoringContent.m_metric96 = static_cast<unsigned char>(m_minParticipantRangeMin);
            common::g_sessionBeginMonitoringContent.m_metric97 = static_cast<unsigned char>(m_minParticipantRangeMax);
            common::g_sessionBeginMonitoringContent.m_metric98 = static_cast<unsigned char>(m_maxParticipantRangeMin);
            common::g_sessionBeginMonitoringContent.m_metric99 = static_cast<unsigned char>(m_maxParticipantRangeMax);
            common::g_sessionBeginMonitoringContent.m_metric103 = static_cast<unsigned char>(m_sessionType);
            common::g_sessionBeginMonitoringContent.m_metric104 = flags;
            common::g_sessionBeginMonitoringContent.m_metric105 = static_cast<unsigned char>(
                m_randomSessionSelectionMethod);
            common::g_sessionBeginMonitoringContent.m_metric118 = m_scoreBasedGeoIpUsed;
            common::g_sessionBeginMonitoringContent.m_metric114 = m_scoreBasedRatingValue;
            common::g_sessionBeginMonitoringContent.m_metric116 = m_scoreBasedDisconnectionRate;
            common::g_sessionBeginMonitoringContent.m_metric115 = m_scoreBasedViolationRate;
            common::g_sessionBeginMonitoringContent.m_metric117 = static_cast<unsigned char>(m_countryCode);
        }

        for (unsigned int i = 0; i < cAttributeNum; ++i)
        {
            if ((m_conditionMask & (1u << (i + cConditionAttributeShift))) == 0)
                continue;
            if (m_attributeRangeUsed[i])
            {
                criteria->SetAttributeWithRange(i, m_attributeRangeMin[i], m_attributeRangeMax[i]);
                if (method == cMonitoringMethodFirst)
                {
                    common::g_sessionBeginMonitoringContent.m_metric90[i] = m_attributeRangeMin[i];
                    common::g_sessionBeginMonitoringContent.m_metric91[i] = m_attributeRangeMax[i];
                }
                else if (method == cMonitoringMethodSecond)
                {
                    common::g_sessionBeginMonitoringContent.m_metric101[i] = m_attributeRangeMin[i];
                    common::g_sessionBeginMonitoringContent.m_metric102[i] = m_attributeRangeMax[i];
                }
                continue;
            }
            if (m_attributeListSize[i] != 0)
            {
                if (m_attributeListSize[i] < 2)
                {
                    criteria->SetAttribute(i, m_attributeList[i][0]);
                }
                else
                {
                    nn::nex::qVector<unsigned int> values;
                    for (unsigned int j = 0; j < m_attributeListSize[i]; ++j)
                        values.push_back(m_attributeList[i][j]);
                    criteria->SetAttribute(i, values);
                }
            }
            if (method == cMonitoringMethodFirst)
                common::g_sessionBeginMonitoringContent.m_metric89[i] = static_cast<unsigned char>(
                    m_attributeListSize[i]);
            else if (method == cMonitoringMethodSecond)
                common::g_sessionBeginMonitoringContent.m_metric100[i] = static_cast<unsigned char>(
                    m_attributeListSize[i]);
        }
    }

    if (resultRange != 0)
    {
        resultRange->m_uiOffset = m_resultRangeOffset;
        resultRange->m_uiSize = m_resultRangeSize;
    }
    return 0;
}

unsigned int NexSessionSearchCriteria::IsValid() const
{
    return (m_conditionMask >> 4) & 1;
}

}
}
}
