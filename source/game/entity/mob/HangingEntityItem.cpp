#include "game/entity/mob/HangingEntityItem.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/Painting.hpp"
#include "game/core/Stats.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/item/Items.hpp"

HangingEntityItem::HangingEntityItem(unsigned long long entityType) : Item(), mEntityType(entityType)
{
}

boost::shared_ptr<HangingEntity> HangingEntityItem::createEntity(Level* level, const BlockPos& pos,
                                                                 const Direction* direction, int auxValue)
{
    if (mEntityType == EntityType::PAINTING)
    {
        boost::shared_ptr<Painting> painting(new Painting(level, pos, direction));
        painting->PaintingPostConstructor(direction, -1);
        return boost::static_pointer_cast<HangingEntity>(painting);
    }
    else if (mEntityType == EntityType::ITEM_FRAME)
    {
        boost::shared_ptr<ItemFrame> itemFrame(new ItemFrame(level, pos, direction));
        level->updateObservingBlocksAt(pos, Blocks::air);
        return boost::static_pointer_cast<HangingEntity>(itemFrame);
    }
    return boost::shared_ptr<HangingEntity>();
}

InteractionResult::EInteractionResult HangingEntityItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                               const BlockPos& pos,
                                                               InteractionHand::EInteractionHand hand,
                                                               const Direction* direction, float hitX, float hitY,
                                                               float hitZ, bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    BlockPos placePos = pos.relative(direction);

    if (direction == Direction::DOWN || direction == Direction::UP
        || !player->mayUseItemAt(placePos, direction, itemInstance))
        return InteractionResult::eInteractionResult_Fail;

    if (simulate)
    {
        const BlockState* state = level->getBlockState(pos);
        if (!state->getMaterial()->isSolid() && !DiodeBlock::isDiode(state))
            return InteractionResult::eInteractionResult_Fail;
        return InteractionResult::eInteractionResult_Success;
    }

    boost::shared_ptr<HangingEntity> entity = createEntity(level, placePos, direction, itemInstance->getAuxValue());
    if (entity != NULL && entity->survives())
    {
        if (!level->mIsClientSide)
        {
            if (level->addEntity(entity))
            {
                entity->playPlacementSound();

                Block* block = NULL;
                if (mEntityType == EntityType::PAINTING)
                    block = Block::byItem(Items::painting);
                else if (mEntityType == EntityType::ITEM_FRAME)
                    block = Block::byItem(Items::itemFrame);
                if (block != NULL)
                {
                    const BlockState* state = block->getBlockState(itemInstance->getAuxValue());
                    player->awardStat(GenericStats::blocksPlaced(block), GenericStats::param_blocksPlaced(state, 1));
                }
            }
            else
            {
                player->displayClientMessage(0x1EE681BA, true);
                return InteractionResult::eInteractionResult_Fail;
            }
        }

        itemInstance->shrink(1);
    }
    return InteractionResult::eInteractionResult_Success;
}

void HangingEntityItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                        std::vector<HtmlString>* lines, bool advanced)
{
}
