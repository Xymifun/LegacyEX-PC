#ifndef GAME_CORE_VARIANTCONDITION_HPP
#define GAME_CORE_VARIANTCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

enum EMiniGameVariantId
{
    eMiniGameVariantId_ScoreAttack = 1
};

class VariantCondition : public MultiConditionMedal::MultiCondition
{
public:
    VariantCondition(EMiniGameVariantId variant);
    virtual ~VariantCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;

private:
    EMiniGameVariantId mVariant;
};

#endif
