#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_DATE_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_DATE_TIME_HPP

namespace nn
{
namespace pia
{
namespace common
{

class DateTime
{
public:
    DateTime() : m_year(0), m_month(0), m_day(0), m_hour(0), m_minute(0), m_second(0), m_isValid(false) {}

    DateTime(unsigned short year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute,
             unsigned char second)
        : m_year(year),
          m_month(month),
          m_day(day),
          m_hour(hour),
          m_minute(minute),
          m_second(second),
          m_isValid(true)
    {
    }

    void Clear()
    {
        m_year = 0;
        m_month = 0;
        m_day = 0;
        m_hour = 0;
        m_minute = 0;
        m_second = 0;
        m_isValid = false;
    }

    unsigned short GetYear() const { return m_year; }

    unsigned char GetMonth() const { return m_month; }

    unsigned char GetDay() const { return m_day; }

    unsigned char GetHour() const { return m_hour; }

    unsigned char GetMinute() const { return m_minute; }

    unsigned char GetSecond() const { return m_second; }

    bool IsValid() const { return m_isValid; }

    DateTime& operator=(const DateTime& other);

    virtual ~DateTime() {}

private:
    unsigned short m_year;
    unsigned char m_month;
    unsigned char m_day;
    unsigned char m_hour;
    unsigned char m_minute;
    unsigned char m_second;
    bool m_isValid;
};

inline DateTime& DateTime::operator=(const DateTime& other)
{
    m_year = other.m_year;
    m_month = other.m_month;
    m_day = other.m_day;
    m_hour = other.m_hour;
    m_minute = other.m_minute;
    m_second = other.m_second;
    m_isValid = other.m_isValid;
    return *this;
}

}
}
}

#endif
