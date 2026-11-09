#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREREQGETINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREREQGETINFO_HPP

#include "nn/nex/_DDL_DataStoreReqGetInfo.hpp"

namespace nn
{
namespace nex
{

class DataStoreReqGetInfo : public _DDL_DataStoreReqGetInfo
{
public:
    DataStoreReqGetInfo() { m_uiDataId = 0; }

    virtual ~DataStoreReqGetInfo() {}
};

}
}

#endif
