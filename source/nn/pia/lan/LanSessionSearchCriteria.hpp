#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONSEARCHCRITERIA_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanSessionInfo;

class LanSessionSearchCriteria : public session::SessionSearchCriteria
{
public:
    enum
    {
        cAttributeCount = 6,
        cAttributeListMax = 20
    };

    enum SetFlag
    {
        cSetFlagMinParticipantNum = 1 << 0,
        cSetFlagMaxParticipantNum = 1 << 1,
        cSetFlagOpenedOnly = 1 << 2,
        cSetFlagVacantOnly = 1 << 3,
        cSetFlagGameMode = 1 << 4,
        cSetFlagSessionType = 1 << 5,
        cAttributeFlagShift = 6
    };

    LanSessionSearchCriteria();
    virtual ~LanSessionSearchCriteria();
    virtual void Reset();
    virtual void Trace(unsigned long long value) const;

    void SetMaxParticipantNum(unsigned short participantNum);
    void SetVacantOnly(bool vacantOnly);
    void SetOpenedOnly(bool openedOnly);
    bool IsSetGameMode() const;
    bool IsSetMinParticipantNum() const;
    bool IsSetMaxParticipantNum() const;
    bool IsSetSessionType() const;
    bool IsSetAttribute(unsigned int index) const;
    bool IsHit(const LanSessionInfo* sessionInfo);
    unsigned int GetSerializedSize() const;
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int bufferSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int bufferSize);
    void Copy(const LanSessionSearchCriteria& other);
    LanSessionSearchCriteria& operator=(const LanSessionSearchCriteria& other);

    unsigned int m_gameMode;
    unsigned int m_sessionType;
    unsigned int m_attributeList[cAttributeCount][cAttributeListMax];
    unsigned int m_attributeListSize[cAttributeCount];
    unsigned int m_attributeRangeMin[cAttributeCount];
    unsigned int m_attributeRangeMax[cAttributeCount];
    bool m_attributeRangeUsed[cAttributeCount];
    unsigned short m_minParticipantRangeMax;
    unsigned short m_minParticipantRangeMin;
    unsigned short m_maxParticipantRangeMax;
    unsigned short m_maxParticipantRangeMin;
    bool m_openedOnly;
    bool m_vacantOnly;
    unsigned int m_setFlags;
};

}
}
}

#endif
