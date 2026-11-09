#ifndef GAME_ITEM_DEBUGFOURJITEM_HPP
#define GAME_ITEM_DEBUGFOURJITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/util/ResourceLocation.hpp"

class BlockPos;
class Direction;
class Level;
class LivingEntity;
class Player;

class DebugFourjItem : public Item
{
public:
    DebugFourjItem();

    virtual bool interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);

private:
    ResourceLocation mLootTables[22];
};

#endif
