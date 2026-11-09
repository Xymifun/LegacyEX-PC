#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_CONNECTIONDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_CONNECTIONDATA_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_ConnectionData : public RootObject
{
public:
    _DDL_ConnectionData() : m_uiStructureVersion(0) {}

    virtual ~_DDL_ConnectionData() {}

    _DDL_ConnectionData& operator=(const _DDL_ConnectionData& rSource);
    bool operator==(const _DDL_ConnectionData& rSource) const;

    static void Add(Message* pMessage, const _DDL_ConnectionData& rSource);
    static void Extract(Message* pMessage, _DDL_ConnectionData* pDestination);

    uint8_t m_uiStructureVersion;
    StationURL m_StationURL;
    uint32_t m_uiValue;
};

}
}

#endif
