#include "game/core/XmlValue.hpp"

bool XmlValue::isString() const
{
    return m_value.isSet();
}

std::wstring XmlValue::asString() const
{
    return std::wstring(m_value.get(), 0);
}
