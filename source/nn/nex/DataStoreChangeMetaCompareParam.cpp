#include "nn/nex/DataStoreChangeMetaCompareParam.hpp"

namespace nn
{
namespace nex
{

void DataStoreChangeMetaCompareParam::Reset()
{
    m_uiComparisonFlag = 0;
    m_Name = String(L"");
    m_Permission = DataStorePermission();
    m_DelPermission = DataStorePermission();
    m_uiPeriod = 90;
    m_Tags = qVector<String>();
    m_MetaBinary.clear();
    m_uiReferredCnt = 0;
    m_uiDataType = 0;
    m_uiStatus = 0;
}

}
}
