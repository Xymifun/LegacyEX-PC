#include "game/core/XmlConstruct.hpp"
#include "game/core/StringUtils.hpp"

template <typename T> T _fromString(const std::wstring& text);

std::wstring XmlConstruct::asString() const
{
    return L"";
}

std::wstring XmlConstruct::asString(const std::wstring& defaultValue) const
{
    return isString() ? asString() : defaultValue;
}

int XmlConstruct::asInt() const
{
    return _fromString<int>(asString());
}

int XmlConstruct::asInt(int defaultValue) const
{
    if (isInt())
        return asInt();

    return defaultValue;
}

unsigned int XmlConstruct::asUInt() const
{
    return _fromString<unsigned int>(asString());
}

unsigned int XmlConstruct::asUInt(unsigned int defaultValue) const
{
    if (isUInt())
        return asUInt();

    return defaultValue;
}

bool XmlConstruct::asBool() const
{
    if (!isString())
        return false;

    std::wstring value = asString();
    if (equalsIgnoreCase(value, L"true"))
    {
        return true;
    }
    else if (equalsIgnoreCase(value, L"false"))
    {
        return false;
    }
    else
    {
        return false;
    }
}

bool XmlConstruct::asBool(bool defaultValue) const
{
    return isBool() ? asBool() : defaultValue;
}

double XmlConstruct::asDouble() const
{
    return _fromString<double>(asString());
}

double XmlConstruct::asDouble(double defaultValue) const
{
    if (isDouble())
        return asDouble();

    return defaultValue;
}

float XmlConstruct::asFloat() const
{
    return _fromString<float>(asString());
}

float XmlConstruct::asFloat(float defaultValue) const
{
    if (isFloat())
        return asFloat();

    return defaultValue;
}

bool XmlConstruct::isString() const
{
    return true;
}

bool XmlConstruct::isInt() const
{
    return isString();
}

bool XmlConstruct::isUInt() const
{
    return isString();
}

bool XmlConstruct::isBool() const
{
    return isString();
}

bool XmlConstruct::isDouble() const
{
    return isString();
}

bool XmlConstruct::isFloat() const
{
    return isString();
}

maybe<std::wstring> XmlConstruct::maybeString() const
{
    return isString() ? maybe<std::wstring>(asString()) : maybe<std::wstring>();
}

maybe<int> XmlConstruct::maybeInt() const
{
    return isInt() ? maybe<int>(asInt()) : maybe<int>();
}

maybe<unsigned int> XmlConstruct::maybeUInt() const
{
    return isUInt() ? maybe<unsigned int>(asUInt()) : maybe<unsigned int>();
}

maybe<bool> XmlConstruct::maybeBool() const
{
    return isBool() ? maybe<bool>(asBool()) : maybe<bool>();
}

maybe<double> XmlConstruct::maybeDouble() const
{
    return isDouble() ? maybe<double>(asDouble()) : maybe<double>();
}

maybe<float> XmlConstruct::maybeFloat() const
{
    return isFloat() ? maybe<float>(asFloat()) : maybe<float>();
}
