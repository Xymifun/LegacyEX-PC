#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_CREATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_CREATEMATCHMAKESESSIONPARAM_HPP

#include <stdint.h>

#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_CreateMatchmakeSessionParam : public RootObject
{
public:
    _DDL_CreateMatchmakeSessionParam()
        : m_uiStructureVersion(0),
          m_MatchmakeSession(),
          m_AdditionalParticipants(),
          m_strJoinMessage()
    {
    }

    virtual ~_DDL_CreateMatchmakeSessionParam() {}

    _DDL_CreateMatchmakeSessionParam& operator=(const _DDL_CreateMatchmakeSessionParam& rSource);

    static void Add(Message* pMessage, const _DDL_CreateMatchmakeSessionParam& rSource);
    static void Extract(Message* pMessage, _DDL_CreateMatchmakeSessionParam* pDestination);

    uint8_t m_uiStructureVersion;
    MatchmakeSession m_MatchmakeSession;
    qList<uint32_t> m_AdditionalParticipants;
    uint32_t m_uiGatheringIDForParticipationCheck;
    uint32_t m_uiCreateMatchmakeSessionOption;
    String m_strJoinMessage;
    uint16_t m_uiParticipationCount;
};

}
}

#endif
