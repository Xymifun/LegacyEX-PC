#include "game/world/block/RemoveBlockGoal.hpp"

#include <vector>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

RemoveBlockGoal::RemoveBlockGoal(Block* block, PathfinderMob* mob, double speedModifier, int searchRange,
                                 int verticalSearchStart, double maxStayDistance, const SoundEvent* startSound,
                                 const SoundEvent* endSound)
    : MoveToBlockGoal(mob, speedModifier, searchRange, verticalSearchStart, maxStayDistance),
      mStartSound(startSound),
      mEndSound(endSound)
{
    mBlock = block;
    mMob = mob;
}

RemoveBlockGoal::~RemoveBlockGoal()
{
}

bool RemoveBlockGoal::canUse()
{
    if (!mMob->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) || mMob->getRandom()->nextInt(20) != 0)
    {
        return false;
    }
    return MoveToBlockGoal::canUse();
}

int RemoveBlockGoal::GetNextStartTick(PathfinderMob*)
{
    return 0;
}

bool RemoveBlockGoal::canContinueToUse()
{
    return MoveToBlockGoal::canContinueToUse();
}

void RemoveBlockGoal::stop()
{
    Goal::stop();
    mMob->mFallDistance = 1.0f;
}

void RemoveBlockGoal::start()
{
    MoveToBlockGoal::start();
    mTicks = 0;
}

BlockPos* RemoveBlockGoal::getPosWithBlock(BlockPos pos, Level* level)
{
    if (level->getBlockState(pos)->getBlock() == mBlock)
    {
        return new BlockPos(pos);
    }

    std::vector<BlockPos> positions;
    positions.push_back(pos.below());
    positions.push_back(pos.west());
    positions.push_back(pos.east());
    positions.push_back(pos.north());
    positions.push_back(pos.south());
    positions.push_back(pos.below().below());
    for (std::vector<BlockPos>::iterator it = positions.begin(); it != positions.end(); ++it)
    {
        BlockPos candidate = *it;
        if (level->getBlockState(candidate)->getBlock() == mBlock)
        {
            return new BlockPos(candidate);
        }
    }
    return NULL;
}

void RemoveBlockGoal::tick()
{
    MoveToBlockGoal::tick();
    Level* level = mMob->mLevel;
    BlockPos origin(mMob);
    BlockPos* position = getPosWithBlock(origin, level);
    Random* random = mMob->getRandom();
    if (isReachedTarget())
    {
        if (position != NULL)
        {
            if (mTicks % 6 == 0 && mMob->mVerticalCollision)
            {
                mMob->mYd = 0.3f;
            }
            if (mTicks % 6 == 3)
            {
                mMob->mYd = -0.3f;
            }

            if (mTicks > 0)
            {
                if (!level->mIsClientSide)
                {
                    int params[2] = {Block::getId(mBlock), 0};
                    if (mStartSound != NULL && mTicks % 6 == 0)
                    {
                        mMob->playSound(mStartSound, 0.65f, 1.0f);
                    }
                    static_cast<ServerLevel*>(level)->sendParticles(
                        PARTICLE_ICON_CRACK, position->x + 0.5, position->y + 0.7, position->z + 0.5, 3,
                        (random->nextFloat() - 0.5) * 0.08, (random->nextFloat() - 0.5) * 0.08,
                        (random->nextFloat() - 0.5) * 0.08, 0.15f, arrayWithLength<int>(params, 2));
                }

                if (mTicks > 60)
                {
                    level->removeBlock(*position);
                    if (!level->mIsClientSide)
                    {
                        if (mEndSound != NULL)
                        {
                            mMob->playSound(mEndSound, 0.65f, 1.0f);
                        }
                        for (int i = 0; i < 20; ++i)
                        {
                            double xDist = random->nextGaussian() * 0.02;
                            double yDist = random->nextGaussian() * 0.02;
                            double zDist = random->nextGaussian() * 0.02;
                            static_cast<ServerLevel*>(level)->sendParticles(
                                PARTICLE_EXPLODE, position->x + 0.5, position->y, position->z + 0.5, 1, xDist, yDist,
                                zDist, 0.15f, arrayWithLength<int>());
                        }
                    }
                }
            }
            ++mTicks;
            delete position;
        }
    }
    else if (position != NULL)
    {
        delete position;
    }
}

bool RemoveBlockGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    return level->getBlock(pos) == mBlock;
}

bool RemoveBlockGoal::isValidTargetArea(Level* level, const BlockPos& from, const BlockPos& to)
{
    int blockId = Block::getId(mBlock);
    BlockPos end = to.offset(4, 4, 4);
    for (int x = from.x; x < end.x; x += 4)
    {
        for (int y = from.y; y < end.y; y += 4)
        {
            for (int z = from.z; z < end.z; z += 4)
            {
                if (level->sectionContainsBlockId(x, y, z, blockId))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
