#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETACOMPAREPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETACOMPAREPARAM_HPP

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

class _DDL_DataStoreChangeMetaCompareParam : public RootObject
{
public:
    _DDL_DataStoreChangeMetaCompareParam();

    virtual ~_DDL_DataStoreChangeMetaCompareParam() {}

    _DDL_DataStoreChangeMetaCompareParam& operator=(const _DDL_DataStoreChangeMetaCompareParam& rSource);
    bool operator==(const _DDL_DataStoreChangeMetaCompareParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreChangeMetaCompareParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreChangeMetaCompareParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiComparisonFlag;
    String m_Name;
    DataStorePermission m_Permission;
    DataStorePermission m_DelPermission;
    uint16_t m_uiPeriod;
    qBuffer m_MetaBinary;
    qVector<String> m_Tags;
    uint32_t m_uiReferredCnt;
    uint16_t m_uiDataType;
    uint8_t m_uiStatus;
};

}
}

#endif
