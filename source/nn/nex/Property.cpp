#include "nn/nex/Property.hpp"

namespace nn
{
namespace nex
{

Property::Property()
{
}

Property::~Property()
{
}

unsigned int Property::GetID() const
{
    return m_ID;
}

void Property::SetID(const unsigned int& uiID)
{
    m_ID = uiID;
}

int Property::GetValue() const
{
    return m_Value;
}

void Property::SetValue(const int& iValue)
{
    m_Value = iValue;
}

PropertyVariant::PropertyVariant()
{
}

PropertyVariant::~PropertyVariant()
{
}

unsigned int PropertyVariant::GetID() const
{
    return m_ID;
}

void PropertyVariant::SetID(const unsigned int& uiID)
{
    m_ID = uiID;
}

Variant PropertyVariant::GetValue() const
{
    return m_Value;
}

void PropertyVariant::SetValue(const Variant& value)
{
    m_Value = value;
}

}
}
