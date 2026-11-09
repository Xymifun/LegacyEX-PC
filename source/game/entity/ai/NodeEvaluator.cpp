#include "game/entity/ai/NodeEvaluator.hpp"

#include "game/entity/mob/EntityTickCache.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Mth.hpp"

NodeEvaluator::NodeEvaluator()
{
    mNodeMap = boost::unordered_map<int, Node*, IntKeyHash2, IntKeyEq>();
    mCanPassDoors = false;
    mCanOpenDoors = false;
    mCanFloat = false;
    mCanSinkInWaterWalk = false;
    mMob = 0;
}

NodeEvaluator::~NodeEvaluator()
{
}

void NodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    mLevelSource = levelSource;
    mMob = mob;
    mNodeMap.clear();
    mBlockStateMap.clear();
    mBlockPathTypeMap.clear();
    Node::ResetPool();
    mEntityWidth = Mth::floor(mob->mBbWidth + 1.0f);
    mEntityHeight = Mth::floor(mob->mBbHeight + 1.0f);
    mEntityDepth = Mth::floor(mob->mBbWidth + 1.0f);
}

void NodeEvaluator::done()
{
}

Node* NodeEvaluator::getNode(int x, int y, int z)
{
    int hash = Node::createHash(x, y, z);
    boost::unordered_map<int, Node*, IntKeyHash2, IntKeyEq>::iterator it = mNodeMap.find(hash);
    Node* node;
    if (it != mNodeMap.end())
    {
        node = it->second;
    }
    else
    {
        node = Node::GetNodeFromPool(x, y, z);
        mNodeMap.insert(std::make_pair(hash, node));
    }
    return node;
}

Node* NodeEvaluator::getDirectGoal(const BlockPos& pos)
{
    (void)pos;
    return 0;
}

void NodeEvaluator::setCanPassDoors(bool value)
{
    mCanPassDoors = value;
}

void NodeEvaluator::setCanOpenDoors(bool value)
{
    mCanOpenDoors = value;
}

void NodeEvaluator::setCanFloat(bool value)
{
    mCanFloat = value;
}

bool NodeEvaluator::isCanPassDoors()
{
    return mCanPassDoors;
}

bool NodeEvaluator::isCanOpenDoors()
{
    return mCanOpenDoors;
}

bool NodeEvaluator::isCanFloat()
{
    return mCanFloat;
}

void NodeEvaluator::setSinkInWaterWalk(bool value)
{
    mCanSinkInWaterWalk = value;
}

bool NodeEvaluator::isCanSinkInWaterWalk()
{
    return mCanSinkInWaterWalk;
}

const BlockState* NodeEvaluator::getBlockState(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBlockState(pos);
}

const BlockState* NodeEvaluator::getBlockState(EntityTickCache& tickCache, LevelSource* levelSource,
                                               const BlockPos& pos)
{
    return levelSource->getBlockState(tickCache, pos);
}
