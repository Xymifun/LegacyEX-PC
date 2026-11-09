#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERMISSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERMISSION_HPP

#include <stdint.h>

#include "nn/nex/_DDL_DataStorePermission.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class DataStorePermission : public _DDL_DataStorePermission
{
public:
    DataStorePermission(uint8_t uiPermission = 3, const qVector<uint32_t>& rRecipientIds = qVector<uint32_t>())
    {
        m_uiPermission = uiPermission;
        m_RecipientIds = rRecipientIds;
    }

    virtual ~DataStorePermission() {}
};

}
}

#endif
