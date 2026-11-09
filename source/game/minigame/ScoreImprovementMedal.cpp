#include "game/minigame/ScoreImprovementMedal.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/PlayerStats.hpp"

float ScoreImprovementMedal::GetScore(PlayerStats* playerStats) const
{
    ClientMasterGameMode* clientMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    int finalRound = Minecraft::GetInstance()->GetMiniGame()->GetFinalRoundIndex();

    if (clientMode != NULL)
    {
        const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
        if (miniGame->ArePointsActive())
        {
            int firstPoints = playerStats->getRoundStats(0)->mPoints + playerStats->getRoundStats(0)->mUnbankedPoints;
            return static_cast<double>(
                static_cast<float>(static_cast<unsigned int>((playerStats->getRoundStats(finalRound)->mPoints
                                                              + playerStats->getRoundStats(finalRound)->mUnbankedPoints)
                                                             - firstPoints)));
        }

        miniGame = CommonMasterGameMode::GetMiniGame();
        if (miniGame->ArePowerupsActive())
        {
            int firstScore = 0;
            unsigned int round = 0;
            while (round < static_cast<unsigned int>(playerStats->getTotalRounds()))
            {
                PlayerStats::RoundStats* stats = playerStats->getRoundStats(round);
                if (stats->mReachedEnd)
                {
                    firstScore = stats->mLifeSpan * 2;
                    break;
                }
                ++round;
            }

            if (playerStats->getRoundStats(finalRound)->mReachedEnd)
            {
                return static_cast<double>(static_cast<float>(
                    static_cast<unsigned int>(firstScore - playerStats->getRoundStats(finalRound)->mLifeSpan)));
            }
        }
    }

    return 0.0;
}
