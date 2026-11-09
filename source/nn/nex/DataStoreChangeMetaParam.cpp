#include "nn/nex/DataStoreChangeMetaParam.hpp"

#include "nn/nex/DataStorePersistenceInitParam.hpp"

namespace nn
{
namespace nex
{

void DataStoreChangeMetaParam::Reset()
{
    m_uiDataId = 0;
    m_uiModifiesFlag = 0;
    m_Name = String(L"");
    m_Permission = DataStorePermission();
    m_DelPermission = DataStorePermission();
    m_uiPeriod = 90;
    m_Tags = qVector<String>();
    m_MetaBinary.clear();
    m_uiUpdatePassword = 0;
    m_uiReferredCnt = 0;
    m_uiDataType = 0;
    m_uiStatus = 0;
    m_CompareParam = DataStoreChangeMetaCompareParam();

    DataStorePersistenceTarget persistenceTarget;
    persistenceTarget.m_uiOwnerId = 0;
    persistenceTarget.m_uiPersistenceSlotId = DataStorePersistenceInitParam::NO_PERSISTENCE_SLOT_ID;
    m_PersistenceTarget = persistenceTarget;
}

}
}
