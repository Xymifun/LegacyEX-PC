#ifndef GAME_ITEM_ITEMWRITTENBOOKPAGESSTRICTJSONFIX_HPP
#define GAME_ITEM_ITEMWRITTENBOOKPAGESSTRICTJSONFIX_HPP

#include "game/data/DataFix.hpp"
#include "game/data/CompoundTag.hpp"

class ItemWrittenBookPagesStrictJsonFix : public DataFix
{
public:
    virtual ~ItemWrittenBookPagesStrictJsonFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
