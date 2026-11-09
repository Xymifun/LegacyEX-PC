#ifndef GAME_UTIL_ACTUUID_HPP
#define GAME_UTIL_ACTUUID_HPP

struct ACTUuid
{
    unsigned char data[16];

    bool operator==(const ACTUuid& other) const;
};

#endif
