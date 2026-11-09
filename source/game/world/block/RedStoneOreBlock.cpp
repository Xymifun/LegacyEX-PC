#include "game/world/block/RedStoneOreBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

RedStoneOreBlock::RedStoneOreBlock(bool lit) : Block(Material::STONE)
{
    DerivedInit();
    if (lit)
    {
        setTicking(true);
    }

    mLit = lit;
}

int RedStoneOreBlock::getTickDelay(Level*)
{
    return 30;
}

void RedStoneOreBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
    interact(level, pos);
    Block::attack(level, pos, player);
}

void RedStoneOreBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    interact(level, pos);
    Block::stepOn(level, pos, entity);
}

bool RedStoneOreBlock::TestUse()
{
    return this == Blocks::redstone_ore;
}

bool RedStoneOreBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                           InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                           float hitZ, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }

    interact(level, pos);
    return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false);
}

void RedStoneOreBlock::spawnParticles(Level* level, const BlockPos& pos)
{
    Random* random = level->mRandom;
    double edge = 0.0625;

    for (int i = 0; i < 6; ++i)
    {
        double x = (double)((float)pos.x + random->nextFloat());
        double y = (double)((float)pos.y + random->nextFloat());
        double z = (double)((float)pos.z + random->nextFloat());

        if (i == 0 && !level->getBlockState(pos.above())->isSolidRender())
        {
            y = (double)pos.y + edge + 1.0;
        }
        if (i == 1 && !level->getBlockState(pos.below())->isSolidRender())
        {
            y = (double)pos.y - edge;
        }
        if (i == 2 && !level->getBlockState(pos.south())->isSolidRender())
        {
            z = (double)pos.z + edge + 1.0;
        }
        if (i == 3 && !level->getBlockState(pos.north())->isSolidRender())
        {
            z = (double)pos.z - edge;
        }
        if (i == 4 && !level->getBlockState(pos.east())->isSolidRender())
        {
            x = (double)pos.x + edge + 1.0;
        }
        if (i == 5 && !level->getBlockState(pos.west())->isSolidRender())
        {
            x = (double)pos.x - edge;
        }

        if (x < (double)pos.x || x > (double)(pos.x + 1) || y < 0.0 || y > (double)(pos.y + 1) || z < (double)pos.z
            || z > (double)(pos.z + 1))
        {
            level->addParticle(PARTICLE_RED_DUST, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }
}

void RedStoneOreBlock::interact(Level* level, const BlockPos& pos)
{
    spawnParticles(level, pos);
    if (!level->mIsClientSide && this == Blocks::redstone_ore)
    {
        level->setBlock(pos, Blocks::lit_redstone_ore->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
}

void RedStoneOreBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    if (this == Blocks::lit_redstone_ore)
    {
        level->setBlock(pos, Blocks::redstone_ore->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
}

Item* RedStoneOreBlock::getResource(const BlockState*, Random*, int)
{
    return Items::redstone;
}

int RedStoneOreBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return getResourceCount(random) + random->nextInt(lootBonus + 1);
}

int RedStoneOreBlock::getResourceCount(Random* random)
{
    return random->nextInt(2) + 4;
}

void RedStoneOreBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                      int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    if (getResource(state, level->mRandom, lootBonus) != Item::byBlock(this))
    {
        int amount = level->mRandom->nextInt(5) + 1;
        popExperience(level, pos, amount);
    }
}

void RedStoneOreBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (mLit)
    {
        spawnParticles(level, pos);
    }
}

unsigned int RedStoneOreBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_USE;
    }
    return Block::GetInteractTooltip(tooltipData);
}

not_null_ptr<ItemInstance> RedStoneOreBlock::getSilkTouchItemInstance(const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_ore));
}

not_null_ptr<ItemInstance> RedStoneOreBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::redstone_ore)));
}

bool RedStoneOreBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState*)
{
    return this == Blocks::lit_redstone_ore;
}
