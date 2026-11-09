#include "game/entity/ai/TurtleRandomStrollGoal.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Turtle.hpp"

TurtleRandomStrollGoal::TurtleRandomStrollGoal(Turtle* turtle, double speedModifier, int interval)
    : RandomStrollGoal(turtle, speedModifier, interval)
{
}

TurtleRandomStrollGoal::~TurtleRandomStrollGoal()
{
}

bool TurtleRandomStrollGoal::canUse()
{
    if ((!eTYPE_DERIVED_FROM(EntityType::TURTLE, mMob->GetType()) || !static_cast<Turtle*>(mMob)->hasEgg()
         || static_cast<Turtle*>(mMob)->hasGivenUpOnEgg())
        && !mMob->isInWater())
    {
        return RandomStrollGoal::canUse();
    }
    return false;
}
