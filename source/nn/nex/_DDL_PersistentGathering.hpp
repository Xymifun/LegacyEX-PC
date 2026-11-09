#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PERSISTENTGATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PERSISTENTGATHERING_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_PersistentGathering : public Gathering
{
public:
    _DDL_PersistentGathering()
        : Gathering(),
          m_uiStructureVersion(0),
          m_strPassword(),
          m_Attributes(0),
          m_ApplicationBuffer(0),
          m_ParticipationStartDate(),
          m_ParticipationEndDate()
    {
    }

    virtual ~_DDL_PersistentGathering() {}

    virtual Gathering* Clone() const;
    virtual String GetGatheringType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    _DDL_PersistentGathering& operator=(const _DDL_PersistentGathering& rSource);

    static void Add(Message* pMessage, const _DDL_PersistentGathering& rSource);
    static void Extract(Message* pMessage, _DDL_PersistentGathering* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiPersistentGatheringType;
    String m_strPassword;
    qVector<uint32_t> m_Attributes;
    qVector<uint8_t> m_ApplicationBuffer;
    DateTime m_ParticipationStartDate;
    DateTime m_ParticipationEndDate;
    uint32_t m_uiMatchmakeSessionCount;
    uint32_t m_uiParticipationCount;
};

}
}

#endif
