#ifndef GAME_ITEM_SPLASHPOTIONITEM_HPP
#define GAME_ITEM_SPLASHPOTIONITEM_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/PotionItem.hpp"

class HtmlString;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;

class SplashPotionItem : public PotionItem
{
public:
    virtual unsigned int getPotionType() const;
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual std::wstring getBottleTextureName();
};

#endif
