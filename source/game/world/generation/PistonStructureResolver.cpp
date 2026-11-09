#include "game/world/generation/PistonStructureResolver.hpp"

#include <algorithm>
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/Direction.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/PistonBaseBlock.hpp"

PistonStructureResolver::PistonStructureResolver(Level* level, const BlockPos& pistonPos, const Direction* direction,
                                                 bool extending)
{
    mLevel = level;
    mPistonPos = pistonPos;
    if (extending)
    {
        mMoveDirection = direction;
        mBlockPos = pistonPos.relative(direction);
    }
    else
    {
        mMoveDirection = direction->getOpposite();
        mBlockPos = pistonPos.relative(direction, 2);
    }
}

void PistonStructureResolver::reorderListAtCollision(int collisionIndex, int distance)
{
    std::vector<BlockPos> before;
    std::vector<BlockPos> collided;
    std::vector<BlockPos> after;
    before.insert(before.end(), mToPush.begin(), mToPush.begin() + distance);
    collided.insert(collided.end(), mToPush.begin() + mToPush.size() - collisionIndex,
                    mToPush.begin() + mToPush.size());
    after.insert(after.end(), mToPush.begin() + distance, mToPush.begin() + mToPush.size() - collisionIndex);

    mToPush.clear();
    mToPush.insert(mToPush.end(), before.begin(), before.end());
    mToPush.insert(mToPush.end(), collided.begin(), collided.end());
    mToPush.insert(mToPush.end(), after.begin(), after.end());
}

bool PistonStructureResolver::addBranchingBlocks(const BlockPos& pos)
{
    std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
    if (it != Direction::VALUES.end())
    {
        do
        {
            const Direction* side = *it;
            if (side->getAxis() != mMoveDirection->getAxis())
            {
                if (!addBlockLine(pos.relative(side), side))
                {
                    return false;
                }
            }
            it++;
        } while (it != Direction::VALUES.end());
    }
    return true;
}

bool PistonStructureResolver::addBlockLine(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (state->getMaterial() == Material::AIR)
    {
        return true;
    }
    if (!PistonBaseBlock::isPushable(state, mLevel, pos, mMoveDirection, false, direction))
    {
        return true;
    }
    if (pos.equals(mPistonPos))
    {
        return true;
    }
    if (std::find(mToPush.begin(), mToPush.end(), pos) != mToPush.end())
    {
        return true;
    }

    int length = 1;
    if (mToPush.size() + length > 12)
    {
        return false;
    }
    while (state->getBlock() == Blocks::slime)
    {
        const BlockPos behind = pos.relative(mMoveDirection->getOpposite(), length);
        state = mLevel->getBlockState(behind);
        if (state->getMaterial() == Material::AIR
            || !PistonBaseBlock::isPushable(state, mLevel, behind, mMoveDirection, false, mMoveDirection->getOpposite())
            || behind.equals(mPistonPos))
        {
            break;
        }
        ++length;
        if (length + mToPush.size() > 12)
        {
            return false;
        }
    }

    int added = 0;
    for (int offset = length - 1; offset >= 0; --offset)
    {
        mToPush.push_back(pos.relative(mMoveDirection->getOpposite(), offset));
        ++added;
    }

    for (int step = 1;; ++step)
    {
        const BlockPos next = pos.relative(mMoveDirection, step);
        std::vector<BlockPos>::iterator collision = std::find(mToPush.begin(), mToPush.end(), next);
        if (collision != mToPush.end())
        {
            const int collisionIndex = static_cast<int>(collision - mToPush.begin());
            reorderListAtCollision(added, collisionIndex);
            for (int i = 0; i <= collisionIndex + added; ++i)
            {
                const BlockPos moved = mToPush.at(i);
                if (mLevel->getBlockState(moved)->getBlock() == Blocks::slime && !addBranchingBlocks(moved))
                {
                    return false;
                }
            }
            return true;
        }

        state = mLevel->getBlockState(next);
        if (state->getMaterial() == Material::AIR)
        {
            return true;
        }
        if (!PistonBaseBlock::isPushable(state, mLevel, next, mMoveDirection, true, mMoveDirection)
            || next.equals(mPistonPos))
        {
            return false;
        }
        if (state->getPistonPushReaction() == PISTON_PUSH_DESTROY)
        {
            mToDestroy.push_back(next);
            return true;
        }
        if (mToPush.size() >= 12)
        {
            return false;
        }
        mToPush.push_back(next);
        ++added;
    }
}

bool PistonStructureResolver::resolve()
{
    mToPush.clear();
    mToDestroy.clear();

    const BlockState* state = mLevel->getBlockState(mBlockPos);
    if (!PistonBaseBlock::isPushable(state, mLevel, mBlockPos, mMoveDirection, false, mMoveDirection))
    {
        if (state->getPistonPushReaction() == PISTON_PUSH_DESTROY)
        {
            mToDestroy.push_back(mBlockPos);
            return true;
        }
        return false;
    }
    if (!addBlockLine(mBlockPos, mMoveDirection))
    {
        return false;
    }
    unsigned int index = 0;
    while (index < mToPush.size())
    {
        const BlockPos moved = mToPush.at(index);
        if (mLevel->getBlockState(moved)->getBlock() == Blocks::slime && !addBranchingBlocks(moved))
        {
            return false;
        }
        ++index;
    }
    return true;
}

std::vector<BlockPos>& PistonStructureResolver::getToPush()
{
    return mToPush;
}

std::vector<BlockPos>& PistonStructureResolver::getToDestroy()
{
    return mToDestroy;
}
