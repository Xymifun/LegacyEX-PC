#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATEOBJECTPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATEOBJECTPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRateObjectParam : public RootObject
{
public:
    _DDL_DataStoreRateObjectParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreRateObjectParam() {}

    _DDL_DataStoreRateObjectParam& operator=(const _DDL_DataStoreRateObjectParam& rSource);
    bool operator==(const _DDL_DataStoreRateObjectParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRateObjectParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRateObjectParam* pDestination);

    uint8_t m_uiStructureVersion;
    int32_t m_iRatingValue;
    uint64_t m_uiAccessPassword;
};

}
}

#endif
