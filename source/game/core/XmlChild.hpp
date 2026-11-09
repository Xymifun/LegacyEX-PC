#ifndef GAME_CORE_XMLCHILD_HPP
#define GAME_CORE_XMLCHILD_HPP

#include <string>

#include "game/core/XmlConstruct.hpp"

class XmlObject;

class XmlChild : public XmlConstruct
{
public:
    int m_type;

private:
    void* m_value;

public:
    XmlChild();
    XmlChild(XmlObject* value);
    XmlChild(const std::wstring& value);
    XmlChild(const std::wstring& value, bool isText);
    XmlChild(const XmlChild& other);
    virtual ~XmlChild() {}

    std::wstring txt() const;
    XmlObject* xml();
    XmlObject* xml() const;
    virtual std::wstring asString() const;
};

#endif
