#ifndef GAME_ITEM_WORLDLYCONTAINER_HPP
#define GAME_ITEM_WORLDLYCONTAINER_HPP

#include "game/item/Container.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Direction;
class ItemInstance;

class WorldlyContainer : public virtual Container
{
public:
    WorldlyContainer();

    virtual arrayWithLength<int> getSlotsForFace(const Direction* face) = 0;
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face) = 0;
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face) = 0;
    virtual ~WorldlyContainer();
};

#endif
