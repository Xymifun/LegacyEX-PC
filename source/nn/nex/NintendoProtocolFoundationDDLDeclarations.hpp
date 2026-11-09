#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPROTOCOLFOUNDATIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPROTOCOLFOUNDATIONDDLDECLARATIONS_HPP

#include <stdint.h>

#include "nn/nex/DDLDeclarations.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_u8KeyValue : public Data
{
public:
    _DDL_u8KeyValue() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_u8KeyValue() {}

    _DDL_u8KeyValue& operator=(const _DDL_u8KeyValue& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_u8KeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_u8KeyValue* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiKey;
    uint8_t m_uiValue;
};

class u8KeyValue : public _DDL_u8KeyValue
{
public:
    u8KeyValue() : _DDL_u8KeyValue() {}

    virtual ~u8KeyValue() {}
};

class _DDL_u32KeyValue : public Data
{
public:
    _DDL_u32KeyValue() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_u32KeyValue() {}

    _DDL_u32KeyValue& operator=(const _DDL_u32KeyValue& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_u32KeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_u32KeyValue* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiKey;
    uint32_t m_uiValue;
};

class u32KeyValue : public _DDL_u32KeyValue
{
public:
    u32KeyValue() : _DDL_u32KeyValue() {}

    virtual ~u32KeyValue() {}
};

class _DDL_u64KeyValue : public Data
{
public:
    _DDL_u64KeyValue() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_u64KeyValue() {}

    _DDL_u64KeyValue& operator=(const _DDL_u64KeyValue& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_u64KeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_u64KeyValue* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiKey;
    uint64_t m_uiValue;
};

class u64KeyValue : public _DDL_u64KeyValue
{
public:
    u64KeyValue() : _DDL_u64KeyValue() {}

    virtual ~u64KeyValue() {}
};

class _DDL_StringKeyValue : public Data
{
public:
    _DDL_StringKeyValue() : Data(), m_uiStructureVersion(0), m_strValue() {}

    virtual ~_DDL_StringKeyValue() {}

    _DDL_StringKeyValue& operator=(const _DDL_StringKeyValue& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_StringKeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_StringKeyValue* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiKey;
    String m_strValue;
};

class StringKeyValue : public _DDL_StringKeyValue
{
public:
    StringKeyValue() : _DDL_StringKeyValue() {}

    virtual ~StringKeyValue() {}
};

class _DDL_NintendoNotificationEventGeneral : public Data
{
public:
    _DDL_NintendoNotificationEventGeneral() : Data(), m_uiStructureVersion(0), m_strStrParam() {}

    virtual ~_DDL_NintendoNotificationEventGeneral() {}

    _DDL_NintendoNotificationEventGeneral& operator=(const _DDL_NintendoNotificationEventGeneral& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_NintendoNotificationEventGeneral& rSource);
    static void Extract(Message* pMessage, _DDL_NintendoNotificationEventGeneral* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiU32Param;
    uint64_t m_uiU64Param1;
    uint64_t m_uiU64Param2;
    String m_strStrParam;
};

class NintendoNotificationEventGeneral : public _DDL_NintendoNotificationEventGeneral
{
public:
    NintendoNotificationEventGeneral() : _DDL_NintendoNotificationEventGeneral() {}

    virtual ~NintendoNotificationEventGeneral() {}
};

class _DDL_NintendoNotificationEventProfile : public Data
{
public:
    _DDL_NintendoNotificationEventProfile() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_NintendoNotificationEventProfile() {}

    _DDL_NintendoNotificationEventProfile& operator=(const _DDL_NintendoNotificationEventProfile& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_NintendoNotificationEventProfile& rSource);
    static void Extract(Message* pMessage, _DDL_NintendoNotificationEventProfile* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiRegion;
    uint8_t m_uiCountry;
    uint8_t m_uiArea;
    uint8_t m_uiLanguage;
    uint8_t m_uiPlatform;
};

class NintendoNotificationEventProfile : public _DDL_NintendoNotificationEventProfile
{
public:
    NintendoNotificationEventProfile() : _DDL_NintendoNotificationEventProfile() {}

    virtual ~NintendoNotificationEventProfile() {}
};

class _DDL_NintendoNotificationEventKeyValue : public Data
{
public:
    _DDL_NintendoNotificationEventKeyValue() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_NintendoNotificationEventKeyValue() {}

    _DDL_NintendoNotificationEventKeyValue& operator=(const _DDL_NintendoNotificationEventKeyValue& rSource);

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_NintendoNotificationEventKeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_NintendoNotificationEventKeyValue* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    qList<u8KeyValue> m_U8KeyValues;
    qList<u32KeyValue> m_U32KeyValues;
    qList<u64KeyValue> m_U64KeyValues;
    qList<StringKeyValue> m_StringKeyValues;
};

class NintendoNotificationEventKeyValue : public _DDL_NintendoNotificationEventKeyValue
{
public:
    NintendoNotificationEventKeyValue() : _DDL_NintendoNotificationEventKeyValue() {}

    virtual ~NintendoNotificationEventKeyValue() {}
};

class NintendoProtocolFoundationDDLDeclarations : public DDLDeclarations
{
public:
    NintendoProtocolFoundationDDLDeclarations();

    virtual ~NintendoProtocolFoundationDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<NintendoProtocolFoundationDDLDeclarations> s_declarations;
};

}
}

#endif
