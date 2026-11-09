#ifndef GAME_ITEM_ITEMCOOKEDFISHIDFIX_HPP
#define GAME_ITEM_ITEMCOOKEDFISHIDFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemCookedFishIdFix : public DataFix
{
public:
    virtual ~ItemCookedFishIdFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
