#include "nn/nex/DataStoreChangeMetaParamV1.hpp"

namespace nn
{
namespace nex
{

void DataStoreChangeMetaParamV1::Reset()
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
}

}
}
