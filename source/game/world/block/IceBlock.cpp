#include "game/world/block/IceBlock.hpp"

#include "game/core/FoodConstants.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

IceBlock::IceBlock(bool isClientSide) : HalfTransparentBlock(L"ice", Material::ICE, false)
{
    if (!isClientSide)
    {
        DerivedInit();
    }
    mFriction = 0.98f;
    setTicking(true);
}

int IceBlock::getRenderLayer()
{
    return 1;
}

void IceBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                             const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                             not_null_ptr<ItemInstance> tool)
{
    Player* awardee = player.operator->();
    Stat* stat = gStats->get_blocksMined(Block::getId(this));
    awardee->awardStat(stat, GenericStats::param_blocksMined(state, 1));
    player->causeFoodExhaustion((float)FoodConstants::ExhaustionMine());

    if (isSilkTouchable() && EnchantmentHelper::getItemEnchantmentLevel(Enchantments::SILK_TOUCH, tool) > 0)
    {
        Block::popResource(level, pos, getSilkTouchItemInstance(state));
    }
    else
    {
        if (level->mDimension->isUltraWarm())
        {
            level->removeBlock(pos);
            return;
        }

        int fortune = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::BLOCK_FORTUNE, tool);
        spawnResources(level, pos, state, fortune);
        Material* material = level->getBlockState(pos.below())->getBlock()->getCraftingMaterial();
        if (material->blocksMotion() || material->isLiquid())
        {
            level->setBlockAndUpdate(pos, Blocks::flowing_water->defaultBlockState());
        }
    }
}

int IceBlock::getResourceCount(Random* random)
{
    return 0;
}

void IceBlock::melt(Level* level, const BlockPos& pos)
{
    if (level->mDimension->isUltraWarm())
    {
        level->removeBlock(pos);
        return;
    }

    level->setBlockAndUpdate(pos, Blocks::flowing_water->defaultBlockState());
    level->neighborChanged(pos, level->getBlock(pos), pos);
}

void IceBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (11 - defaultBlockState()->getLightBlock() < level->getBrightness(LightLayer::BLOCK, pos))
    {
        melt(level, pos);
    }
}

EPistonPushReaction IceBlock::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_NORMAL;
}

bool IceBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return level->getBrightness(LightLayer::BLOCK, pos) > 11 - state->getLightBlock();
}
