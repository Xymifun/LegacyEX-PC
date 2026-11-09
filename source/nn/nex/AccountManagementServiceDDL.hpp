#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTSERVICEDDL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTSERVICEDDL_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_PublicData : public Data
{
public:
    _DDL_PublicData() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_PublicData() {}

    _DDL_PublicData& operator=(const _DDL_PublicData& rSource);
    bool operator==(const _DDL_PublicData& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_PublicData& rSource);
    static void Extract(Message* pMessage, _DDL_PublicData* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
};

class PublicData : public _DDL_PublicData
{
public:
    PublicData() : _DDL_PublicData() {}
};

class _DDL_PrivateData : public Data
{
public:
    _DDL_PrivateData() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_PrivateData() {}

    _DDL_PrivateData& operator=(const _DDL_PrivateData& rSource);
    bool operator==(const _DDL_PrivateData& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_PrivateData& rSource);
    static void Extract(Message* pMessage, _DDL_PrivateData* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
};

class PrivateData : public _DDL_PrivateData
{
public:
    PrivateData() : _DDL_PrivateData() {}
};

}
}

#endif
