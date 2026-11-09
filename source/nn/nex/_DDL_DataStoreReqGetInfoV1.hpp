#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETINFOV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETINFOV1_HPP

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

class _DDL_DataStoreReqGetInfoV1 : public RootObject
{
public:
    _DDL_DataStoreReqGetInfoV1() : m_uiStructureVersion(0), m_RequestHeaders(0), m_RootCaCert(0) {}

    virtual ~_DDL_DataStoreReqGetInfoV1() {}

    _DDL_DataStoreReqGetInfoV1& operator=(const _DDL_DataStoreReqGetInfoV1& rSource);
    bool operator==(const _DDL_DataStoreReqGetInfoV1& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreReqGetInfoV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqGetInfoV1* pDestination);

    uint8_t m_uiStructureVersion;
    String m_Url;
    qVector<DataStoreKeyValue> m_RequestHeaders;
    uint32_t m_uiSize;
    qVector<unsigned char> m_RootCaCert;
};

}
}

#endif
