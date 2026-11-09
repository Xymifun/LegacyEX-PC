#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQPOSTINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQPOSTINFO_HPP

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

class _DDL_DataStoreReqPostInfo : public RootObject
{
public:
    _DDL_DataStoreReqPostInfo() : m_uiStructureVersion(0), m_RequestHeaders(0), m_FormFields(0), m_RootCaCert(0) {}

    virtual ~_DDL_DataStoreReqPostInfo() {}

    _DDL_DataStoreReqPostInfo& operator=(const _DDL_DataStoreReqPostInfo& rSource);
    bool operator==(const _DDL_DataStoreReqPostInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreReqPostInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqPostInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    String m_Url;
    qVector<DataStoreKeyValue> m_RequestHeaders;
    qVector<DataStoreKeyValue> m_FormFields;
    qVector<unsigned char> m_RootCaCert;
};

}
}

#endif
