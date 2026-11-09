#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATETIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATETIME_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

struct OSCalendarTime;

namespace nn
{
namespace nex
{

class DateTime : public RootObject
{
public:
    DateTime();
    DateTime(const DateTime& other);
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    explicit DateTime(const OSCalendarTime& calendarTime);

    explicit DateTime(uint64_t value) : m_value(value) {}

    DateTime& operator=(const DateTime& other);

    operator unsigned long long();
    operator unsigned long long() const;

    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    int64_t operator-(const DateTime& other) const;

    uint16_t GetYear() const;
    uint8_t GetMonth() const;
    uint8_t GetDay() const;
    uint8_t GetHour() const;
    uint8_t GetMinute() const;
    uint8_t GetSecond() const;

    uint32_t IsLeapYear(int32_t year) const;
    int32_t DateToDays(int32_t year, int32_t month, int32_t day) const;
    void DaysToDate(int32_t days);

    uint64_t ToEpochTime() const;
    int64_t ToUnixEpochTime() const;
    OSCalendarTime ToCafeDateTime() const;

    uint32_t IsValid() const;
    bool IsNever() const;
    void Trace(uint64_t flags);

    static void GetSystemTime(DateTime& dateTime);
    static DateTime FromEpochTime(int64_t epochTime);
    static DateTime FromUnixEpochTime(int64_t epochTime);
    static DateTime FromCustomEpochTime(int64_t epochTime, int32_t dayOffset);

    static const DateTime& s_never;
    static const DateTime& s_future;

private:
    unsigned long long m_value;
};

class StringStream;
StringStream& operator<<(StringStream& rStream, const DateTime& rDateTime);

}
}

#endif
