#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_RESULTRANGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_RESULTRANGE_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_ResultRange : public RootObject
{
public:
    _DDL_ResultRange() { m_uiStructureVersion = 0; }

    virtual ~_DDL_ResultRange() {}

    _DDL_ResultRange& operator=(const _DDL_ResultRange& rSource);
    bool operator==(const _DDL_ResultRange& rSource) const;

    static void Add(Message* pMessage, const _DDL_ResultRange& rSource);
    static void Extract(Message* pMessage, _DDL_ResultRange* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiOffset;
    uint32_t m_uiSize;
};

}
}

#endif
