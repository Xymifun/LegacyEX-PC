#ifndef GAME_WORLD_BLOCK_THEENDPORTALBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_THEENDPORTALBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class Direction;

class TheEndPortalBlockEntity : public BlockEntity
{
public:
    virtual ~TheEndPortalBlockEntity() {}

    static BlockEntity* create();
    virtual unsigned long long GetType();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual bool shouldRenderFace(const Direction* direction);
};

#endif
