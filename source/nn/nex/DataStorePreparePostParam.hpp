#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREPOSTPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPREPAREPOSTPARAM_HPP

#include "nn/nex/_DDL_DataStorePreparePostParam.hpp"

namespace nn
{
namespace nex
{

class DataStorePreparePostParam : public _DDL_DataStorePreparePostParam
{
public:
    DataStorePreparePostParam() { Reset(); }

    virtual ~DataStorePreparePostParam() {}

    void Reset();
};

}
}

#endif
