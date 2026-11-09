#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETAPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETAPARAMV1_HPP

#include <stdint.h>

#include "nn/nex/DataStorePermission.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qBuffer.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreChangeMetaParamV1 : public RootObject
{
public:
    _DDL_DataStoreChangeMetaParamV1() : m_uiStructureVersion(0), m_Tags(0) {}

    virtual ~_DDL_DataStoreChangeMetaParamV1() {}

    _DDL_DataStoreChangeMetaParamV1& operator=(const _DDL_DataStoreChangeMetaParamV1& rSource);
    bool operator==(const _DDL_DataStoreChangeMetaParamV1& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreChangeMetaParamV1& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreChangeMetaParamV1* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiModifiesFlag;
    String m_Name;
    DataStorePermission m_Permission;
    DataStorePermission m_DelPermission;
    uint16_t m_uiPeriod;
    qBuffer m_MetaBinary;
    qVector<String> m_Tags;
    uint64_t m_uiUpdatePassword;
};

}
}

#endif
