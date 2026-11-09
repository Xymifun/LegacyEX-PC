#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPASSWORDINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPASSWORDINFO_HPP

#include "nn/nex/_DDL_DataStorePasswordInfo.hpp"

namespace nn
{
namespace nex
{

class DataStorePasswordInfo : public _DDL_DataStorePasswordInfo
{
public:
    DataStorePasswordInfo() { m_uiDataId = 0; }

    virtual ~DataStorePasswordInfo() {}
};

}
}

#endif
