#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGICALCLOCKTMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGICALCLOCKTMPL_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename ValueType, typename DifferenceType>
class LogicalClockTmpl : public RootObject
{
public:
    LogicalClockTmpl() : m_value(0) {}

    LogicalClockTmpl(ValueType value) : m_value(value) {}

    virtual ~LogicalClockTmpl() {}

    ValueType GetValue() const { return m_value; }

    LogicalClockTmpl& operator++()
    {
        ++m_value;
        return *this;
    }

    DifferenceType operator-(const LogicalClockTmpl& other) const
    {
        return static_cast<DifferenceType>(m_value - other.m_value);
    }

    bool operator==(const LogicalClockTmpl& other) const { return m_value == other.m_value; }

    bool operator!=(const LogicalClockTmpl& other) const { return m_value != other.m_value; }

    bool operator>(const LogicalClockTmpl& other) const { return *this - other > 0; }

    bool operator>=(const LogicalClockTmpl& other) const { return *this == other || *this > other; }

    bool operator<(const LogicalClockTmpl& other) const { return other - *this > 0; }

private:
    ValueType m_value;
};

}
}

#endif
