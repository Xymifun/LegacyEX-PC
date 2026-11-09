#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORETOUCHOBJECTPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORETOUCHOBJECTPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreTouchObjectParam : public RootObject
{
public:
    _DDL_DataStoreTouchObjectParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreTouchObjectParam() {}

    _DDL_DataStoreTouchObjectParam& operator=(const _DDL_DataStoreTouchObjectParam& rSource);

    bool operator==(const _DDL_DataStoreTouchObjectParam& rSource) const
    {
        return m_uiDataId == rSource.m_uiDataId && m_uiLockId == rSource.m_uiLockId
               && m_uiAccessPassword == rSource.m_uiAccessPassword;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreTouchObjectParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreTouchObjectParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiLockId;
    uint64_t m_uiAccessPassword;
};

}
}

#endif
