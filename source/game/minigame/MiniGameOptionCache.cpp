#include "game/minigame/MiniGameOptionCache.hpp"

#include "game/core/GameModeOptions.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/CProfile.hpp"

extern C_4JProfile* gProfile;

MiniGameOptionCache gMiniGameOptionCache;

unsigned int MiniGameOptionCache::KeyHasher::operator()(const std::pair<PlayerUID, EMiniGameId>& key) const
{
    return hash_value(key.first) ^ static_cast<unsigned int>(key.second);
}

MiniGameOptionCache::MiniGameOptionCache()
{
}

void MiniGameOptionCache::updateCachedOptions(int playerIndex, EMiniGameId miniGameId,
                                              const MiniGamePlayerSettings& options)
{
    PlayerUID playerUid(0);
    gProfile->GetXUID(playerIndex, &playerUid, true);
    mOptions[std::make_pair(playerUid, miniGameId)] = options;
}

MiniGamePlayerSettings& MiniGameOptionCache::getPreviousOptions(int playerIndex, EMiniGameId miniGameId)
{
    PlayerUID playerUid(0);
    gProfile->GetXUID(playerIndex, &playerUid, true);
    std::pair<PlayerUID, EMiniGameId> key(std::make_pair(playerUid, miniGameId));
    OptionMap::iterator it = mOptions.find(key);
    if (it == mOptions.end())
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(miniGameId, true);
        MiniGamePlayerSettings options = miniGame->GetDefaultPlayerSettings();
        if (miniGameId == eMiniGameId_Tumble || miniGameId == eMiniGameId_Build)
        {
            options.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, 0);
        }
        mOptions[key] = options;
        it = mOptions.find(key);
    }
    return it->second;
}
