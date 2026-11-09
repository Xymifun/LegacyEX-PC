#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGTARGET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGTARGET_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingTarget : public RootObject
{
public:
    _DDL_DataStoreRatingTarget() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreRatingTarget() {}

    _DDL_DataStoreRatingTarget& operator=(const _DDL_DataStoreRatingTarget& rSource);
    bool operator==(const _DDL_DataStoreRatingTarget& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRatingTarget& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingTarget* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    int8_t m_iSlot;
};

}
}

#endif
