#ifndef GAME_WORLD_BLOCK_BEDBLOCKENTITYINJECTER_HPP
#define GAME_WORLD_BLOCK_BEDBLOCKENTITYINJECTER_HPP

#include "game/data/DataFix.hpp"
#include "game/data/CompoundTag.hpp"

class BedBlockEntityInjecter : public DataFix
{
public:
    virtual ~BedBlockEntityInjecter();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
