#ifndef GAME_ITEM_SHOVELITEM_HPP
#define GAME_ITEM_SHOVELITEM_HPP

#include "game/item/DiggerItem.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"

class BlockPos;
class BlockState;
class Direction;
class Level;
class Player;

class ShovelItem : public DiggerItem
{
public:
    ShovelItem(const Item::Tier* tier);

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual bool canDestroySpecial(const BlockState* state);

    static void staticCtor();
};

#endif
