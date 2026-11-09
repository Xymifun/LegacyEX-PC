#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_BASICACCOUNTINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_BASICACCOUNTINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_BasicAccountInfo : public RootObject
{
public:
    _DDL_BasicAccountInfo() : m_uiStructureVersion(0), m_Name() {}

    virtual ~_DDL_BasicAccountInfo() {}

    _DDL_BasicAccountInfo& operator=(const _DDL_BasicAccountInfo& rSource);
    bool operator==(const _DDL_BasicAccountInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_BasicAccountInfo& rSource);
    static void Extract(Message* pMessage, _DDL_BasicAccountInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiPid;
    String m_Name;
};

}
}

#endif
