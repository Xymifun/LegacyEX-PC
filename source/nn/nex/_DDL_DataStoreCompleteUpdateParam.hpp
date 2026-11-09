#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEUPDATEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEUPDATEPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreCompleteUpdateParam : public RootObject
{
public:
    _DDL_DataStoreCompleteUpdateParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreCompleteUpdateParam() {}

    _DDL_DataStoreCompleteUpdateParam& operator=(const _DDL_DataStoreCompleteUpdateParam& rSource);

    bool operator==(const _DDL_DataStoreCompleteUpdateParam& rSource) const
    {
        return m_uiDataId == rSource.m_uiDataId && m_uiVersion == rSource.m_uiVersion
               && m_bIsSuccess == rSource.m_bIsSuccess;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreCompleteUpdateParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreCompleteUpdateParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiVersion;
    bool m_bIsSuccess;
};

}
}

#endif
