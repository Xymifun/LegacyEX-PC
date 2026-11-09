#include "game/entity/ai/SwimNodeEvaluator.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/Vec3i.hpp"

SwimNodeEvaluator::SwimNodeEvaluator(bool allowBreaching) : NodeEvaluator(), mAllowBreaching(allowBreaching)
{
}

const BlockPathTypes* SwimNodeEvaluator::getPathTypeAtPos(int x, int y, int z, bool checkNeighbors)
{
    BlockPos pos(x, y, z);
    const Material* liquidMaterial = NULL;
    const BlockState* state = mLevelSource->getBlockState(mMob->mTickCache, pos);
    const Material* material = state->getMaterial();
    if (material == Material::AIR)
        liquidMaterial = mLevelSource->getLiquidBlock(pos)->getCraftingMaterial();

    if (state->getMaterial() == Material::AIR && liquidMaterial == Material::AIR)
    {
        const BlockState* belowState = mLevelSource->getBlockState(mMob->mTickCache, pos.below());
        if (belowState->getMaterial() == Material::WATER)
            return BlockPathTypes::BREACH;
        if (static_cast<double>(pos.y) <= mMob->mY)
            return BlockPathTypes::WALKABLE;
    }
    else if (state->getMaterial() != Material::WATER)
    {
        Block* block = state->getBlock();
        if (block->isSlabBlock() && HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state)))
            return BlockPathTypes::WALKABLE;
    }
    else
    {
        if (checkNeighbors)
        {
            for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
                 it != Direction::VALUES.end(); ++it)
            {
                const Direction* direction = *it;
                if (direction != Direction::DOWN && direction != Direction::UP
                    && getPathTypeAtPos(x + direction->getStepX(), y + direction->getStepY(), z + direction->getStepZ(),
                                        false)
                           == BlockPathTypes::BLOCKED)
                    return BlockPathTypes::WATER_BORDER;
            }
        }
        return BlockPathTypes::WATER;
    }
    return BlockPathTypes::BLOCKED;
}

Node* SwimNodeEvaluator::getStart()
{
    return NodeEvaluator::getNode(Mth::floor(mMob->mX), Mth::floor(mMob->mY + 0.5), Mth::floor(mMob->mZ));
}

Node* SwimNodeEvaluator::getGoal(double x, double y, double z)
{
    return NodeEvaluator::getNode(Mth::floor(x - mMob->mBbWidth * 0.5f), Mth::floor(y + 0.5),
                                  Mth::floor(z - mMob->mBbWidth * 0.5f));
}

Node* SwimNodeEvaluator::getDirectGoal(const BlockPos& pos)
{
    return NodeEvaluator::getNode(pos.x, pos.y, pos.z);
}

const BlockPathTypes* SwimNodeEvaluator::isFree(int x, int y, int z)
{
    for (int xx = x; xx < x + mEntityWidth; ++xx)
        for (int yy = y; yy < y + mEntityHeight; ++yy)
            for (int zz = z; zz < z + mEntityDepth; ++zz)
            {
                const BlockPathTypes* type = getPathTypeAtPos(xx, yy, zz, true);
                if (type != BlockPathTypes::WATER)
                    return type;
            }
    return BlockPathTypes::WATER;
}

Node* SwimNodeEvaluator::getWaterNode(int x, int y, int z)
{
    const BlockPathTypes* type = isFree(x, y, z);
    if (type == BlockPathTypes::WATER || (mAllowBreaching && type == BlockPathTypes::BREACH))
        return NodeEvaluator::getNode(x, y, z);

    if (!mMob->isInWaterOrBubble())
    {
        if (type == BlockPathTypes::WATER_BORDER || type == BlockPathTypes::WALKABLE)
            return NodeEvaluator::getNode(x, y, z);
    }
    else if (type == BlockPathTypes::WATER_BORDER)
    {
        if (getPathTypeAtPos(Mth::floor(mMob->mX), Mth::floor(mMob->mY), Mth::floor(mMob->mZ), false)
            == BlockPathTypes::WATER)
            return NodeEvaluator::getNode(x, y, z);
    }
    return NULL;
}

int SwimNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    std::vector<const Direction*>::const_iterator it;
    for (it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* direction = *it;
        Node* node = getWaterNode(current->x + direction->getStepX(), current->y + direction->getStepY(),
                                  current->z + direction->getStepZ());
        if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
        {
            neighbors->data[count++] = node;
        }
    }
    return count;
}

const BlockPathTypes* SwimNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob,
                                                          int sizeX, int sizeY, int sizeZ, bool canOpenDoors,
                                                          bool canPassDoors)
{
    return BlockPathTypes::WATER;
}

const BlockPathTypes* SwimNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    return BlockPathTypes::WATER;
}

void SwimNodeEvaluator::blockChanged(const BlockPos& pos)
{
}
