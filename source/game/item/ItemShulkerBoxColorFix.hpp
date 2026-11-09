#ifndef GAME_ITEM_ITEMSHULKERBOXCOLORFIX_HPP
#define GAME_ITEM_ITEMSHULKERBOXCOLORFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemShulkerBoxColorFix : public DataFix
{
public:
    virtual ~ItemShulkerBoxColorFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
