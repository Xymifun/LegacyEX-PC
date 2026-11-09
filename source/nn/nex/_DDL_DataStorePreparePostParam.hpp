#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREPOSTPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREPOSTPARAM_HPP

#include <stdint.h>

#include "nn/nex/DataStorePermission.hpp"
#include "nn/nex/DataStorePersistenceInitParam.hpp"
#include "nn/nex/DataStoreRatingInitParamWithSlot.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qBuffer.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePreparePostParam : public RootObject
{
public:
    _DDL_DataStorePreparePostParam() : m_uiStructureVersion(0), m_Tags(0), m_RatingInitParams(0), m_ExtraData(0) {}

    virtual ~_DDL_DataStorePreparePostParam() {}

    _DDL_DataStorePreparePostParam& operator=(const _DDL_DataStorePreparePostParam& rSource);
    bool operator==(const _DDL_DataStorePreparePostParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePreparePostParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePreparePostParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiSize;
    String m_Name;
    uint16_t m_uiDataType;
    qBuffer m_MetaBinary;
    DataStorePermission m_Permission;
    DataStorePermission m_DelPermission;
    uint32_t m_uiFlag;
    uint16_t m_uiPeriod;
    uint32_t m_uiReferDataId;
    qVector<String> m_Tags;
    qVector<DataStoreRatingInitParamWithSlot> m_RatingInitParams;
    DataStorePersistenceInitParam m_PersistenceInitParam;
    qVector<String> m_ExtraData;
};

}
}

#endif
