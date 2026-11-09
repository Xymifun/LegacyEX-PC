#ifndef GAME_ITEM_FIREWORKSITEM_HPP
#define GAME_ITEM_FIREWORKSITEM_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockPos;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;

class FireworksItem : public Item
{
public:
    static const std::wstring TAG_FIREWORKS;
    static const std::wstring TAG_EXPLOSION;
    static const std::wstring TAG_EXPLOSIONS;
    static const std::wstring TAG_FLIGHT;
    static const std::wstring TAG_E_TYPE;
    static const std::wstring TAG_E_TRAIL;
    static const std::wstring TAG_E_FLICKER;
    static const std::wstring TAG_E_COLORS;
    static const std::wstring TAG_E_FADECOLORS;

    FireworksItem();

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
