#include "nn/nex/Variant.hpp"

#include <cwchar>

#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

Variant::Variant() : m_type(TYPE_NONE)
{
}

Variant::Variant(const Variant& other) : m_type(TYPE_NONE)
{
    *this = other;
}

Variant::Variant(long long value) : m_integer(value), m_type(TYPE_INTEGER)
{
}

Variant::Variant(unsigned long long value) : m_unsignedInteger(value), m_type(TYPE_UNSIGNED_INTEGER)
{
}

Variant::Variant(int value) : m_integer(value), m_type(TYPE_INTEGER)
{
}

Variant::Variant(unsigned int value) : m_unsignedInteger(value), m_type(TYPE_UNSIGNED_INTEGER)
{
}

Variant::Variant(double value) : m_double(value), m_type(TYPE_DOUBLE)
{
}

Variant::Variant(bool value) : m_bool(value), m_type(TYPE_BOOL)
{
}

Variant::Variant(const String& value)
{
    CopyString(&m_text, static_cast<const wchar_t*>(value));
    m_type = TYPE_STRING;
}

Variant::Variant(const DateTime& value)
{
    m_unsignedInteger = value;
    m_type = TYPE_DATE_TIME;
}

Variant::Variant(const wchar_t* value)
{
    CopyString(&m_text, value);
    m_type = TYPE_STRING;
}

Variant::~Variant()
{
    if (m_type == TYPE_STRING)
        SpecialDeleteArray<wchar_t>(m_text);
}

Variant& Variant::operator=(const Variant& other)
{
    if (m_type == TYPE_STRING)
        SpecialDeleteArray<wchar_t>(m_text);

    m_type = other.GetType();

    switch (other.GetType())
    {
    case TYPE_STRING:
    {
        String text = other.GetStringValue();
        CopyString(&m_text, static_cast<const wchar_t*>(text));
    }
    break;
    case TYPE_INTEGER:
        m_integer = other.GetInt64Value();
        break;
    case TYPE_UNSIGNED_INTEGER:
        m_unsignedInteger = other.GetUInt64Value();
        break;
    case TYPE_DOUBLE:
        m_double = other.GetDoubleValue();
        break;
    case TYPE_BOOL:
        m_bool = other.GetBoolValue();
        break;
    case TYPE_DATE_TIME:
        m_unsignedInteger = other.m_unsignedInteger;
        break;
    default:
        break;
    }

    return *this;
}

unsigned int Variant::GetType() const
{
    return m_type;
}

String Variant::GetStringValue() const
{
    const wchar_t* text = L"";

    if (GetType() == TYPE_STRING)
        text = m_text;

    return String(text);
}

long long Variant::GetInt64Value() const
{
    unsigned int type = GetType();

    if (type != TYPE_INTEGER && type != TYPE_UNSIGNED_INTEGER)
        return 0;

    return m_integer;
}

unsigned long long Variant::GetUInt64Value() const
{
    unsigned int type = GetType();

    if (type != TYPE_INTEGER && type != TYPE_UNSIGNED_INTEGER)
        return 0;

    return m_unsignedInteger;
}

double Variant::GetDoubleValue() const
{
    if (GetType() != TYPE_DOUBLE)
        return 0.0;

    return m_double;
}

bool Variant::GetBoolValue() const
{
    bool value = false;

    if (GetType() == TYPE_BOOL)
        value = m_bool;

    return value;
}

int Variant::GetInt32Value() const
{
    unsigned int type = GetType();

    if (type != TYPE_INTEGER && type != TYPE_UNSIGNED_INTEGER)
        return 0;

    return static_cast<int>(m_integer);
}

unsigned int Variant::GetUInt32Value() const
{
    unsigned int type = GetType();

    if (type != TYPE_INTEGER && type != TYPE_UNSIGNED_INTEGER)
        return 0;

    return static_cast<unsigned int>(m_unsignedInteger);
}

DateTime Variant::GetDateTimeValue() const
{
    if (GetType() != TYPE_DATE_TIME)
        return DateTime();

    return DateTime(static_cast<uint64_t>(m_unsignedInteger));
}

StringStream& operator<<(StringStream& rStream, const DateTime& rDateTime)
{
    wchar_t buffer[24];
    std::swprintf(buffer, 24, L"%04d-%02d-%02d %02d:%02d:%02d", rDateTime.GetYear(), rDateTime.GetMonth(),
                  rDateTime.GetDay(), rDateTime.GetHour(), rDateTime.GetMinute(), rDateTime.GetSecond());
    rStream << buffer;
    return rStream;
}

void Variant::Trace(unsigned int) const
{
}

}
}
