#include "nn/nex/DataStoreSearchParam.hpp"

namespace nn
{
namespace nex
{

DataStoreSearchParam::DataStoreSearchParam()
{
    Reset();
}

void DataStoreSearchParam::Reset()
{
    m_uiSearchTarget = 1;
    m_OwnerIds = qVector<uint32_t>();
    m_uiOwnerType = 0;
    m_DestinationIds = qVector<uint32_t>();
    m_uiDataType = 0xFFFF;
    m_DataTypes = qVector<uint16_t>();
    m_CreatedAfter = DateTime::s_future;
    m_CreatedBefore = DateTime::s_future;
    m_UpdatedAfter = DateTime::s_future;
    m_UpdatedBefore = DateTime::s_future;
    m_uiResultOrderColumn = 0;
    m_uiReferDataId = 0;
    m_uiResultOrder = 0;
    m_ResultRange = ResultRange();
    m_uiResultOption = 0;
    m_Tags = qVector<String>();
    m_bUseCache = false;
    m_bTotalCountEnabled = false;
    m_uiMinimalRatingFrequency = 0;
}

}
}
