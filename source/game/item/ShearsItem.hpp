#ifndef GAME_ITEM_SHEARSITEM_HPP
#define GAME_ITEM_SHEARSITEM_HPP

#include "game/item/Item.hpp"

class ShearsItem : public Item
{
public:
    ShearsItem();
    virtual bool mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity);
    virtual bool canDestroySpecial(const BlockState* state);
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
