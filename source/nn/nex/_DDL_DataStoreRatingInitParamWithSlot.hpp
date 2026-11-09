#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINITPARAMWITHSLOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINITPARAMWITHSLOT_HPP

#include <stdint.h>

#include "nn/nex/DataStoreRatingInitParam.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingInitParamWithSlot : public RootObject
{
public:
    _DDL_DataStoreRatingInitParamWithSlot() : m_uiStructureVersion(0), m_Param() {}

    virtual ~_DDL_DataStoreRatingInitParamWithSlot() {}

    _DDL_DataStoreRatingInitParamWithSlot& operator=(const _DDL_DataStoreRatingInitParamWithSlot& rSource);
    bool operator==(const _DDL_DataStoreRatingInitParamWithSlot& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRatingInitParamWithSlot& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingInitParamWithSlot* pDestination);

    uint8_t m_uiStructureVersion;
    int8_t m_iSlot;
    DataStoreRatingInitParam m_Param;
};

}
}

#endif
