#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORENOTIFICATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORENOTIFICATION_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreNotification : public RootObject
{
public:
    _DDL_DataStoreNotification() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreNotification() {}

    _DDL_DataStoreNotification& operator=(const _DDL_DataStoreNotification& rSource);
    bool operator==(const _DDL_DataStoreNotification& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreNotification& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreNotification* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiNotificationId;
    uint64_t m_uiDataId;
};

}
}

#endif
