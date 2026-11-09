#include "game/entity/ai/TurtlePathNavigation.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/PathFinder.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

TurtlePathNavigation::TurtlePathNavigation(Turtle* turtle, Level* level)
    : WaterBoundPathNavigation(turtle, level, false, false)
{
    mPathFinder = createPathFinder();
}

PathFinder* TurtlePathNavigation::createPathFinder()
{
    AmphibiousNodeEvaluator* evaluator = new AmphibiousNodeEvaluator(mMob);
    return new PathFinder(evaluator);
}

bool TurtlePathNavigation::canUpdatePath()
{
    return true;
}

bool TurtlePathNavigation::isStableDestination(const BlockPos& pos)
{
    if (eTYPE_DERIVED_FROM(EntityType::TURTLE, mMob->GetType()))
    {
        Turtle* turtle = static_cast<Turtle*>(mMob);
        if (turtle->isTravelling() && turtle->isInWater())
        {
            const BlockState* state = mLevel->getBlockState(pos);
            return state->getBlock() == Blocks::water;
        }
    }
    const BlockState* below = mLevel->getBlockState(pos.below());
    return below->getMaterial() != Material::AIR;
}
