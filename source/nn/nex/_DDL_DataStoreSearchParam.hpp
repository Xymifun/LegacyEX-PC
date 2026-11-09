#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHPARAM_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreSearchParam : public RootObject
{
public:
    _DDL_DataStoreSearchParam() : m_uiStructureVersion(3), m_OwnerIds(0), m_DestinationIds(0), m_Tags(0), m_DataTypes(0)
    {
    }

    virtual ~_DDL_DataStoreSearchParam() {}

    _DDL_DataStoreSearchParam& operator=(const _DDL_DataStoreSearchParam& rSource);
    bool operator==(const _DDL_DataStoreSearchParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreSearchParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreSearchParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiSearchTarget;
    qVector<uint32_t> m_OwnerIds;
    uint8_t m_uiOwnerType;
    qVector<uint32_t> m_DestinationIds;
    uint16_t m_uiDataType;
    DateTime m_CreatedAfter;
    DateTime m_CreatedBefore;
    DateTime m_UpdatedAfter;
    DateTime m_UpdatedBefore;
    uint32_t m_uiReferDataId;
    qVector<String> m_Tags;
    uint8_t m_uiResultOrderColumn;
    uint8_t m_uiResultOrder;
    ResultRange m_ResultRange;
    uint8_t m_uiResultOption;
    uint32_t m_uiMinimalRatingFrequency;
    bool m_bUseCache;
    bool m_bTotalCountEnabled;
    qVector<uint16_t> m_DataTypes;
};

}
}

#endif
