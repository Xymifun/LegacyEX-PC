#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREGETPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREGETPARAM_HPP

#include "nn/nex/_DDL_DataStorePrepareGetParam.hpp"

namespace nn
{
namespace nex
{

class DataStorePrepareGetParam : public _DDL_DataStorePrepareGetParam
{
public:
    DataStorePrepareGetParam()
    {
        m_uiDataId = 0;
        m_uiLockId = 0;
        m_PersistenceTarget = DataStorePersistenceTarget();
        m_uiAccessPassword = 0;
        m_ExtraData.clear();
    }

    virtual ~DataStorePrepareGetParam() {}
};

}
}

#endif
