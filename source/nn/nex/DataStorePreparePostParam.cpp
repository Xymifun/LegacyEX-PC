#include "nn/nex/DataStorePreparePostParam.hpp"

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/DataStoreRatingInitParam.hpp"
#include "nn/nex/MemAllocator.hpp"

namespace nn
{
namespace nex
{

void DataStorePreparePostParam::Reset()
{
    m_uiSize = 0;
    m_Name = String(L"");
    m_uiDataType = 0;
    m_Permission = DataStorePermission();
    m_DelPermission = DataStorePermission();
    m_uiFlag = 0;
    m_uiPeriod = 90;
    m_uiReferDataId = 0;
    m_Tags = qVector<String>();
    m_MetaBinary.clear();

    DataStorePersistenceInitParam persistenceInitParam;
    persistenceInitParam.m_uiPersistenceSlotId = DataStorePersistenceInitParam::NO_PERSISTENCE_SLOT_ID;
    persistenceInitParam.m_bDeleteLastObject = true;
    m_PersistenceInitParam = persistenceInitParam;

    std::map<int8_t, DataStoreRatingInitParam, std::less<int8_t>,
             MemAllocator<std::pair<const int8_t, DataStoreRatingInitParam> > >
        ratingInitParams;

    m_RatingInitParams.clear();

    for (std::map<int8_t, DataStoreRatingInitParam, std::less<int8_t>,
                  MemAllocator<std::pair<const int8_t, DataStoreRatingInitParam> > >::const_iterator it =
             ratingInitParams.begin();
         it != ratingInitParams.end(); ++it)
    {
        DataStoreRatingInitParamWithSlot ratingInitParamWithSlot;
        ratingInitParamWithSlot.m_iSlot = it->first;
        ratingInitParamWithSlot.m_Param = it->second;
        m_RatingInitParams.push_back(ratingInitParamWithSlot);
    }

    m_ExtraData.clear();
}

}
}
