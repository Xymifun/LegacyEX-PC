#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETACOMPAREPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETACOMPAREPARAM_HPP

#include "nn/nex/_DDL_DataStoreChangeMetaCompareParam.hpp"

namespace nn
{
namespace nex
{

class DataStoreChangeMetaCompareParam : public _DDL_DataStoreChangeMetaCompareParam
{
public:
    DataStoreChangeMetaCompareParam() { Reset(); }

    virtual ~DataStoreChangeMetaCompareParam() {}

    void Reset();
};

}
}

#endif
