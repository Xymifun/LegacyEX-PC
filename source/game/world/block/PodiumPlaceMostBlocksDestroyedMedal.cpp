#include "game/world/block/PodiumPlaceMostBlocksDestroyedMedal.hpp"

#include "game/core/GameStats.hpp"
#include "game/core/StatsUID.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"

PodiumPlaceMostBlocksDestroyedMedal::PodiumPlaceMostBlocksDestroyedMedal(EPodium podium, int name, float target,
                                                                         unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false),
      mPodium(podium)
{
}

float PodiumPlaceMostBlocksDestroyedMedal::GetScore(PlayerStats* playerStats) const
{
    GameStats* gameStats = getGameStats();
    int score = 0;
    PlayerList* playerList = gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
    boost::shared_ptr<ServerPlayer> player = playerList->getPlayerFromStatsUID(playerStats->mStatsUID);
    boost::shared_ptr<ServerPlayer> firstPlace = gameStats->GetFirstPlace();
    boost::shared_ptr<ServerPlayer> secondPlace = gameStats->GetSecondPlace();
    boost::shared_ptr<ServerPlayer> thirdPlace = gameStats->GetThirdPlace();
    unsigned int mostBlocksDestroyed = gameStats->GetMostBlocksDestroyed();
    unsigned int secondMostBlocksDestroyed = gameStats->GetSecondMostBlocksDestroyed();
    unsigned int thirdMostBlocksDestroyed = gameStats->GetThirdMostBlocksDestroyed();

    if (player)
    {
        switch (mPodium)
        {
        case PODIUM_FIRST:
            if (player == firstPlace)
            {
                score = playerStats->getTotalBlocksDestroyed();
            }
            break;
        case PODIUM_SECOND:
            if ((player != firstPlace || playerStats->getTotalBlocksDestroyed() != mostBlocksDestroyed)
                && (player == secondPlace || player == firstPlace))
            {
                unsigned int blocksDestroyed = playerStats->getTotalBlocksDestroyed();

                if (blocksDestroyed >= secondMostBlocksDestroyed)
                {
                    score = blocksDestroyed - secondMostBlocksDestroyed;
                }
            }
            break;
        case PODIUM_THIRD:
            if (((player != firstPlace && player != secondPlace)
                 || playerStats->getTotalBlocksDestroyed() < secondMostBlocksDestroyed)
                && (player == secondPlace || player == firstPlace || player == thirdPlace))
            {
                unsigned int blocksDestroyed = playerStats->getTotalBlocksDestroyed();

                if (blocksDestroyed >= thirdMostBlocksDestroyed)
                {
                    score = blocksDestroyed - thirdMostBlocksDestroyed;
                }
            }
            break;
        }
    }

    return score;
}
