#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETINFO_HPP

#include <stdint.h>

#include "nn/nex/DataStoreKeyValue.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreReqGetInfo : public RootObject
{
public:
    _DDL_DataStoreReqGetInfo() : m_uiStructureVersion(0), m_RequestHeaders(0), m_RootCaCert(0) {}

    virtual ~_DDL_DataStoreReqGetInfo() {}

    _DDL_DataStoreReqGetInfo& operator=(const _DDL_DataStoreReqGetInfo& rSource);
    bool operator==(const _DDL_DataStoreReqGetInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreReqGetInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqGetInfo* pDestination);

    uint8_t m_uiStructureVersion;
    String m_Url;
    qVector<DataStoreKeyValue> m_RequestHeaders;
    uint32_t m_uiSize;
    qVector<unsigned char> m_RootCaCert;
    uint64_t m_uiDataId;
};

}
}

#endif
