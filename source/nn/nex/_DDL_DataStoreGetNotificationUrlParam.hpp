#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETNOTIFICATIONURLPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETNOTIFICATIONURLPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreGetNotificationUrlParam : public RootObject
{
public:
    _DDL_DataStoreGetNotificationUrlParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreGetNotificationUrlParam() {}

    bool operator==(const _DDL_DataStoreGetNotificationUrlParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreGetNotificationUrlParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreGetNotificationUrlParam* pDestination);

    uint8_t m_uiStructureVersion;
    String m_PreviousUrl;
};

}
}

#endif
