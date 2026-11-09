#include "game/network/PathNavigationListener.hpp"
#include <algorithm>
#include "game/core/EntityType.hpp"

#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/ai/NodeEvaluator.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/AABB.hpp"

PathNavigationListener::PathNavigationListener() : mPathNavigations()
{
    InitializeCriticalSection(&mMutex);
}

PathNavigationListener::~PathNavigationListener()
{
    DeleteCriticalSection(&mMutex);
}

bool PathNavigationListener::didBlockChange(Level* level, const BlockPos& pos, const BlockState* oldState,
                                            const BlockState* newState)
{
    const AABB* oldShape = oldState->getClipAABB((LevelSource*)level, pos);
    const AABB* newShape = newState->getClipAABB((LevelSource*)level, pos);
    int same = 1;
    if (oldShape != newShape && (oldShape == NULL || !oldShape->equals(newShape)))
        same = 0;
    return !same;
}

void PathNavigationListener::blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState,
                                          const BlockState* newState, int, bool)
{
    if (!didBlockChange(level, pos, oldState, newState))
        return;
    EnterCriticalSection(&mMutex);
    for (std::vector<PathNavigation*>::iterator it = mPathNavigations.begin(); it != mPathNavigations.end(); ++it)
    {
        PathNavigation* navigation = *it;
        if (navigation == NULL)
            continue;
        if (navigation->getNodeEvaluator() != NULL)
            navigation->getNodeEvaluator()->blockChanged(pos);
        PathFinder* finder = navigation->getPathFinder();
        if (finder != NULL && finder->mNodeEvaluator != NULL)
            finder->mNodeEvaluator->blockChanged(pos);
        if (navigation->hasDelayedRecomputation())
            continue;
        Path* path = navigation->getPath();
        if (path != NULL && !path->isDone() && path->getSize() != 0)
        {
            Node* last = navigation->mPath->last();
            double distance = pos.distSqr((last->x + navigation->mMob->mX) * 0.5,
                                          (last->y + navigation->mMob->mY) * 0.5,
                                          (last->z + navigation->mMob->mZ) * 0.5);
            int remaining = path->getSize() - path->getIndex();
            if (distance < static_cast<double>(remaining * remaining))
                navigation->recomputePath();
        }
    }
    LeaveCriticalSection(&mMutex);
}

void PathNavigationListener::entityAdded(boost::shared_ptr<Entity> entity)
{
    if (entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        EnterCriticalSection(&mMutex);
        mPathNavigations.push_back(boost::static_pointer_cast<Mob>(entity)->getNavigation());
        LeaveCriticalSection(&mMutex);
    }
}

void PathNavigationListener::entityRemoved(boost::shared_ptr<Entity> entity)
{
    if (entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        EnterCriticalSection(&mMutex);
        std::vector<PathNavigation*>::iterator it = std::find(mPathNavigations.begin(), mPathNavigations.end(),
                                                              boost::static_pointer_cast<Mob>(entity)->getNavigation());
        if (it != mPathNavigations.end())
            mPathNavigations.erase(it);
        LeaveCriticalSection(&mMutex);
    }
}

void PathNavigationListener::blockLightChanged(const BlockPos&)
{
}
void PathNavigationListener::setBlocksDirty(int, int, int, int, int, int, Level*)
{
}
void PathNavigationListener::allChanged()
{
}
void PathNavigationListener::playStreamingMusic(const SoundEvent*, const BlockPos&, bool)
{
}
void PathNavigationListener::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource, double,
                                       double, double, float, float, float)
{
}
void PathNavigationListener::addParticle(Level*, ePARTICLE_TYPE, bool, double, double, double, double, double, double,
                                         arrayWithLength<int>)
{
}
void PathNavigationListener::addParticle(Level*, ePARTICLE_TYPE, bool, bool, double, double, double, double, double,
                                         double, arrayWithLength<int>)
{
}
void PathNavigationListener::playerRemoved(boost::shared_ptr<Entity>)
{
}
void PathNavigationListener::globalLevelEvent(int, const BlockPos&, int)
{
}
void PathNavigationListener::levelEvent(boost::shared_ptr<Player>, int, const BlockPos&, int)
{
}
void PathNavigationListener::destroyBlockProgress(int, const BlockPos&, int)
{
}
