#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREDELETEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREDELETEPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreDeleteParam : public RootObject
{
public:
    _DDL_DataStoreDeleteParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreDeleteParam() {}

    _DDL_DataStoreDeleteParam& operator=(const _DDL_DataStoreDeleteParam& rSource);
    bool operator==(const _DDL_DataStoreDeleteParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreDeleteParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreDeleteParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint64_t m_uiUpdatePassword;
};

}
}

#endif
