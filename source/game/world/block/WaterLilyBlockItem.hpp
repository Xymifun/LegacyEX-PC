#ifndef GAME_WORLD_BLOCK_WATERLILYBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_WATERLILYBLOCKITEM_HPP

#include "game/core/InteractionResultHolder.hpp"
#include "game/world/block/TintedBlockItem.hpp"

class Block;
class ItemTooltipDataHolder;
class Level;
class Player;

class WaterLilyBlockItem : public TintedBlockItem
{
public:
    explicit WaterLilyBlockItem(Block* block);

    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
