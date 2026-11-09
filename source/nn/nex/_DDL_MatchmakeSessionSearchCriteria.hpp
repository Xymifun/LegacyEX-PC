#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKESESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKESESSIONSEARCHCRITERIA_HPP

#include <stdint.h>

#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_MatchmakeSessionSearchCriteria : public RootObject
{
public:
    _DDL_MatchmakeSessionSearchCriteria()
        : m_uiStructureVersion(3),
          m_Attributes(0),
          m_strGameMode(),
          m_strMinParticipants(),
          m_strMaxParticipants(),
          m_strMatchmakeSystemType(),
          m_MatchmakeParam()
    {
    }

    virtual ~_DDL_MatchmakeSessionSearchCriteria() {}

    _DDL_MatchmakeSessionSearchCriteria& operator=(const _DDL_MatchmakeSessionSearchCriteria& rSource);

    static void Add(Message* pMessage, const _DDL_MatchmakeSessionSearchCriteria& rSource);
    static void Extract(Message* pMessage, _DDL_MatchmakeSessionSearchCriteria* pDestination);

    uint8_t m_uiStructureVersion;
    qVector<String> m_Attributes;
    String m_strGameMode;
    String m_strMinParticipants;
    String m_strMaxParticipants;
    String m_strMatchmakeSystemType;
    bool m_bVacantOnly;
    bool m_bExcludeLocked;
    bool m_bExcludeNonHostPid;
    uint32_t m_uiSelectionMethod;
    uint16_t m_uiVacantParticipants;
    MatchmakeParam m_MatchmakeParam;
    bool m_bExcludeUserPasswordSet;
    bool m_bExcludeSystemPasswordSet;
    uint32_t m_uiReferGatheringID;
};

}
}

#endif
