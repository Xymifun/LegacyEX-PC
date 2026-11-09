#ifndef GAME_ITEM_BEDITEMCOLORFIX_HPP
#define GAME_ITEM_BEDITEMCOLORFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class BedItemColorFix : public DataFix
{
public:
    virtual ~BedItemColorFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
