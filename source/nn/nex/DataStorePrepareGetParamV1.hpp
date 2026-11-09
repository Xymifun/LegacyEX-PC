#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREGETPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREGETPARAMV1_HPP

#include "nn/nex/_DDL_DataStorePrepareGetParamV1.hpp"

namespace nn
{
namespace nex
{

class DataStorePrepareGetParamV1 : public _DDL_DataStorePrepareGetParamV1
{
public:
    DataStorePrepareGetParamV1() { m_uiDataId = 0; }

    virtual ~DataStorePrepareGetParamV1() {}
};

}
}

#endif
