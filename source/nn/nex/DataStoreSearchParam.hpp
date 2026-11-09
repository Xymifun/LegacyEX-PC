#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORESEARCHPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORESEARCHPARAM_HPP

#include "nn/nex/_DDL_DataStoreSearchParam.hpp"

namespace nn
{
namespace nex
{

class DataStoreSearchParam : public _DDL_DataStoreSearchParam
{
public:
    DataStoreSearchParam();
    virtual ~DataStoreSearchParam() {}

    void Reset();
};

}
}

#endif
