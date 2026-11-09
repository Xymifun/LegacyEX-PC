#include "game/minigame/RingsMedal.hpp"

#include <cstring>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/LevelGenerationOptions.hpp"

RingsMedal::RingsMedal(int name, unsigned int weight, eRingMedalAmount* amounts, EOperator op)
    : GameMedal(name, 0.0f, weight, op)
{
    std::memcpy(mAmounts, amounts, sizeof(mAmounts));
}

bool RingsMedal::CanAward(const StatsUID& uid, GameStats* gameStats) const
{
    if (gApp.getLevelGenerationOptions() != NULL && gApp.getLevelGenerationOptions()->getRequiredGameRules() != NULL)
    {
        if (Minecraft::GetInstance()->GetMiniGame()->GetGameVariant() == MiniGameDef::eGlideVariant_ScoreAttack)
        {
            std::vector<TargetAreaRuleDefinition*> targetAreas;
            gApp.getLevelGenerationOptions()->getRequiredGameRules()->getTargetAreas(&targetAreas);

            int targetAreaCounts[3];
            std::memset(targetAreaCounts, 0, sizeof(targetAreaCounts));
            for (std::vector<TargetAreaRuleDefinition*>::iterator targetArea = targetAreas.begin();
                 targetArea != targetAreas.end(); ++targetArea)
            {
                if (!(*targetArea)->mPrimary)
                    ++targetAreaCounts[(*targetArea)->getSize()];
            }

            PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
            if (playerStats != NULL)
            {
                for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
                {
                    PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
                    bool award = true;
                    for (int size = 0; size < 3; ++size)
                    {
                        bool matched = false;
                        switch (mAmounts[size])
                        {
                        case eRingAmountAll:
                            if (roundStats->mBankedRings[size] + roundStats->mUnbankedRings[size]
                                == targetAreaCounts[size])
                                matched = true;
                            break;
                        case eRingAmountSome:
                            if (roundStats->mBankedRings[size] + roundStats->mUnbankedRings[size] != 0)
                                matched = true;
                            break;
                        case eRingAmountNone:
                            if (roundStats->mBankedRings[size] + roundStats->mUnbankedRings[size] == 0)
                                matched = true;
                            break;
                        case eRingAmountMost:
                            matched = true;
                            break;
                        }
                        award &= matched;
                    }
                    if (award)
                        return true;
                }
            }
        }
    }
    return false;
}
