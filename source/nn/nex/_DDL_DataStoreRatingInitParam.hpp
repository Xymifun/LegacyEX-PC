#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINITPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINITPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingInitParam : public RootObject
{
public:
    _DDL_DataStoreRatingInitParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreRatingInitParam() {}

    _DDL_DataStoreRatingInitParam& operator=(const _DDL_DataStoreRatingInitParam& rSource);
    bool operator==(const _DDL_DataStoreRatingInitParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRatingInitParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingInitParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiFlag;
    uint8_t m_uiInternalFlag;
    uint8_t m_uiLockType;
    int64_t m_iInitialValue;
    int32_t m_iRangeMin;
    int32_t m_iRangeMax;
    int8_t m_iPeriodHour;
    int16_t m_iPeriodDuration;
};

}
}

#endif
