#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DELETIONENTRY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DELETIONENTRY_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DeletionEntry : public RootObject
{
public:
    _DDL_DeletionEntry() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DeletionEntry() {}

    static void Add(Message* pMessage, const _DDL_DeletionEntry& rSource);
    static void Extract(Message* pMessage, _DDL_DeletionEntry* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
    uint32_t m_uiValue3;
};

}
}

#endif
