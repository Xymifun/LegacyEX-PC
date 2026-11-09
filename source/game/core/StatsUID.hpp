#ifndef GAME_CORE_STATSUID_HPP
#define GAME_CORE_STATSUID_HPP

#include <string>

#include "game/world/generation/PlayerUID.hpp"

class INetworkPlayer;

class StatsUID : public PlayerUID
{
public:
    StatsUID() {}

    StatsUID(const PlayerUID& uid);

    static StatsUID EMPTY_SUID();
    static StatsUID NETWORKPLAYER_SUID(INetworkPlayer* networkPlayer);

    PlayerUID GetID() const;
    std::wstring toString() const;

    bool operator==(const StatsUID& other) const;
    bool operator!=(const StatsUID& other) const;
};

unsigned int hash_value(const StatsUID& uid);

#endif
