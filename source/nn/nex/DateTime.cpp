#include "nn/nex/DateTime.hpp"

#include <coreinit/time.h>

namespace nn
{
namespace nex
{
namespace
{

enum
{
    SECOND_SHIFT = 0,
    MINUTE_SHIFT = 6,
    HOUR_SHIFT = 12,
    DAY_SHIFT = 17,
    MONTH_SHIFT = 22,
    YEAR_SHIFT = 26,
    STATUS_SHIFT = 40
};

enum
{
    SECOND_MASK = 0x3f,
    MINUTE_MASK = 0x3f,
    HOUR_MASK = 0x1f,
    DAY_MASK = 0x1f,
    MONTH_MASK = 0xf,
    YEAR_MASK = 0x3fff,
    STATUS_MASK = 3
};

const unsigned long long SECOND_FIELD = static_cast<unsigned long long>(SECOND_MASK) << SECOND_SHIFT;
const unsigned long long MINUTE_FIELD = static_cast<unsigned long long>(MINUTE_MASK) << MINUTE_SHIFT;
const unsigned long long HOUR_FIELD = static_cast<unsigned long long>(HOUR_MASK) << HOUR_SHIFT;
const unsigned long long DAY_FIELD = static_cast<unsigned long long>(DAY_MASK) << DAY_SHIFT;
const unsigned long long MONTH_FIELD = static_cast<unsigned long long>(MONTH_MASK) << MONTH_SHIFT;
const unsigned long long YEAR_FIELD = static_cast<unsigned long long>(YEAR_MASK) << YEAR_SHIFT;
const unsigned long long STATUS_PRE_EPOCH = 1ULL << STATUS_SHIFT;
const unsigned long long STATUS_BITS = static_cast<unsigned long long>(STATUS_MASK) << STATUS_SHIFT;

}

const DateTime& DateTime::s_never = DateTime();
const DateTime& DateTime::s_future = DateTime(9999, 12, 31, 23, 59, 59);

DateTime::DateTime() : m_value(0)
{
}

DateTime::DateTime(const DateTime& other) : m_value(other.m_value)
{
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) : m_value(0)
{
    m_value = (m_value & ~YEAR_FIELD) | ((static_cast<unsigned long long>(year) << YEAR_SHIFT) & YEAR_FIELD);
    m_value = (m_value & ~MONTH_FIELD) | ((static_cast<unsigned long long>(month) & MONTH_MASK) << MONTH_SHIFT);
    m_value = (m_value & ~DAY_FIELD) | ((static_cast<unsigned long long>(day) & DAY_MASK) << DAY_SHIFT);
    m_value = (m_value & ~HOUR_FIELD) | ((static_cast<unsigned long long>(hour) & HOUR_MASK) << HOUR_SHIFT);
    m_value = (m_value & ~MINUTE_FIELD) | ((static_cast<unsigned long long>(minute) & MINUTE_MASK) << MINUTE_SHIFT);
    m_value = (m_value & ~SECOND_FIELD) | ((static_cast<unsigned long long>(second) & SECOND_MASK) << SECOND_SHIFT);

    bool yearOutOfRange = false;

    if (year < 1900)
        m_value = (m_value & ~STATUS_BITS) | STATUS_PRE_EPOCH;
    else if (year > 9999)
        yearOutOfRange = true;

    if (yearOutOfRange || static_cast<uint32_t>(month) - 1 >= 12 || static_cast<uint32_t>(day) - 1 >= 31)
    {
        m_value = (m_value & ~STATUS_BITS) | STATUS_PRE_EPOCH;
        *this = s_never;
    }
}

DateTime& DateTime::operator=(const DateTime& other)
{
    m_value = other.m_value;
    return *this;
}

DateTime::operator unsigned long long()
{
    return m_value;
}

DateTime::operator unsigned long long() const
{
    return m_value;
}

bool DateTime::operator==(const DateTime& other) const
{
    return m_value == other.m_value;
}

bool DateTime::operator!=(const DateTime& other) const
{
    return m_value != other.m_value;
}

bool DateTime::operator<(const DateTime& other) const
{
    return m_value < other.m_value;
}

bool DateTime::operator>(const DateTime& other) const
{
    return m_value > other.m_value;
}

bool DateTime::operator<=(const DateTime& other) const
{
    return m_value <= other.m_value;
}

bool DateTime::operator>=(const DateTime& other) const
{
    return m_value >= other.m_value;
}

int64_t DateTime::operator-(const DateTime& other) const
{
    int64_t difference = ToEpochTime() - other.ToEpochTime();

    return difference * 1000;
}

uint16_t DateTime::GetYear() const
{
    return static_cast<uint16_t>((m_value & YEAR_FIELD) >> YEAR_SHIFT);
}

uint8_t DateTime::GetMonth() const
{
    return static_cast<uint8_t>((m_value & MONTH_FIELD) >> MONTH_SHIFT);
}

uint8_t DateTime::GetDay() const
{
    return static_cast<uint8_t>((m_value & DAY_FIELD) >> DAY_SHIFT);
}

uint8_t DateTime::GetHour() const
{
    return static_cast<uint8_t>((m_value & HOUR_FIELD) >> HOUR_SHIFT);
}

uint8_t DateTime::GetMinute() const
{
    return static_cast<uint8_t>((m_value & MINUTE_FIELD) >> MINUTE_SHIFT);
}

uint8_t DateTime::GetSecond() const
{
    return static_cast<uint8_t>((m_value & SECOND_FIELD) >> SECOND_SHIFT);
}

uint32_t DateTime::IsLeapYear(int32_t year) const
{
    if (year % 400 == 0)
        return 1;

    if (year % 100 == 0)
        return 0;

    if ((year & 3) == 0)
        return 1;

    return 0;
}

int32_t DateTime::DateToDays(int32_t year, int32_t month, int32_t day) const
{
    uint32_t leapAdjustment;

    day -= 1;
    year -= 2000;

    if (month <= 2)
    {
        month += 9;
        year -= 1;
    }
    else
    {
        month -= 3;
    }

    leapAdjustment = 1;
    if (year < 0)
        leapAdjustment = IsLeapYear(year);

    return ((year / 100) * 146097) / 4 + ((year % 100) * 1461) / 4 + (month * 153 + 2) / 5 + day + 59 + leapAdjustment;
}

void DateTime::DaysToDate(int32_t days)
{
    int32_t day = days + 2520114;
    int32_t century = (day * 4) / 146097;
    day -= (century * 146097 + 3) / 4;
    int32_t yearOfCentury = (day * 4000 + 4000) / 1461001;
    day = day - (yearOfCentury * 1461) / 4 + 31;
    int32_t monthIndex = (day * 80) / 2447;
    day -= (monthIndex * 2447) / 80;
    int32_t month = monthIndex + 2 - (monthIndex / 11) * 12;
    int32_t year = century * 100 - 4900 + yearOfCentury + monthIndex / 11;

    m_value = (m_value & ~YEAR_FIELD) | ((static_cast<unsigned long long>(year) << YEAR_SHIFT) & YEAR_FIELD);
    m_value = (m_value & ~MONTH_FIELD) | ((static_cast<unsigned long long>(month) & MONTH_MASK) << MONTH_SHIFT);
    m_value = (m_value & ~DAY_FIELD) | ((static_cast<unsigned long long>(day) & DAY_MASK) << DAY_SHIFT);
}

uint64_t DateTime::ToEpochTime() const
{
    return GetSecond() + GetMinute() * 60ULL + GetHour() * 3600ULL
           + DateToDays(GetYear(), GetMonth(), GetDay()) * 86400LL;
}

uint32_t DateTime::IsValid() const
{
    return ((m_value >> STATUS_SHIFT) & STATUS_MASK) == 0;
}

bool DateTime::IsNever() const
{
    unsigned long long value = m_value & ~STATUS_BITS;

    return value == s_never;
}

void DateTime::Trace(uint64_t)
{
}

void DateTime::GetSystemTime(DateTime& dateTime)
{
    OSCalendarTime calendarTime;

    OSTicksToCalendarTime(OSGetTime(), &calendarTime);

    dateTime.m_value = (dateTime.m_value & ~YEAR_FIELD)
                       | ((static_cast<unsigned long long>(calendarTime.tm_year) << YEAR_SHIFT) & YEAR_FIELD);
    dateTime.m_value = (dateTime.m_value & ~MONTH_FIELD)
                       | (((static_cast<unsigned long long>(calendarTime.tm_mon) + 1) & MONTH_MASK) << MONTH_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~DAY_FIELD)
                       | ((static_cast<unsigned long long>(calendarTime.tm_mday) & DAY_MASK) << DAY_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~HOUR_FIELD)
                       | ((static_cast<unsigned long long>(calendarTime.tm_hour) & HOUR_MASK) << HOUR_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~MINUTE_FIELD)
                       | ((static_cast<unsigned long long>(calendarTime.tm_min) & MINUTE_MASK) << MINUTE_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~SECOND_FIELD)
                       | ((static_cast<unsigned long long>(calendarTime.tm_sec) & SECOND_MASK) << SECOND_SHIFT);
}

DateTime DateTime::FromCustomEpochTime(int64_t epochTime, int32_t dayOffset)
{
    DateTime dateTime;

    dateTime.DaysToDate(static_cast<int32_t>(epochTime / 86400) + dayOffset);
    dateTime.m_value = (dateTime.m_value & ~HOUR_FIELD)
                       | (((static_cast<uint64_t>(epochTime) / 3600) % 24 & HOUR_MASK) << HOUR_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~MINUTE_FIELD)
                       | (((static_cast<uint64_t>(epochTime) / 60) % 60 & MINUTE_MASK) << MINUTE_SHIFT);
    dateTime.m_value = (dateTime.m_value & ~SECOND_FIELD)
                       | ((static_cast<uint64_t>(epochTime) % 60 & SECOND_MASK) << SECOND_SHIFT);

    return dateTime;
}

DateTime DateTime::FromEpochTime(int64_t epochTime)
{
    return FromCustomEpochTime(epochTime, 0);
}

}
}
