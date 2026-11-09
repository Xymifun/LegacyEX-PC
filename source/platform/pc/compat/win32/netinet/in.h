#ifndef PLATFORM_PC_COMPAT_WIN32_NETINET_IN_H
#define PLATFORM_PC_COMPAT_WIN32_NETINET_IN_H

#include <stdint.h>

typedef uint16_t sa_family_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr
{
    in_addr_t s_addr;
};

struct sockaddr_in
{
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};

#define AF_INET 2
#define INADDR_ANY ((in_addr_t)0x00000000)
#define INADDR_BROADCAST ((in_addr_t)0xffffffff)

#endif
