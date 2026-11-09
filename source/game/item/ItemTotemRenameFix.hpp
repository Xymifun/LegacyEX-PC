#ifndef GAME_ITEM_ITEMTOTEMRENAMEFIX_HPP
#define GAME_ITEM_ITEMTOTEMRENAMEFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemTotemRenameFix : public DataFix
{
public:
    virtual ~ItemTotemRenameFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
