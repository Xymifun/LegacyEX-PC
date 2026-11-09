#ifndef PLATFORM_PC_COMPAT_WIN32_SYS_SOCKET_H
#define PLATFORM_PC_COMPAT_WIN32_SYS_SOCKET_H

#include <stdint.h>

typedef uint32_t socklen_t;

struct sockaddr
{
    uint16_t sa_family;
    char sa_data[14];
};

#endif
