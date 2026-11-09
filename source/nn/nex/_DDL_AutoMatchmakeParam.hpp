#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_AUTOMATCHMAKEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_AUTOMATCHMAKEPARAM_HPP

#include <stdint.h>

#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_AutoMatchmakeParam : public RootObject
{
public:
    _DDL_AutoMatchmakeParam()
        : m_uiStructureVersion(0),
          m_MatchmakeSession(),
          m_AdditionalParticipants(),
          m_strJoinMessage(),
          m_SearchCriteria(),
          m_TargetGatheringIDs()
    {
    }

    virtual ~_DDL_AutoMatchmakeParam() {}

    _DDL_AutoMatchmakeParam& operator=(const _DDL_AutoMatchmakeParam& rSource);

    static void Add(Message* pMessage, const _DDL_AutoMatchmakeParam& rSource);
    static void Extract(Message* pMessage, _DDL_AutoMatchmakeParam* pDestination);

    uint8_t m_uiStructureVersion;
    MatchmakeSession m_MatchmakeSession;
    qList<uint32_t> m_AdditionalParticipants;
    uint32_t m_uiGatheringIDForParticipationCheck;
    uint32_t m_uiAutoMatchmakeOption;
    String m_strJoinMessage;
    uint16_t m_uiParticipationCount;
    qList<MatchmakeSessionSearchCriteria> m_SearchCriteria;
    qList<uint32_t> m_TargetGatheringIDs;
};

}
}

#endif
