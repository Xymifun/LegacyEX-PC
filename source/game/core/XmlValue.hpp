#ifndef GAME_CORE_XMLVALUE_HPP
#define GAME_CORE_XMLVALUE_HPP

#include <string>

#include "game/core/XmlConstruct.hpp"
#include "game/core/maybe.hpp"

class XmlValue : public XmlConstruct
{
public:
    XmlValue(const maybe<std::wstring>& value) : m_value(value) {}

    virtual ~XmlValue() {}
    virtual bool isString() const;
    virtual std::wstring asString() const;

private:
    maybe<std::wstring> m_value;
};

#endif
