#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_INVITATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_INVITATION_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_Invitation : public RootObject
{
public:
    _DDL_Invitation() : m_uiStructureVersion(0), m_StringValue() {}

    virtual ~_DDL_Invitation() {}

    static void Add(Message* pMessage, const _DDL_Invitation& rSource);
    static void Extract(Message* pMessage, _DDL_Invitation* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
    String m_StringValue;
};

}
}

#endif
