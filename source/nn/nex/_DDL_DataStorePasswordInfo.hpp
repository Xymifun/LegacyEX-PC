#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPASSWORDINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPASSWORDINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePasswordInfo : public RootObject
{
public:
    _DDL_DataStorePasswordInfo() { m_uiStructureVersion = 0; }

    virtual ~_DDL_DataStorePasswordInfo() {}

    _DDL_DataStorePasswordInfo& operator=(const _DDL_DataStorePasswordInfo& rSource);
    bool operator==(const _DDL_DataStorePasswordInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePasswordInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePasswordInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint64_t m_uiAccessPassword;
    uint64_t m_uiUpdatePassword;
};

}
}

#endif
