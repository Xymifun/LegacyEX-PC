#include "game/core/XmlChild.hpp"
#include "game/core/XmlObject.hpp"

XmlChild::XmlChild() : m_type(3), m_value(0)
{
}

XmlChild::XmlChild(XmlObject* value) : m_type(1), m_value(value)
{
}

XmlChild::XmlChild(const std::wstring& value) : m_type(0), m_value(new std::wstring(value))
{
}

XmlChild::XmlChild(const std::wstring& value, bool isText) : m_type(isText ? 0 : 2), m_value(new std::wstring(value))
{
}

XmlChild::XmlChild(const XmlChild& other) : m_type(other.m_type), m_value(other.m_value)
{
    m_value = other.m_value;
}

std::wstring XmlChild::txt() const
{
    return *static_cast<const std::wstring*>(m_value);
}

XmlObject* XmlChild::xml()
{
    return static_cast<XmlObject*>(m_value);
}

XmlObject* XmlChild::xml() const
{
    return static_cast<XmlObject*>(m_value);
}

std::wstring XmlChild::asString() const
{
    switch (m_type)
    {
    case 0:
        return *static_cast<const std::wstring*>(m_value);
    case 1:
        return static_cast<const XmlObject*>(m_value)->asString();
    default:
        return L"";
    }
}
