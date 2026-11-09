#ifndef GAME_ITEM_AXEITEM_HPP
#define GAME_ITEM_AXEITEM_HPP

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/DiggerItem.hpp"

class BlockPos;
class BlockState;
class Direction;
class ItemInstance;
class Level;
class Player;

class AxeItem : public DiggerItem
{
public:
    AxeItem(const Item::Tier* tier);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    static void staticCtor();
};

#endif
