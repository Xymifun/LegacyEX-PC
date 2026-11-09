#include "nn/nex/DataStorePreparePostParamV1.hpp"

namespace nn
{
namespace nex
{

DataStorePreparePostParamV1::DataStorePreparePostParamV1()
{
    m_uiSize = 0;
    m_Name = String("");
    m_uiDataType = 0;
    m_MetaBinary.clear();
    m_Permission = DataStorePermission();
    m_DelPermission = DataStorePermission();
    m_uiFlag = 0;
    m_uiPeriod = 90;
    m_uiReferDataId = 0;
    m_Tags = qVector<String>();
}

}
}
