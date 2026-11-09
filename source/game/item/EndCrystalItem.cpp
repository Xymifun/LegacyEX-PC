#include "game/item/EndCrystalItem.hpp"

#include <vector>

#include "game/core/EnderCrystal.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/StringIds.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

EndCrystalItem::EndCrystalItem() : Item()
{
    setIconName(std::wstring(L"end_crystal"));
}

InteractionResult::EInteractionResult EndCrystalItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                            const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                            const Direction* direction, float hitX, float hitY,
                                                            float hitZ, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != Blocks::obsidian && state->getBlock() != Blocks::bedrock)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos abovePos = pos.above();
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(abovePos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos aboveAbovePos = abovePos.above();
    bool blocked = !level->isEmptyBlock(abovePos)
                   && !level->getBlockState(abovePos)->getBlock()->mayReplaceWithPlace(level, abovePos);
    blocked = blocked
              | (!level->isEmptyBlock(aboveAbovePos)
                 && !level->getBlockState(aboveAbovePos)->getBlock()->mayReplaceWithPlace(level, aboveAbovePos));
    if (blocked)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    double x = abovePos.x;
    double y = abovePos.y;
    double z = abovePos.z;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        boost::shared_ptr<Entity>(), AABB::newTemp(x, y, z, x + 1.0, y + 2.0, z + 1.0), NULL);
    if (!entities->empty())
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        if (!level->mIsClientSide)
        {
            if (level->canCreateMore(EntityType::ENDER_CRYSTAL, Level::eSpawnType_Egg))
            {
                {
                    boost::shared_ptr<EnderCrystal> crystal(
                        new EnderCrystal(level, pos.x + 0.5f, pos.y + 1, pos.z + 0.5f));
                    crystal->setShowBottom(false);
                    level->addEntity(crystal);
                }

                if (dynamic_cast<TheEndDimension*>(level->mDimension) != NULL)
                {
                    static_cast<TheEndDimension*>(level->mDimension)->getDragonFight()->tryRespawn();
                }
            }
            else
            {
                player->displayClientMessage(IDS_MAX_ENDER_CRYSTALS, true);
                return InteractionResult::eInteractionResult_Fail;
            }
        }

        itemInstance->shrink(1);
    }
    return InteractionResult::eInteractionResult_Success;
}

bool EndCrystalItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}
