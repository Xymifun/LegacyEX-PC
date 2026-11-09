#include "game/core/PIXNamedEvents.hpp"

#include <cstdarg>

unsigned int XPartyGetUserList(_XPARTY_USER_LIST* users)
{
    return 0;
}

unsigned int XBackgroundDownloadSetMode(_XBACKGROUND_DOWNLOAD_MODE mode)
{
    return 0;
}

void PIXAddNamedCounter(float colour, const char* name, ...)
{
    va_list args;
    va_start(args, name);
    (void)colour;
    va_end(args);
}

void PIXBeginNamedEvent(float colour, const char* name, ...)
{
    va_list args;
    va_start(args, name);
    (void)colour;
    va_end(args);
}

void PIXEndNamedEvent()
{
}

void PIXSetMarkerDeprecated(float colour, const char* name, ...)
{
}

int XUserGetSigninInfo(unsigned int, unsigned int, _XUSER_SIGNIN_INFO*)
{
    return 0;
}
