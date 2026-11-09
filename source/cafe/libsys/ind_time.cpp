#include <stddef.h>
#include <time.h>

#include <coreinit/time.h>

enum TimeConstant
{
    EPOCH_YEAR = 1970,
    DAYS_IN_COMMON_YEAR = 365,
    SECS_IN_MIN = 60,
    SECS_IN_HOUR = 60 * SECS_IN_MIN,
    SECS_IN_DAY = 24 * SECS_IN_HOUR
};

static int DaysBeforeMonth[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
static int LeapDaysBeforeMonth[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

extern "C"
{

int IsLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int GetYearDays(int year, int month)
{
    int* daysBeforeMonth = DaysBeforeMonth;
    if (IsLeapYear(year))
        daysBeforeMonth = LeapDaysBeforeMonth;
    return daysBeforeMonth[month];
}

int GetLeapDays(int year)
{
    return (year + 3) / 4 - (year - 1) / 100 + (year - 1) / 400;
}

time_t time(time_t* tptr)
{
    OSCalendarTime calendarTime;
    long long epochDays;
    long long days;
    long long seconds;

    OSTicksToCalendarTime(OSGetTime(), &calendarTime);
    epochDays = EPOCH_YEAR * DAYS_IN_COMMON_YEAR + GetLeapDays(EPOCH_YEAR);
    days = calendarTime.tm_year * DAYS_IN_COMMON_YEAR + GetLeapDays(calendarTime.tm_year)
           + GetYearDays(calendarTime.tm_year, calendarTime.tm_mon) + calendarTime.tm_mday - 1;
    seconds = (days - epochDays) * SECS_IN_DAY + (long long)calendarTime.tm_hour * SECS_IN_HOUR
              + (long long)calendarTime.tm_min * SECS_IN_MIN + calendarTime.tm_sec;
    if (tptr != NULL)
        *tptr = seconds;
    return seconds;
}
}
