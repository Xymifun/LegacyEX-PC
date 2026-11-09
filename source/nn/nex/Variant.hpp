#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VARIANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VARIANT_HPP

#include "nn/nex/DateTime.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Variant : public RootObject
{
public:
    enum Type
    {
        TYPE_NONE = 0,
        TYPE_INTEGER = 1,
        TYPE_DOUBLE = 2,
        TYPE_BOOL = 3,
        TYPE_STRING = 4,
        TYPE_DATE_TIME = 5,
        TYPE_UNSIGNED_INTEGER = 6
    };

    Variant();
    Variant(const Variant& other);
    Variant(long long value);
    Variant(unsigned long long value);
    Variant(int value);
    Variant(unsigned int value);
    Variant(double value);
    Variant(bool value);
    Variant(const String& value);
    Variant(const DateTime& value);
    Variant(const wchar_t* value);
    ~Variant();

    Variant& operator=(const Variant& other);

    unsigned int GetType() const;
    String GetStringValue() const;
    long long GetInt64Value() const;
    unsigned long long GetUInt64Value() const;
    double GetDoubleValue() const;
    bool GetBoolValue() const;
    int GetInt32Value() const;
    unsigned int GetUInt32Value() const;
    DateTime GetDateTimeValue() const;

    void Trace(unsigned int flags) const;

private:
    union
    {
        long long m_integer;
        unsigned long long m_unsignedInteger;
        double m_double;
        bool m_bool;
        wchar_t* m_text;
    };

    unsigned int m_type;
};

}
}

#endif
