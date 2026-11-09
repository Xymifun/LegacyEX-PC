#ifndef GAME_ITEM_DEFAULTDISPENSEITEMBEHAVIOR_HPP
#define GAME_ITEM_DEFAULTDISPENSEITEMBEHAVIOR_HPP

#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockSource;
class Direction;
class ItemInstance;
class Level;
class Position;

class DefaultDispenseItemBehavior : public DispenseItemBehavior
{
public:
    enum eOUTCOME
    {
        eOUTCOME_Success,
        eOUTCOME_Default,
        eOUTCOME_Failure
    };

    static const unsigned int MAX_PROJECTILES = 300;

    virtual ~DefaultDispenseItemBehavior() {}

    virtual not_null_ptr<ItemInstance> dispense(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance);
    virtual not_null_ptr<ItemInstance> execute(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                               eOUTCOME& outcome);
    virtual void playSound(BlockSource* blockSource, eOUTCOME outcome);
    virtual void playAnimation(BlockSource* blockSource, const Direction* direction, eOUTCOME outcome);
    virtual int getLevelEventDataFrom(const Direction* direction);

    static void spawnItem(Level* level, not_null_ptr<ItemInstance> itemInstance, int power, const Direction* direction,
                          Position* position);
};

#endif
