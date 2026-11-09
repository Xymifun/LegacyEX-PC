#ifndef GAME_ITEM_ITEMBANNERCOLORFIX_HPP
#define GAME_ITEM_ITEMBANNERCOLORFIX_HPP

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemBannerColorFix : public DataFix
{
public:
    virtual ~ItemBannerColorFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
