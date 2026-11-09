#include "game/item/ShovelItem.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Direction.hpp"

static arrayWithLength<Block*> BLOCKS(12, false);

void ShovelItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::clay;
    BLOCKS.data[1] = Blocks::dirt;
    BLOCKS.data[2] = Blocks::farmland;
    BLOCKS.data[3] = Blocks::grass;
    BLOCKS.data[4] = Blocks::gravel;
    BLOCKS.data[5] = Blocks::mycelium;
    BLOCKS.data[6] = Blocks::sand;
    BLOCKS.data[7] = Blocks::snow;
    BLOCKS.data[8] = Blocks::snow_layer;
    BLOCKS.data[9] = Blocks::soul_sand;
    BLOCKS.data[10] = Blocks::grass_path;
    BLOCKS.data[11] = Blocks::concrete_powder;
}

ShovelItem::ShovelItem(const Item::Tier* tier) : DiggerItem(1.0f, -3.0f, tier, BLOCKS)
{
}

bool ShovelItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();
    if (block == Blocks::snow_layer || block == Blocks::snow)
    {
        return true;
    }
    return false;
}

InteractionResult::EInteractionResult ShovelItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    Block* block = level->getBlockState(pos)->getBlock();
    if (direction == Direction::DOWN || level->getBlockState(pos.above())->getMaterial() != Material::AIR
        || block != Blocks::grass)
    {
        return InteractionResult::eInteractionResult_Pass;
    }

    if (!simulate)
    {
        level->playSound(player, pos, SoundEvents::SHOVEL_FLATTEN, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
        if (!level->mIsClientSide)
        {
            const BlockState* pathState = Blocks::grass_path->defaultBlockState();
            level->setBlock(pos, pathState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            itemInstance->hurtAndBreak(1, player);
        }
    }
    return InteractionResult::eInteractionResult_Success;
}
