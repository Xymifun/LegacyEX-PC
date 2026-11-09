#ifndef GAME_ITEM_CARROTONASTICKITEM_HPP
#define GAME_ITEM_CARROTONASTICKITEM_HPP

#include "game/item/Item.hpp"

class ItemTooltipDataHolder;
class Level;
class Player;

class CarrotOnAStickItem : public Item
{
public:
    CarrotOnAStickItem();
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool isHandEquipped();
    virtual bool isMirroredArt();
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
