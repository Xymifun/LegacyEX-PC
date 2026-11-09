#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_JOINMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_JOINMATCHMAKESESSIONPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_JoinMatchmakeSessionParam : public RootObject
{
public:
    _DDL_JoinMatchmakeSessionParam()
        : m_uiStructureVersion(1),
          m_AdditionalParticipants(),
          m_strUserPassword(),
          m_strSystemPassword(),
          m_strJoinMessage()
    {
    }

    virtual ~_DDL_JoinMatchmakeSessionParam() {}

    _DDL_JoinMatchmakeSessionParam& operator=(const _DDL_JoinMatchmakeSessionParam& rSource);

    static void Add(Message* pMessage, const _DDL_JoinMatchmakeSessionParam& rSource);
    static void Extract(Message* pMessage, _DDL_JoinMatchmakeSessionParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiGatheringID;
    qList<uint32_t> m_AdditionalParticipants;
    uint32_t m_uiGatheringIDForParticipationCheck;
    uint32_t m_uiJoinMatchmakeSessionOption;
    uint8_t m_uiJoinMatchmakeSessionBehavior;
    String m_strUserPassword;
    String m_strSystemPassword;
    String m_strJoinMessage;
    uint16_t m_uiParticipationCount;
    uint16_t m_uiExtraParticipants;
};

}
}

#endif
