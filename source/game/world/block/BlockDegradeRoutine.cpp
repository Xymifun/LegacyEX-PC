#include "game/world/block/BlockDegradeRoutine.hpp"
#include "game/world/block/BlockId.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/DegradationRuleDefinition.hpp"
#include "game/minigame/DegradationSequenceRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"

BlockDegradeRoutine::BlockDegradeRoutine(ServerLevel* level, DegradationSequenceRuleDefinition* definition)
    : mLevel(level),
      mDefinition(definition),
      mNextActionTime(0),
      mAction(&BlockDegradeRoutine::Action_Wait)
{
    std::vector<GameRuleDefinition*> children;
    mDefinition->getChildren(&children);

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        Info info;
        info.rule = dynamic_cast<DegradationRuleDefinition*>(*it);
        info.cache = getCacheFor(info.rule->getTargetLayer());
        mActions.push_back(info);
    }
}

void BlockDegradeRoutine::setNextAction(Action action, long long nextActionTime)
{
    mAction = action;
    mNextActionTime = nextActionTime;
}

void BlockDegradeRoutine::Action_Wait(long long time, long long ticks)
{
    if (!mActions.empty())
    {
        AABB* bounds = AABB::newTemp(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        bounds->set(mActions.front().cache->getCurrentBounds());
        bounds->mMaxY += 2.0;

        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Player), bounds);
        unsigned int entityCount = entities->size();
        delete entities;

        if (entityCount == 0)
        {
            LayerCache* currentCache = mActions.front().cache;
            DegradationRuleDefinition* rule = mActions.front().rule;
            mActions.pop_front();

            {
                AABB* area = currentCache->getCurrentBounds();
                BlockPos from(area->mMinX, area->mMinY, area->mMinZ);
                BlockPos to(area->mMaxX, area->mMaxY, area->mMaxZ);
                std::vector<BlockPos> positions;
                Iterator<BlockPos*>* it = currentCache->iterator();
                BlockPos* pos = it->next();
                while (it->hasNext())
                {
                    if (currentCache->inLayer(*pos))
                        positions.push_back(*pos);
                    it->next();
                }
                mBlocks = arrayWithLength<BlockPos>(positions);
                delete it;
            }
            delete currentCache;
            rule->ClearCaches();

            mProgress = 11;
            setNextAction(&BlockDegradeRoutine::Action_Break, time + 500);
        }
        else
        {
            setNextAction(&BlockDegradeRoutine::Action_Wait, time + 400);
        }
    }
}

void BlockDegradeRoutine::Action_Break(long long time, long long ticks)
{
    if (mProgress < 10)
    {
        Random random(0);
        int id = -1;
        for (int i = 0; i < (int)mBlocks.length; ++i)
        {
            BlockPos& pos = mBlocks[i];
            mLevel->destroyBlockProgress(id, pos, Mth::clamp(mProgress + random.nextInt(5) - 1, 0, 9));
            --id;
        }

        int progress = mProgress + 1;
        if (progress > 10)
            progress = 10;
        mProgress = progress;

        setNextAction(&BlockDegradeRoutine::Action_Break, time + 400);
    }
    else
    {
        int id = -1;
        for (unsigned int i = 0; i < mBlocks.length; ++i)
        {
            BlockPos& pos = mBlocks[i];
            mLevel->destroyBlockProgress(id, pos, -1);
            mLevel->setBlockAndData(pos.x, pos.y, pos.z, BlockId::AIR, 0, Block::UPDATE_CLIENTS);
            --id;
        }

        mProgress = 1;
        setNextAction(&BlockDegradeRoutine::Action_Wait, time + 100);
    }
}

BlockDegradeRoutine::LayerCache* BlockDegradeRoutine::getCacheFor(const std::wstring& name) const
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset != NULL)
    {
        NamedAreaRuleDefinition* area = ruleset->getNamedAreaRule(name);
        if (area != NULL)
            return new LayerCache(mLevel, area);
    }
    return NULL;
}

BlockDegradeRoutine::LayerCache::LayerCache(ServerLevel* level, NamedAreaRuleDefinition* rule)
    : mLevel(level),
      mRule(rule),
      mCurrentBounds(rule->getArea())
{
}

BlockDegradeRoutine::LayerCache::~LayerCache()
{
}

AABB* BlockDegradeRoutine::LayerCache::getCurrentBounds() const
{
    return mCurrentBounds;
}

Iterator<BlockPos*>* BlockDegradeRoutine::LayerCache::iterator() const
{
    AABB* area = getCurrentBounds();
    BlockPos from(area->mMinX, area->mMinY, area->mMinZ);
    BlockPos to(area->mMaxX, area->mMaxY, area->mMaxZ);
    return BlockPos::between(from, to);
}

void BlockDegradeRoutine::LayerCache::setBroken(const BlockPos* positions, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
        mBroken.insert(positions[i]);
}

bool BlockDegradeRoutine::LayerCache::isBroken(const BlockPos& pos)
{
    if (mBroken.find(pos) != mBroken.end())
        return true;

    Block* block = mLevel->getBlock(pos);
    bool broken = block == Blocks::air;
    if (broken)
        setBroken(&pos, 1);
    return broken;
}

bool BlockDegradeRoutine::LayerCache::inLayer(const BlockPos& pos)
{
    return !isBroken(pos) && mRule->containsBlock(Block::getId(mLevel->getBlock(pos)), pos);
}
