#include <limits.h>
#include <stddef.h>
#include <time.h>

static const char mons[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};

#define SECS_IN_MIN ((time_t)60)
#define MINS_IN_HOUR ((time_t)60)
#define SECS_IN_HOUR (MINS_IN_HOUR * SECS_IN_MIN)
#define HOURS_IN_DAY ((time_t)24)
#define SECS_IN_DAY (HOURS_IN_DAY * SECS_IN_HOUR)
#define DAYS_IN_WEEK ((time_t)7)

#define DAYS_IN_COMMON_YEAR ((time_t)365)
#define DAYS_IN_400_YEARS (DAYS_IN_COMMON_YEAR * 400 + 97)

enum __ghs_ThreadLocalStorage_specifier
{
    __ghs_TLS_asctime_buff,
    __ghs_TLS_tmpnam_space,
    __ghs_TLS_strtok_saved_pos,
    __ghs_TLS_Errno,
    __ghs_TLS_gmtime_temp,
    __ghs_TLS___eh_globals,
    __ghs_TLS_SignalHandlers,
    __ghs_TLS__eh_mem_manage,
    __ghs_TLS__eh_store_globals,
    __ghs_TLS__eh_store_globals_tdeh,
    __ghs_TLS__eh_init_block
};

extern "C"
{

void* __ghs_GetThreadLocalStorageItem(int specifier);

struct tm* gmtime_r(const time_t* timer, struct tm* result)
{
    time_t time_v = *timer;
    int i, m, islpyr;
    time_t y, t, s;

    if (time_v == (time_t)-1)
        return NULL;

    result->tm_isdst = 0;
    t = time_v / SECS_IN_DAY;
    s = time_v % SECS_IN_DAY;
    if (s < 0)
    {
        s += SECS_IN_DAY;
        t--;
    }

    result->tm_hour = s / SECS_IN_HOUR;
    s %= SECS_IN_HOUR;
    result->tm_min = s / SECS_IN_MIN;
    result->tm_sec = s % SECS_IN_MIN;

    s = (t + 4) % DAYS_IN_WEEK;
    if (s < 0)
    {
        s += DAYS_IN_WEEK;
    }
    result->tm_wday = s;

    if (DAYS_IN_COMMON_YEAR * 69 > LONG_MAX / SECS_IN_DAY)
    {
        t += DAYS_IN_COMMON_YEAR * 70 + 18;

        y = t / (DAYS_IN_COMMON_YEAR * 4 + 1) * 4;
        t = t % (DAYS_IN_COMMON_YEAR * 4 + 1);
        if (t >= DAYS_IN_COMMON_YEAR + 1)
        {
            y += (t - 1) / DAYS_IN_COMMON_YEAR;
            t = (t - 1) % DAYS_IN_COMMON_YEAR;
        }
        islpyr = (y & 3) == 0;
    }
    else
    {
        time_t century;
        t -= DAYS_IN_COMMON_YEAR * 30 + 7;

        century = t / DAYS_IN_400_YEARS * 4;
        t %= DAYS_IN_400_YEARS;
        if (t < 0)
        {
            t += DAYS_IN_400_YEARS;
            century -= 4;
        }

        if (t >= DAYS_IN_COMMON_YEAR * 100 + 25)
        {
            century += (t - 1) / (DAYS_IN_COMMON_YEAR * 100 + 24);
            t = (t - 1) % (DAYS_IN_COMMON_YEAR * 100 + 24);
            if (t >= DAYS_IN_COMMON_YEAR)
            {
                t++;
            }
        }

        y = t / (DAYS_IN_COMMON_YEAR * 4 + 1) * 4;
        t = t % (DAYS_IN_COMMON_YEAR * 4 + 1);
        if (t >= DAYS_IN_COMMON_YEAR + 1)
        {
            y += (t - 1) / DAYS_IN_COMMON_YEAR;
            t = (t - 1) % DAYS_IN_COMMON_YEAR;
        }

        islpyr = (y & 3) == 0 && (y != 0 || (century & 3) == 0);
        y += (century + 1) * 100;
        if (y != (int)y)
        {
            return NULL;
        }
    }
    result->tm_year = y;
    result->tm_yday = t;

    t++;
    for (i = 0; i < 11; i++)
    {
        m = mons[i];
        if (i == 1 && islpyr)
        {
            m++;
        }
        if (t <= m)
            break;
        t -= m;
    }

    result->tm_mon = i;
    result->tm_mday = t;
    return result;
}

struct tm __ghs_static_gmtime_temp;

struct tm* gmtime(const time_t* timer)
{
    struct tm* tmp = static_cast<struct tm*>(__ghs_GetThreadLocalStorageItem(__ghs_TLS_gmtime_temp));
    if (tmp == NULL)
        tmp = &__ghs_static_gmtime_temp;
    return gmtime_r(timer, tmp);
}
}
