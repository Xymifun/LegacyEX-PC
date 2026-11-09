#ifndef GAME_WORLD_BLOCK_DROPPERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_DROPPERBLOCKENTITY_HPP

#include "game/world/block/DispenserBlockEntity.hpp"

class DataFixerUpper;

class DropperBlockEntity : public DispenserBlockEntity
{
public:
    virtual ~DropperBlockEntity();

    static BlockEntity* create();
    virtual int getTypeId(bool large);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
};

#endif
