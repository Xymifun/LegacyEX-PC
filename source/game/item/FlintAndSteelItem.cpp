#include "game/item/FlintAndSteelItem.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PortalBlock.hpp"

FlintAndSteelItem::FlintAndSteelItem() : Item()
{
    mMaxStackSize = 1;
    setMaxDamage(0x40);
}

InteractionResult::EInteractionResult FlintAndSteelItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                               const BlockPos& pos,
                                                               InteractionHand::EInteractionHand hand,
                                                               const Direction* direction, float hitX, float hitY,
                                                               float hitZ, bool simulate)
{
    BlockPos firePos = pos;
    firePos = firePos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(firePos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        if (level->getBlockState(firePos)->getBlock()->mMaterial == Material::AIR)
        {
            if (static_cast<PortalBlock*>(Blocks::portal)->trySpawnPortal(level, firePos, false))
            {
                player->awardStat(GenericStats::InToTheNether(), GenericStats::param_InToTheNether());
            }
            level->playSound(player, firePos, SoundEvents::FLINTANDSTEEL_USE, SoundSource::eSoundSource_Blocks, 1.0f,
                             random->nextFloat() * 0.4f + 0.8f, 16.0f);
            level->setBlock(firePos, Blocks::fire->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE,
                            false);
        }
        itemInstance->hurtAndBreak(1, player);
    }
    else
    {
        return (level->getBlockState(firePos)->getBlock()->mMaterial == Material::AIR
                && Blocks::fire->mayPlace(level, firePos))
                   ? InteractionResult::eInteractionResult_Success
                   : InteractionResult::eInteractionResult_Fail;
    }

    return InteractionResult::eInteractionResult_Success;
}
