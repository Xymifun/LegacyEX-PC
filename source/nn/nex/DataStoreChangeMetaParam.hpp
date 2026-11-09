#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETAPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHANGEMETAPARAM_HPP

#include "nn/nex/_DDL_DataStoreChangeMetaParam.hpp"

namespace nn
{
namespace nex
{

class DataStoreChangeMetaParam : public _DDL_DataStoreChangeMetaParam
{
public:
    DataStoreChangeMetaParam() { Reset(); }

    virtual ~DataStoreChangeMetaParam() {}

    void Reset();
};

}
}

#endif
