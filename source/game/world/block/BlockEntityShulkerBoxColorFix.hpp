#ifndef GAME_WORLD_BLOCK_BLOCKENTITYSHULKERBOXCOLORFIX_HPP
#define GAME_WORLD_BLOCK_BLOCKENTITYSHULKERBOXCOLORFIX_HPP

#include "game/data/CompoundTag.hpp"

#include "game/data/DataFix.hpp"

class BlockEntityShulkerBoxColorFix : public DataFix
{
public:
    virtual ~BlockEntityShulkerBoxColorFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
