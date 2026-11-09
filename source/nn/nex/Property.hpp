#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROPERTY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROPERTY_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Variant.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_Property : public RootObject
{
public:
    _DDL_Property() : m_uiStructureVersion(0) {}

    virtual ~_DDL_Property() {}

    _DDL_Property& operator=(const _DDL_Property& rSource);
    bool operator==(const _DDL_Property& rSource) const;

    static void Add(Message* pMessage, const _DDL_Property& rSource);
    static void Extract(Message* pMessage, _DDL_Property* pDestination);

    uint8_t m_uiStructureVersion;
    unsigned int m_ID;
    int m_Value;
};

class Property : public _DDL_Property
{
public:
    Property();
    virtual ~Property();

    unsigned int GetID() const;
    void SetID(const unsigned int& uiID);
    int GetValue() const;
    void SetValue(const int& iValue);
};

class _DDL_PropertyVariant : public RootObject
{
public:
    _DDL_PropertyVariant() : m_uiStructureVersion(0), m_Value() {}

    virtual ~_DDL_PropertyVariant() {}

    _DDL_PropertyVariant& operator=(const _DDL_PropertyVariant& rSource);
    bool operator==(const _DDL_PropertyVariant& rSource) const;

    static void Add(Message* pMessage, const _DDL_PropertyVariant& rSource);
    static void Extract(Message* pMessage, _DDL_PropertyVariant* pDestination);

    uint8_t m_uiStructureVersion;
    unsigned int m_ID;
    Variant m_Value;
};

class PropertyVariant : public _DDL_PropertyVariant
{
public:
    PropertyVariant();
    virtual ~PropertyVariant();

    unsigned int GetID() const;
    void SetID(const unsigned int& uiID);
    Variant GetValue() const;
    void SetValue(const Variant& value);
};

}
}

#endif
