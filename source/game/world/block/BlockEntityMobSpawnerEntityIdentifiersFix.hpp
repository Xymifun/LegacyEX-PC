#ifndef GAME_WORLD_BLOCK_BLOCKENTITYMOBSPAWNERENTITYIDENTIFIERSFIX_HPP
#define GAME_WORLD_BLOCK_BLOCKENTITYMOBSPAWNERENTITYIDENTIFIERSFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class BlockEntityMobSpawnerEntityIdentifiersFix : public DataFix
{
public:
    virtual ~BlockEntityMobSpawnerEntityIdentifiersFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
