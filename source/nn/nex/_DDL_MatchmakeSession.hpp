#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKESESSION_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_MatchmakeSession : public Gathering
{
public:
    _DDL_MatchmakeSession()
        : Gathering(),
          m_uiStructureVersion(3),
          m_Attributes(0),
          m_ApplicationBuffer(0),
          m_SessionKey(0),
          m_MatchmakeParam(),
          m_StartedTime(),
          m_strUserPassword()
    {
    }

    virtual ~_DDL_MatchmakeSession() {}

    virtual Gathering* Clone() const;
    virtual String GetGatheringType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    _DDL_MatchmakeSession& operator=(const _DDL_MatchmakeSession& rSource);

    static void Add(Message* pMessage, const _DDL_MatchmakeSession& rSource);
    static void Extract(Message* pMessage, _DDL_MatchmakeSession* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiGameMode;
    qVector<uint32_t> m_Attributes;
    bool m_bOpenParticipation;
    uint32_t m_uiMatchmakeSystemType;
    qVector<uint8_t> m_ApplicationBuffer;
    uint32_t m_uiParticipationCount;
    uint8_t m_uiProgressScore;
    qVector<uint8_t> m_SessionKey;
    uint32_t m_uiOption;
    MatchmakeParam m_MatchmakeParam;
    DateTime m_StartedTime;
    String m_strUserPassword;
    uint32_t m_uiReferGatheringID;
    bool m_bUserPasswordEnabled;
    bool m_bSystemPasswordEnabled;
};

}
}

#endif
