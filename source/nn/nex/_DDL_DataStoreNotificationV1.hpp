#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORENOTIFICATIONV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORENOTIFICATIONV1_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreNotificationV1 : public RootObject
{
public:
    _DDL_DataStoreNotificationV1() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreNotificationV1() {}

    _DDL_DataStoreNotificationV1& operator=(const _DDL_DataStoreNotificationV1& rSource);
    bool operator==(const _DDL_DataStoreNotificationV1& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreNotificationV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreNotificationV1* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiNotificationId;
    uint32_t m_uiDataId;
};

}
}

#endif
