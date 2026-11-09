#include "game/core/VariantCondition.hpp"

#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"

VariantCondition::VariantCondition(EMiniGameVariantId variant)
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false),
      mVariant(variant)
{
}

VariantCondition::~VariantCondition()
{
}

bool VariantCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    return Minecraft::GetInstance()->GetMiniGame()->GetGameVariant() == mVariant;
}
