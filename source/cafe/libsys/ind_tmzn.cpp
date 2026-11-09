#include <stddef.h>
#include <time.h>

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

static long timezone;

extern "C"
{

void* __ghs_GetThreadLocalStorageItem(int specifier);
extern struct tm __ghs_static_gmtime_temp;

int __gh_timezone(void)
{
    return timezone / 60;
}

struct tm* localtime_r(const time_t* timer, struct tm* result)
{
    time_t time_v = *timer - 60 * __gh_timezone();
    struct tm* temp;
    int isdst, sunday_mday, month, after, day;
    static const char mons[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (*timer == (time_t)-1)
        return NULL;
    if ((temp = gmtime_r(&time_v, result)) == NULL)
        return NULL;
    if (temp->tm_year < 66)
        return temp;
    {
        static const struct s_chart
        {
            char year;
            char start_month;
            char start_day;
            char end_month;
            char end_day;
        } chart[] = {
            {107, 2, 14, 10, 7},
            {87, 3, 7, 9, 31},
            {0, 3, 30, 9, 31},
        };
        const struct s_chart* p;

        for (p = chart; p->year > temp->tm_year; p++)
            ;
        isdst = 1;
        after = 1;
        day = p->start_day;
        month = temp->tm_mon;
        if (month == p->start_month)
        {
            isdst = 0;
        }
        else if (month == p->end_month)
        {
            after = 0;
            day = p->end_day;
        }
        else if (month < p->start_month || month > p->end_month)
        {
            return temp;
        }
    }

    sunday_mday = (temp->tm_mday - temp->tm_wday);
    if (sunday_mday <= day - 7)
        ;
    else if (sunday_mday > day)
        isdst = after;
    else if (temp->tm_wday)
        isdst = after;
    else if (temp->tm_hour >= 2)
        isdst = after;

    temp->tm_isdst = isdst;
    if (isdst)
    {
        if (++(temp->tm_hour) == 24)
        {
            temp->tm_hour = 0;
            ++(temp->tm_yday);
            if (++(temp->tm_wday) == 7)
                temp->tm_wday = 0;
            if (++(temp->tm_mday) > mons[month])
            {
                temp->tm_mday = 1;
                ++(temp->tm_mon);
            }
        }
    }
    return temp;
}

struct tm* localtime(const time_t* timer)
{
    struct tm* temp = static_cast<struct tm*>(__ghs_GetThreadLocalStorageItem(__ghs_TLS_gmtime_temp));
    if (!temp)
        temp = &__ghs_static_gmtime_temp;
    return localtime_r(timer, temp);
}
}
