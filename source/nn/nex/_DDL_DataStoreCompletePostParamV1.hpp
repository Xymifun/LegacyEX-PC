#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEPOSTPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECOMPLETEPOSTPARAMV1_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreCompletePostParamV1 : public RootObject
{
public:
    _DDL_DataStoreCompletePostParamV1() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreCompletePostParamV1() {}

    _DDL_DataStoreCompletePostParamV1& operator=(const _DDL_DataStoreCompletePostParamV1& rSource);

    bool operator==(const _DDL_DataStoreCompletePostParamV1& rSource) const
    {
        return m_uiDataId == rSource.m_uiDataId && m_bIsSuccess == rSource.m_bIsSuccess;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreCompletePostParamV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreCompletePostParamV1* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiDataId;
    bool m_bIsSuccess;
};

}
}

#endif
