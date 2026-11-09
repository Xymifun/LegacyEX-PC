#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_UPDATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_UPDATEMATCHMAKESESSIONPARAM_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_UpdateMatchmakeSessionParam : public RootObject
{
public:
    _DDL_UpdateMatchmakeSessionParam()
        : m_uiStructureVersion(1),
          m_Attributes(),
          m_ApplicationBuffer(),
          m_MatchmakeParam(),
          m_StartedTime(),
          m_strValue1(),
          m_strValue2()
    {
    }

    virtual ~_DDL_UpdateMatchmakeSessionParam() {}

    _DDL_UpdateMatchmakeSessionParam& operator=(const _DDL_UpdateMatchmakeSessionParam& rSource);

    static void Add(Message* pMessage, const _DDL_UpdateMatchmakeSessionParam& rSource);
    static void Extract(Message* pMessage, _DDL_UpdateMatchmakeSessionParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
    qVector<uint32_t> m_Attributes;
    bool m_bOpenParticipation;
    qVector<uint8_t> m_ApplicationBuffer;
    uint8_t m_uiProgressScore;
    MatchmakeParam m_MatchmakeParam;
    DateTime m_StartedTime;
    String m_strValue1;
    uint32_t m_uiValue3;
    String m_strValue2;
    uint16_t m_uiValue4;
    uint16_t m_uiValue5;
    uint32_t m_uiValue6;
    uint32_t m_uiValue7;
    uint32_t m_uiValue8;
};

}
}

#endif
