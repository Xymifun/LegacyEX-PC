#ifndef GAME_CORE_XMLCONSTRUCT_HPP
#define GAME_CORE_XMLCONSTRUCT_HPP

#include <string>

#include "game/core/maybe.hpp"

class XmlConstruct
{
public:
    virtual ~XmlConstruct() {}

    virtual std::wstring asString() const;
    virtual std::wstring asString(const std::wstring& defaultValue) const;
    virtual int asInt() const;
    virtual int asInt(int defaultValue) const;
    virtual unsigned int asUInt() const;
    virtual unsigned int asUInt(unsigned int defaultValue) const;
    virtual bool asBool() const;
    virtual bool asBool(bool defaultValue) const;
    virtual double asDouble() const;
    virtual double asDouble(double defaultValue) const;
    virtual float asFloat() const;
    virtual float asFloat(float defaultValue) const;
    virtual bool isString() const;
    virtual bool isInt() const;
    virtual bool isUInt() const;
    virtual bool isBool() const;
    virtual bool isDouble() const;
    virtual bool isFloat() const;

    maybe<std::wstring> maybeString() const;
    maybe<int> maybeInt() const;
    maybe<unsigned int> maybeUInt() const;
    maybe<bool> maybeBool() const;
    maybe<double> maybeDouble() const;
    maybe<float> maybeFloat() const;
};

#endif
