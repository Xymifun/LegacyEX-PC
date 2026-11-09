#include "game/world/chunk/DirtyChunkSorter.hpp"

#include "game/world/chunk/Chunk.hpp"
#include "game/entity/mob/LivingEntity.hpp"

DirtyChunkSorter::DirtyChunkSorter(boost::shared_ptr<LivingEntity> entity, int renderDistance)
{
    mEntity = entity;
    mRenderDistance = renderDistance;
}

bool DirtyChunkSorter::operator()(const Chunk* first, const Chunk* second) const
{
    bool firstVisible = first->mClipChunk->mVisible;
    bool secondVisible = second->mClipChunk->mVisible;
    if (firstVisible && !secondVisible)
    {
        return false;
    }
    if (secondVisible && !firstVisible)
    {
        return true;
    }

    double firstDistance = first->distanceToSqr(mEntity);
    double secondDistance = second->distanceToSqr(mEntity);

    if (firstDistance < secondDistance)
    {
        return false;
    }
    if (firstDistance > secondDistance)
    {
        return true;
    }
    return first->mUnknown >= second->mUnknown;
}
