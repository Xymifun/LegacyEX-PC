#ifndef GAME_ITEM_FLINTANDSTEELITEM_HPP
#define GAME_ITEM_FLINTANDSTEELITEM_HPP

#include "game/item/Item.hpp"

class FlintAndSteelItem : public Item
{
public:
    FlintAndSteelItem();
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
