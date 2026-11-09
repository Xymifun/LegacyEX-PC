#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQUPDATEINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQUPDATEINFO_HPP

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

class _DDL_DataStoreReqUpdateInfo : public RootObject
{
public:
    _DDL_DataStoreReqUpdateInfo() : m_uiStructureVersion(0), m_RequestHeaders(0), m_FormFields(0), m_RootCaCert(0) {}

    virtual ~_DDL_DataStoreReqUpdateInfo() {}

    _DDL_DataStoreReqUpdateInfo& operator=(const _DDL_DataStoreReqUpdateInfo& rSource);
    bool operator==(const _DDL_DataStoreReqUpdateInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreReqUpdateInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqUpdateInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiVersion;
    String m_Url;
    qVector<DataStoreKeyValue> m_RequestHeaders;
    qVector<DataStoreKeyValue> m_FormFields;
    qVector<unsigned char> m_RootCaCert;
};

}
}

#endif
