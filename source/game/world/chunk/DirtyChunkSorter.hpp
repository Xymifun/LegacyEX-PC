#ifndef GAME_WORLD_CHUNK_DIRTYCHUNKSORTER_HPP
#define GAME_WORLD_CHUNK_DIRTYCHUNKSORTER_HPP

#include <boost/shared_ptr.hpp>

class Chunk;
class LivingEntity;

class DirtyChunkSorter
{
public:
    DirtyChunkSorter(boost::shared_ptr<LivingEntity> entity, int renderDistance);
    bool operator()(const Chunk* first, const Chunk* second) const;

private:
    boost::shared_ptr<LivingEntity> mEntity;
    int mRenderDistance;
};

#endif
