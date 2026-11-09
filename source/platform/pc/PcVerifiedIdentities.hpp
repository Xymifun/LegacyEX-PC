#ifndef PLATFORM_PC_PCVERIFIEDIDENTITIES_HPP
#define PLATFORM_PC_PCVERIFIEDIDENTITIES_HPP

#include <string.h>

struct PcVerifiedIdentities
{
    int count;
    unsigned char uuids[4][16];

    PcVerifiedIdentities() : count(0) { memset(uuids, 0, sizeof(uuids)); }

    bool Contains(const unsigned char uuid[16]) const
    {
        for (int index = 0; index < count; ++index)
        {
            if (memcmp(uuids[index], uuid, 16) == 0)
                return true;
        }
        return false;
    }
};

#endif
