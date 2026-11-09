#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREMETAINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREMETAINFO_HPP

#include "nn/nex/_DDL_DataStoreMetaInfo.hpp"

namespace nn
{
namespace nex
{

class DataStoreMetaInfo : public _DDL_DataStoreMetaInfo
{
public:
    DataStoreMetaInfo() { m_uiDataId = 0; }

    virtual ~DataStoreMetaInfo() {}
};

}
}

#endif
