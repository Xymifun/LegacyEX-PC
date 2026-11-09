#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREUPDATEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREUPDATEPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePrepareUpdateParam : public RootObject
{
public:
    _DDL_DataStorePrepareUpdateParam() : m_uiStructureVersion(0), m_ExtraData(0) {}

    virtual ~_DDL_DataStorePrepareUpdateParam() {}

    _DDL_DataStorePrepareUpdateParam& operator=(const _DDL_DataStorePrepareUpdateParam& rSource);
    bool operator==(const _DDL_DataStorePrepareUpdateParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePrepareUpdateParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePrepareUpdateParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiSize;
    uint64_t m_uiUpdatePassword;
    qVector<String> m_ExtraData;
};

}
}

#endif
