#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETAPARAMV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETAPARAMV1_HPP

#include "nn/nex/_DDL_DataStoreChangeMetaParamV1.hpp"

namespace nn
{
namespace nex
{

class DataStoreChangeMetaParamV1 : public _DDL_DataStoreChangeMetaParamV1
{
public:
    DataStoreChangeMetaParamV1() { Reset(); }

    virtual ~DataStoreChangeMetaParamV1() {}

    void Reset();
};

}
}

#endif
