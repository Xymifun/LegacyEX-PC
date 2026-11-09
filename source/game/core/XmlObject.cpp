#include "game/core/XmlObject.hpp"

#include "game/core/XmlChild.hpp"
#include "game/core/XmlValue.hpp"
#include "game/core/StringUtil.hpp"
#include "game/core/StringUtils.hpp"

#include <stdexcept>

XmlObject gXmlObjectNull(L"");

XmlObject::XmlObject(const std::wstring& name) : mName(name)
{
    m_children = std::vector<XmlChild>();
    m_attrs = std::vector<std::pair<std::wstring, std::wstring> >();
}

XmlObject::~XmlObject()
{
    for (std::vector<XmlChild>::iterator item = m_children.begin(); item != m_children.end(); ++item)
    {
        if (item->m_type == 1)
        {
            XmlObject* object = item->xml();
            if (object != 0)
                delete object;
        }
    }
}

XmlObject* XmlObject::child(const std::wstring& name)
{
    std::vector<XmlChild>::iterator item;
    for (item = m_children.begin(); item != m_children.end(); ++item)
        if (item->m_type == 1)
        {
            if (equalsIgnoreCase(item->xml()->mName, name) || name.empty())
                return item->xml();
        }
    return &gXmlObjectNull;
}

const XmlObject* XmlObject::child(const std::wstring& name) const
{
    std::vector<XmlChild>::const_iterator item;
    for (item = m_children.begin(); item != m_children.end(); ++item)
        if (item->m_type == 1)
        {
            if (equalsIgnoreCase(item->xml()->mName, name) || name.empty())
                return item->xml();
        }
    return &gXmlObjectNull;
}

void XmlObject::addChildRaw(const XmlChild& child)
{
    m_children.push_back(child);
}

void XmlObject::addAttr(const std::wstring& name, const std::wstring& value)
{
    m_attrs.push_back(std::pair<std::wstring, std::wstring>(name, value));
}

int XmlObject::size() const
{
    return static_cast<int>(m_children.size());
}

const XmlChild& XmlObject::operator[](int index) const
{
    return m_children.at(static_cast<std::size_t>(index));
}

XmlValue XmlObject::operator[](const std::wstring& name) const
{
    for (std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute = m_attrs.begin();
         attribute != m_attrs.end(); ++attribute)
        if (equalsIgnoreCase(name, attribute->first))
            return XmlValue(maybe<std::wstring>(std::wstring(attribute->second)));
    return XmlValue(maybe<std::wstring>());
}

std::wstring XmlObject::asString() const
{
    return (*this)[0].asString();
}

bool XmlObject::isString() const
{
    return size() != 0;
}

std::wstring XmlObject::toString(int indent, bool formatted) const
{
    std::wstring output(indent, L'\t');
    output += L"<";
    output += mName;
    for (std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute = m_attrs.begin();
         attribute != m_attrs.end(); ++attribute)
    {
        output += L" ";
        output += attribute->first;
        output += L"=\"";
        output += attribute->second;
        output += L"\"";
    }
    if (!m_children.empty())
    {
        output += L">";
        for (std::vector<XmlChild>::const_iterator item = m_children.begin(); item != m_children.end(); ++item)
        {
            if (item->m_type == 0 || item->m_type == 2)
            {
                if (!formatted && item->m_type == 2)
                {
                    output += L"<![CDATA[";
                    output += escape(item->txt());
                    output += L"]]>";
                }
                else
                {
                    output += escape(item->txt());
                }
            }
            else
            {
                output += L"\n";
                output += item->xml()->toString(indent + 1, false);
                output += L"\n";
            }
        }
        if (m_children.back().m_type == 1)
            output += std::wstring(indent, L'\t');
        output += L"</";
        output += mName;
        output += L">";
        if (m_children.back().m_type == 1)
            output += std::wstring(indent, L'\t');
    }
    else
    {
        output += L"/>\n";
    }
    return output;
}
