#include "game/entity/ai/DoorInteractGoal.hpp"

#include <algorithm>

#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"

DoorInteractGoal::DoorInteractGoal(Mob* mob)
{
    mDoorPos = BlockPos::ZERO;
    mMob = mob;
    mDoorBlock = NULL;
    mHasDoor = false;
    mDoorZ = 0.0f;
    mDoorX = 0.0f;
}

Block* DoorInteractGoal::getDoorBlock(const BlockPos& pos)
{
    const BlockState* state = mMob->mLevel->getBlockState(pos);
    Block* block = state->getBlock();
    if (DoorBlock::isDoor(block) && state->getMaterial() == Material::WOOD)
        return state->getBlock();
    return NULL;
}

bool DoorInteractGoal::canUse()
{
    if (!mMob->mHorizontalCollision)
        return false;

    GroundPathNavigation* navigation = static_cast<GroundPathNavigation*>(mMob->getNavigation());
    Path* path = navigation->getPath();
    if (path == NULL || path->isDone() || !navigation->canOpenDoors())
        return false;

    for (int i = 0; i < std::min(path->getIndex() + 2, path->getSize()); ++i)
    {
        Node* node = path->get(i);
        mDoorPos = BlockPos(node->x, node->y + 1, node->z);
        if (mMob->distanceToSqr((double)mDoorPos.x, mMob->mY, (double)mDoorPos.z) <= 2.25)
        {
            mDoorBlock = getDoorBlock(mDoorPos);
            if (mDoorBlock != NULL)
                return true;
        }
    }

    mDoorPos = BlockPos(mMob->shared_from_this()).above();
    mDoorBlock = getDoorBlock(mDoorPos);
    return mDoorBlock != NULL;
}

bool DoorInteractGoal::canContinueToUse()
{
    return !mHasDoor;
}

void DoorInteractGoal::start()
{
    mHasDoor = false;
    mDoorX = (float)((double)((float)mDoorPos.x + 0.5f) - mMob->mX);
    mDoorZ = (float)((double)((float)mDoorPos.z + 0.5f) - mMob->mZ);
}

void DoorInteractGoal::tick()
{
    float x = (float)((double)((float)mDoorPos.x + 0.5f) - mMob->mX);
    float z = (float)((double)((float)mDoorPos.z + 0.5f) - mMob->mZ);
    if (mDoorX * x + mDoorZ * z < 0.0f)
        mHasDoor = true;
}
