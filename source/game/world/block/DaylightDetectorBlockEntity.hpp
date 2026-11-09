#ifndef GAME_WORLD_BLOCK_DAYLIGHTDETECTORBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_DAYLIGHTDETECTORBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class DaylightDetectorBlockEntity : public BlockEntity
{
public:
    DaylightDetectorBlockEntity();

    static BlockEntity* create();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual void tick();
    virtual unsigned long long GetType();
};

#endif
