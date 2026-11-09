#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREUPDATEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREUPDATEPARAM_HPP

#include "nn/nex/_DDL_DataStorePrepareUpdateParam.hpp"

namespace nn
{
namespace nex
{

class DataStorePrepareUpdateParam : public _DDL_DataStorePrepareUpdateParam
{
public:
    DataStorePrepareUpdateParam()
    {
        m_uiDataId = 0;
        m_uiSize = 0;
        m_uiUpdatePassword = 0;
        m_ExtraData.clear();
    }

    virtual ~DataStorePrepareUpdateParam() {}
};

}
}

#endif
