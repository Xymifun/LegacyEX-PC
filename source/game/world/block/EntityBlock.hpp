#ifndef GAME_WORLD_BLOCK_ENTITYBLOCK_HPP
#define GAME_WORLD_BLOCK_ENTITYBLOCK_HPP

#include <boost/shared_ptr.hpp>

class BlockEntity;
class Level;

class EntityBlock
{
public:
    virtual ~EntityBlock() {}

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data) = 0;
};

#endif
