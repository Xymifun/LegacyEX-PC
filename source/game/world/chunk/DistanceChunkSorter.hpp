#ifndef GAME_WORLD_CHUNK_DISTANCECHUNKSORTER_HPP
#define GAME_WORLD_CHUNK_DISTANCECHUNKSORTER_HPP

#include <boost/shared_ptr.hpp>

class Chunk;
class Entity;

class DistanceChunkSorter
{
public:
    DistanceChunkSorter(boost::shared_ptr<Entity> entity);
    bool operator()(const Chunk* first, const Chunk* second) const;

private:
    boost::shared_ptr<Entity> mEntity;
};

#endif
