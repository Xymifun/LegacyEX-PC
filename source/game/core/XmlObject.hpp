#ifndef GAME_CORE_XMLOBJECT_HPP
#define GAME_CORE_XMLOBJECT_HPP

#include <string>
#include <utility>
#include <vector>

#include "game/core/XmlConstruct.hpp"

class XmlChild;
class XmlValue;
class WstringLookup;

class XmlObject : public XmlConstruct
{
public:
    explicit XmlObject(const std::wstring& name);
    virtual ~XmlObject();
    XmlObject* child(const std::wstring& name);
    const XmlObject* child(const std::wstring& name) const;
    void addChildRaw(const XmlChild& child);
    void addAttr(const std::wstring& name, const std::wstring& value);
    int size() const;
    const XmlChild& operator[](int index) const;
    XmlValue operator[](const std::wstring& name) const;
    using XmlConstruct::asString;
    virtual std::wstring asString() const;
    virtual bool isString() const;
    std::wstring toString(int indent, bool formatted) const;
    void addChild(XmlObject* child);
    const std::wstring& getAttr(const std::wstring& name) const;
    std::wstring getText() const;
    WstringLookup* _getLookup(WstringLookup* lookup);
    WstringLookup* getLookup();
    std::vector<XmlChild>& getChildren();
    void _getValues(const std::wstring& name, const std::wstring& value, std::vector<std::wstring>* values);
    std::vector<std::wstring> getAllValues(const std::wstring& name);
    std::vector<std::wstring> getValues(const std::wstring& name, const std::wstring& value);
    std::vector<XmlChild>& getChildren(const std::wstring& name);
    const std::vector<XmlChild>& getChildrenRaw() const { return m_children; }
    std::vector<std::pair<std::wstring, std::wstring> >& getAttrs() { return m_attrs; }

    std::wstring mName;

private:
    std::vector<XmlChild> m_children;
    std::vector<std::pair<std::wstring, std::wstring> > m_attrs;
};

extern XmlObject gXmlObjectNull;

#endif
