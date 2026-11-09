#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINFOWITHSLOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINFOWITHSLOT_HPP

#include <stdint.h>

#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingInfoWithSlot : public RootObject
{
public:
    _DDL_DataStoreRatingInfoWithSlot() : m_uiStructureVersion(0), m_Rating() {}

    virtual ~_DDL_DataStoreRatingInfoWithSlot() {}

    _DDL_DataStoreRatingInfoWithSlot& operator=(const _DDL_DataStoreRatingInfoWithSlot& rSource);
    bool operator==(const _DDL_DataStoreRatingInfoWithSlot& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRatingInfoWithSlot& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingInfoWithSlot* pDestination);

    uint8_t m_uiStructureVersion;
    int8_t m_iSlot;
    DataStoreRatingInfo m_Rating;
};

}
}

#endif
