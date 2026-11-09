#ifndef GAME_ITEM_ITEMPOTIONFIX_HPP
#define GAME_ITEM_ITEMPOTIONFIX_HPP
#include <stdint.h>
#include "game/data/DataFix.hpp"
#include "game/item/ItemInstance.hpp"
class ItemPotionFix : public DataFix
{
public:
    virtual ~ItemPotionFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
    static void staticCtor();
    static void fix(not_null_ptr<ItemInstance> item);
};
#endif
