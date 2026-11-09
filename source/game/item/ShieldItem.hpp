#ifndef GAME_ITEM_SHIELDITEM_HPP
#define GAME_ITEM_SHIELDITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class HtmlString;
class ItemInstance;
class Level;
class Player;

class ShieldItem : public Item
{
public:
    ShieldItem();

    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);
};

#endif
