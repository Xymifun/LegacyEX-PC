#include "game/world/block/EatBlockGoal.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/LevelEvent.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/core/Boxed.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

static Predicate<const BlockState*>* sTallGrassPredicate;

bool tallGrassPredicate(Boxed* value)
{
    const TypedBoxed<TallGrassBlock::Variant*>* boxed =
        value->isA(typeid(TallGrassBlock::Variant*)) ? static_cast<const TypedBoxed<TallGrassBlock::Variant*>*>(value)
                                                     : NULL;
    return boxed->getValue() == TallGrassBlock::Variant::GRASS;
}

EatBlockGoal::EatBlockGoal(Mob* mob)
{
    mEatAnimationTick = 0;
    mMob = mob;
    mLevel = mob->mLevel;
    setRequiredControlFlags(7);
}

EatBlockGoal::~EatBlockGoal()
{
}

void EatBlockGoal::staticCtor()
{
    sTallGrassPredicate = BlockStatePredicate::forBlock(Blocks::tallgrass)
                              ->where(TallGrassBlock::VARIANT,
                                      new Predicates<Boxed*>::FunctionPredicate(tallGrassPredicate));
}

bool EatBlockGoal::canUse()
{
    if (mMob->getRandom()->nextInt(mMob->isBaby() ? 50 : 1000) != 0)
        return false;

    BlockPos pos(mMob->mX, mMob->mY, mMob->mZ);
    if (sTallGrassPredicate->apply(mLevel->getBlockState(pos)))
        return true;
    if (mLevel->getBlockState(pos.below())->getBlock() == Blocks::grass)
        return true;
    return false;
}

void EatBlockGoal::start()
{
    mEatAnimationTick = 40;
    mLevel->broadcastEntityEvent(mMob->shared_from_this(), EntityEvent::EAT_GRASS, 0);
    mMob->getNavigation()->stop();
}

void EatBlockGoal::stop()
{
    mEatAnimationTick = 0;
}

bool EatBlockGoal::canContinueToUse()
{
    return mEatAnimationTick > 0;
}

int EatBlockGoal::getEatAnimationTick()
{
    return mEatAnimationTick;
}

void EatBlockGoal::tick()
{
    mEatAnimationTick = std::max(0, mEatAnimationTick - 1);

    if (mEatAnimationTick == 4)
    {
        BlockPos pos(mMob->mX, mMob->mY, mMob->mZ);
        if (sTallGrassPredicate->apply(mLevel->getBlockState(pos)))
        {
            if (mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
            {
                mLevel->destroyBlock(pos, false);
            }
            mMob->ate();
        }
        else
        {
            BlockPos below = pos.below();
            if (mLevel->getBlockState(below)->getBlock() == Blocks::grass)
            {
                if (mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
                {
                    mLevel->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, below, Block::getId(Blocks::grass));
                    mLevel->setBlock(below, Blocks::dirt->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }
            }
            mMob->ate();
        }
    }
}

void EatBlockGoal::setLevel(Level* level)
{
    mLevel = level;
}
