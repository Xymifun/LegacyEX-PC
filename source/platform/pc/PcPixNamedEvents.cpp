#include "game/core/PIXNamedEvents.hpp"

#include "platform/pc/PcProfiler.hpp"

unsigned int XPartyGetUserList(_XPARTY_USER_LIST*)
{
    return 0;
}

unsigned int XBackgroundDownloadSetMode(_XBACKGROUND_DOWNLOAD_MODE)
{
    return 0;
}

void PIXAddNamedCounter(float, const char*, ...)
{
}

void PIXBeginNamedEvent(float, const char* name, ...)
{
    if (PcProfilerEnabled())
        PcProfilerBegin(name);
}

void PIXEndNamedEvent()
{
    if (PcProfilerEnabled())
        PcProfilerEnd();
}

void PIXSetMarkerDeprecated(float, const char*, ...)
{
}

int XUserGetSigninInfo(unsigned int, unsigned int, _XUSER_SIGNIN_INFO*)
{
    return 0;
}
