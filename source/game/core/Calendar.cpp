#include "game/core/Calendar.hpp"

#include <time.h>

int Calendar::GetDayOfMonth()
{
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    return local->tm_mday;
}

int Calendar::GetMonth()
{
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    return local->tm_mon + 1;
}
