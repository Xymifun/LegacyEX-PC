#include "game/minigame/DamageSourceMedal.hpp"
#include "game/core/DamageTakenBySourceStatTracker.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

DamageSourceMedal::DamageSourceMedal(int name, float target, unsigned int weight, Scoreboard::EDamageSource source)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
    Scoreboard::EDamageSource sources[] = {source};
    mSources = new std::vector<Scoreboard::EDamageSource>(sources, sources + 1);
}

DamageSourceMedal::DamageSourceMedal(int name, float target, unsigned int weight,
                                     std::vector<Scoreboard::EDamageSource>* sources)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false),
      mSources(sources)
{
}

DamageSourceMedal::~DamageSourceMedal()
{
    delete mSources;
}

float DamageSourceMedal::GetScore(PlayerStats* playerStats) const
{
    float score = 0.0f;

    for (std::vector<Scoreboard::EDamageSource>::iterator source = mSources->begin(); source != mSources->end();
         ++source)
    {
        score += playerStats->mDamageTakenBySource[*source]->getValue();
    }

    return score;
}
