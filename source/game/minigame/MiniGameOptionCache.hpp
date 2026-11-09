#ifndef GAME_MINIGAME_MINIGAMEOPTIONCACHE_HPP
#define GAME_MINIGAME_MINIGAMEOPTIONCACHE_HPP

#include <boost/unordered_map.hpp>
#include <utility>

#include "game/minigame/EMiniGameId.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/world/generation/PlayerUID.hpp"

class MiniGameOptionCache
{
public:
    class KeyHasher
    {
    public:
        unsigned int operator()(const std::pair<PlayerUID, EMiniGameId>& key) const;
    };

    MiniGameOptionCache();

    void updateCachedOptions(int playerIndex, EMiniGameId miniGameId, const MiniGamePlayerSettings& options);
    MiniGamePlayerSettings& getPreviousOptions(int playerIndex, EMiniGameId miniGameId);

private:
    typedef boost::unordered_map<std::pair<PlayerUID, EMiniGameId>, MiniGamePlayerSettings, KeyHasher> OptionMap;

    OptionMap mOptions;
};

#endif
