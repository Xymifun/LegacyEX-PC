#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREGETPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPREPAREGETPARAM_HPP

#include <stdint.h>

#include "nn/nex/DataStorePersistenceTarget.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePrepareGetParam : public RootObject
{
public:
    _DDL_DataStorePrepareGetParam() : m_uiStructureVersion(0), m_ExtraData(0) {}

    virtual ~_DDL_DataStorePrepareGetParam() {}

    _DDL_DataStorePrepareGetParam& operator=(const _DDL_DataStorePrepareGetParam& rSource);
    bool operator==(const _DDL_DataStorePrepareGetParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePrepareGetParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePrepareGetParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiLockId;
    DataStorePersistenceTarget m_PersistenceTarget;
    uint64_t m_uiAccessPassword;
    qVector<String> m_ExtraData;
};

}
}

#endif
