#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERSISTENCEINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERSISTENCEINFO_HPP

#include "nn/nex/_DDL_DataStorePersistenceInfo.hpp"

namespace nn
{
namespace nex
{

class DataStorePersistenceInfo : public _DDL_DataStorePersistenceInfo
{
public:
    DataStorePersistenceInfo() { m_uiDataId = 0; }

    virtual ~DataStorePersistenceInfo() {}
};

}
}

#endif
