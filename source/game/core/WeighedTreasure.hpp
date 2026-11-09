#ifndef GAME_CORE_WEIGHTEDTREASURE_HPP
#define GAME_CORE_WEIGHTEDTREASURE_HPP
#include <boost/shared_ptr.hpp>
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/WeighedRandomItem.hpp"
class Container;
class Item;
class ItemInstance;
class Random;
class WeighedTreasure : public WeighedRandomItem
{
public:
    WeighedTreasure(Item*, int, int, int, int);
    WeighedTreasure(not_null_ptr<ItemInstance>, int, int, int);
    not_null_ptr<ItemInstance> getItem();
    static void addChestItems(Random*, arrayWithLength<WeighedTreasure*>, boost::shared_ptr<Container>, int, bool);
    static void addToTreasure(arrayWithLength<WeighedTreasure*>, WeighedTreasure*);

private:
    not_null_ptr<ItemInstance> mItem;
    int mMinCount;
    int mMaxCount;
};
#endif
