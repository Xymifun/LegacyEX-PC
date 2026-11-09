#ifndef GAME_CORE_PIXNAMEDEVENTS_HPP
#define GAME_CORE_PIXNAMEDEVENTS_HPP

#include "vendor/_XPARTY_USER_INFO.hpp"
#include "vendor/_XUSER_SIGNIN_INFO.hpp"

enum _XBACKGROUND_DOWNLOAD_MODE
{
    XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW = 1,
    XBACKGROUND_DOWNLOAD_MODE_AUTO = 2
};

struct _XPARTY_USER_LIST
{
    unsigned int count;
    _XPARTY_USER_INFO users[4];
};

unsigned int XPartyGetUserList(_XPARTY_USER_LIST* users);
unsigned int XBackgroundDownloadSetMode(_XBACKGROUND_DOWNLOAD_MODE mode);
int XUserGetSigninInfo(unsigned int userIndex, unsigned int flags, _XUSER_SIGNIN_INFO* info);
void PIXAddNamedCounter(float colour, const char* name, ...);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void PIXSetMarkerDeprecated(float colour, const char* name, ...);

#endif
