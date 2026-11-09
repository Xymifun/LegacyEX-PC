#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETSPECIFICMETAPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETSPECIFICMETAPARAMV1_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreGetSpecificMetaParamV1 : public RootObject
{
public:
    _DDL_DataStoreGetSpecificMetaParamV1() : m_uiStructureVersion(0), m_DataIds(0) {}

    virtual ~_DDL_DataStoreGetSpecificMetaParamV1() {}

    _DDL_DataStoreGetSpecificMetaParamV1& operator=(const _DDL_DataStoreGetSpecificMetaParamV1& rSource);
    bool operator==(const _DDL_DataStoreGetSpecificMetaParamV1& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreGetSpecificMetaParamV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreGetSpecificMetaParamV1* pDestination);

    uint8_t m_uiStructureVersion;
    qVector<uint32_t> m_DataIds;
};

}
}

#endif
