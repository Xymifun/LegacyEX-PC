#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIA_HPP

#include <stdint.h>

#include "nn/pia/common/FixedString.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/inet/ParamVariant.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace nex
{
class InetAddress;
class MatchmakeParam;
class MatchmakeSessionSearchCriteria;
class ResultRange;
}
}

namespace nn
{
namespace pia
{
namespace inet
{

extern const wchar_t* s_scoreSettingIndexKey;
extern const wchar_t* s_countryCodeKey;
extern const wchar_t* s_overrideIpAddressKey;
extern const wchar_t* s_useGeoIpKey;
extern const wchar_t* s_ratingValueKey;
extern const wchar_t* s_disconnectionRateKey;
extern const wchar_t* s_violationRateKey;
extern const wchar_t* s_totalScoreKey;
extern const wchar_t* s_updateGeoIpKey;

enum SelectionMethod
{
    cSelectionMethod_0 = 0,
    cSelectionMethod_1 = 1,
    cSelectionMethod_2 = 2
};

class NexSessionSearchCriteria : public session::SessionSearchCriteria
{
public:
    enum
    {
        cAttributeNum = 6,
        cAttributeListSize = 100,
        cParticipantNumUnset = 0xffff
    };

    NexSessionSearchCriteria();
    virtual ~NexSessionSearchCriteria();

    virtual nn::pia::Result SetResultRange(unsigned int offset, unsigned int size);
    void SetMinParticipantNum(unsigned short value);
    void SetMaxParticipantNum(unsigned short value);
    void SetGameMode(unsigned int value);
    unsigned int IsSetSessionMatchmakeKeyword() const;
    nn::pia::Result SetAttribute(unsigned int index, unsigned int value);
    nn::pia::Result SetAttributeRange(unsigned int index, unsigned int minimum, unsigned int maximum);
    void SetRandomSessionSelectionMethod(SelectionMethod method);
    const common::String* GetAdditionalScoreBasedParameterValueKey(unsigned int index) const
    {
        if (index >= cAttributeNum / 2)
            return 0;
        return &m_additionalScoreBasedParameterValueKey[index];
    }
    const ParamVariant& GetAdditionalScoreBasedParameterValue(unsigned int index) const
    {
        return m_additionalScoreBasedParameterValues[index];
    }
    const common::String* GetAdditionalScoreBasedParameterStringKey(unsigned int index) const
    {
        if (index != 0)
            return 0;
        return &m_additionalScoreBasedParameterStringKey[0];
    }
    const common::String* GetAdditionalScoreBasedParameterString(unsigned int index) const
    {
        if (index != 0)
            return 0;
        return &m_additionalScoreBasedParameterString[0];
    }
    void Copy(const NexSessionSearchCriteria& other);
    nn::pia::Result ConvertToNexMatchmakeSessionSearchCriteria(nn::nex::MatchmakeSessionSearchCriteria* criteria,
                                                               nn::nex::InetAddress* address,
                                                               nn::nex::MatchmakeParam* param,
                                                               nn::nex::ResultRange* resultRange,
                                                               unsigned char method) const;
    virtual void Trace(unsigned long long value) const;
    virtual void Reset();
    unsigned int IsValid() const;

private:
    unsigned int m_gameMode;
    SessionType m_sessionType;
    unsigned int m_attributeList[cAttributeNum][cAttributeListSize];
    unsigned int m_attributeListSize[cAttributeNum];
    unsigned int m_attributeRangeMin[cAttributeNum];
    unsigned int m_attributeRangeMax[cAttributeNum];
    bool m_attributeRangeUsed[cAttributeNum];
    bool m_excludeNonHostPrincipalId;
    bool m_excludeUserPasswordSet;
    SelectionMethod m_randomSessionSelectionMethod;
    unsigned int m_scoreBasedSettingIndex;
    unsigned int m_scoreBasedRatingValue;
    unsigned int m_scoreBasedDisconnectionRate;
    unsigned int m_scoreBasedViolationRate;
    unsigned int m_countryCode;
    bool m_scoreBasedGeoIpUsed;
    common::InetAddress m_overrideIpAddress;
    unsigned int m_communityId;
    unsigned short m_minParticipantRangeMax;
    unsigned short m_minParticipantRangeMin;
    unsigned short m_maxParticipantRangeMax;
    unsigned short m_maxParticipantRangeMin;
    bool m_openedOnly;
    bool m_vacantOnly;
    common::FixedString<8> m_additionalScoreBasedParameterValueKey[cAttributeNum / 2];
    ParamVariant m_additionalScoreBasedParameterValues[cAttributeNum / 2];
    common::FixedString<8> m_additionalScoreBasedParameterStringKey[1];
    common::FixedString<64> m_additionalScoreBasedParameterString[1];
    common::FixedString<32> m_sessionMatchmakeKeyword;
    unsigned int m_conditionMask;
};

}
}
}

#endif
