#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEPOSTPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEPOSTPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreCompletePostParam : public RootObject
{
public:
    _DDL_DataStoreCompletePostParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreCompletePostParam() {}

    _DDL_DataStoreCompletePostParam& operator=(const _DDL_DataStoreCompletePostParam& rSource);

    bool operator==(const _DDL_DataStoreCompletePostParam& rSource) const
    {
        return m_uiDataId == rSource.m_uiDataId && m_bIsSuccess == rSource.m_bIsSuccess;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreCompletePostParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreCompletePostParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    bool m_bIsSuccess;
};

}
}

#endif
