#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETAPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORECHANGEMETAPARAM_HPP

#include <stdint.h>

#include "nn/nex/DataStoreChangeMetaCompareParam.hpp"
#include "nn/nex/DataStorePermission.hpp"
#include "nn/nex/DataStorePersistenceTarget.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qBuffer.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreChangeMetaParam : public RootObject
{
public:
    _DDL_DataStoreChangeMetaParam() : m_uiStructureVersion(1), m_Tags(0) {}

    virtual ~_DDL_DataStoreChangeMetaParam() {}

    _DDL_DataStoreChangeMetaParam& operator=(const _DDL_DataStoreChangeMetaParam& rSource);
    bool operator==(const _DDL_DataStoreChangeMetaParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreChangeMetaParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreChangeMetaParam* pDestination);

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
    uint32_t m_uiReferredCnt;
    uint16_t m_uiDataType;
    uint8_t m_uiStatus;
    DataStoreChangeMetaCompareParam m_CompareParam;
    DataStorePersistenceTarget m_PersistenceTarget;
};

}
}

#endif
