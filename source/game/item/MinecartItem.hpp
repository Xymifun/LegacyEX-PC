#ifndef GAME_ITEM_MINECARTITEM_HPP
#define GAME_ITEM_MINECARTITEM_HPP

#include "game/core/Minecart.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/Item.hpp"

class BlockSource;

class MinecartItem : public Item
{
public:
    class MinecartDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~MinecartDispenseBehavior();
        virtual not_null_ptr<ItemInstance> execute(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                                   eOUTCOME& outcome);
        virtual void playSound(BlockSource* blockSource, eOUTCOME outcome);

    private:
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };

    MinecartItem(const Minecart::Type* type);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);

private:
    const Minecart::Type* mMinecartType;
};

#endif
