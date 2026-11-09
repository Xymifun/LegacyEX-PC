#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETNOTIFICATIONURLINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETNOTIFICATIONURLINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreReqGetNotificationUrlInfo : public RootObject
{
public:
    _DDL_DataStoreReqGetNotificationUrlInfo() : m_uiStructureVersion(0), m_RootCaCert(0) {}

    virtual ~_DDL_DataStoreReqGetNotificationUrlInfo() {}

    _DDL_DataStoreReqGetNotificationUrlInfo& operator=(const _DDL_DataStoreReqGetNotificationUrlInfo& rSource);
    bool operator==(const _DDL_DataStoreReqGetNotificationUrlInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreReqGetNotificationUrlInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqGetNotificationUrlInfo* pDestination);

    uint8_t m_uiStructureVersion;
    String m_Url;
    String m_Key;
    String m_Query;
    qVector<unsigned char> m_RootCaCert;
};

}
}

#endif
