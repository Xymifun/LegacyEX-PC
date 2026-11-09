#ifndef GAME_ITEM_SKULLITEM_HPP
#define GAME_ITEM_SKULLITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class CompoundTag;
class Direction;
class Level;
class Player;

class SkullItem : public Item
{
public:
    SkullItem();

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual bool verifyTagAfterLoad(CompoundTag* tag);
};

#endif
