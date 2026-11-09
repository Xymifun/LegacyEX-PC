#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETNEWARRIVEDNOTIFICATIONSPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETNEWARRIVEDNOTIFICATIONSPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreGetNewArrivedNotificationsParam : public RootObject
{
public:
    _DDL_DataStoreGetNewArrivedNotificationsParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreGetNewArrivedNotificationsParam() {}

    _DDL_DataStoreGetNewArrivedNotificationsParam& operator=(
        const _DDL_DataStoreGetNewArrivedNotificationsParam& rSource);
    bool operator==(const _DDL_DataStoreGetNewArrivedNotificationsParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreGetNewArrivedNotificationsParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreGetNewArrivedNotificationsParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiLastNotificationId;
    uint16_t m_uiLimit;
};

}
}

#endif
