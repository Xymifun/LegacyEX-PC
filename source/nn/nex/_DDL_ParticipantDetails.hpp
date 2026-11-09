#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PARTICIPANTDETAILS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PARTICIPANTDETAILS_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_ParticipantDetails : public RootObject
{
public:
    _DDL_ParticipantDetails() : m_uiStructureVersion(0), m_StringValue1(), m_StringValue2() {}

    virtual ~_DDL_ParticipantDetails() {}

    static void Add(Message* pMessage, const _DDL_ParticipantDetails& rSource);
    static void Extract(Message* pMessage, _DDL_ParticipantDetails* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    String m_StringValue1;
    String m_StringValue2;
    uint16_t m_uiShortValue;
};

}
}

#endif
