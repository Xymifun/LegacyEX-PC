#ifndef GAME_ITEM_SEEDFOODITEM_HPP
#define GAME_ITEM_SEEDFOODITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/FoodItem.hpp"

class Block;
class BlockPos;
class Direction;
class Level;
class Player;

class SeedFoodItem : public FoodItem
{
public:
    SeedFoodItem(int nutrition, float saturationModifier, Block* cropBlock, Block* soilBlock);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);

private:
    Block* mCropBlock;
    Block* mSoilBlock;
};

#endif
