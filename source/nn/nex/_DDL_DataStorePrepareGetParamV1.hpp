#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREGETPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREGETPARAMV1_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePrepareGetParamV1 : public RootObject
{
public:
    _DDL_DataStorePrepareGetParamV1() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStorePrepareGetParamV1() {}

    _DDL_DataStorePrepareGetParamV1& operator=(const _DDL_DataStorePrepareGetParamV1& rSource);
    bool operator==(const _DDL_DataStorePrepareGetParamV1& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePrepareGetParamV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePrepareGetParamV1* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiDataId;
    uint32_t m_uiLockId;
};

}
}

#endif
