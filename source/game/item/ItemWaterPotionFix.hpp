#ifndef GAME_ITEM_ITEMWATERPOTIONFIX_HPP
#define GAME_ITEM_ITEMWATERPOTIONFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemWaterPotionFix : public DataFix
{
public:
    virtual ~ItemWaterPotionFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
