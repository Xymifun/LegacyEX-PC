#ifndef GAME_ITEM_LEADITEM_HPP
#define GAME_ITEM_LEADITEM_HPP

#include "game/item/Item.hpp"

class BlockPos;
class Level;

class LeadItem : public Item
{
public:
    LeadItem();
    static bool bindPlayerMobsTest(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos);
    static bool bindPlayerMobs(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
