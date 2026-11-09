#include "game/world/block/MobSpawnerBlock.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"

MobSpawnerBlock::MobSpawnerBlock() : BaseEntityBlock(Material::STONE)
{
    DerivedInit();
}

boost::shared_ptr<BlockEntity> MobSpawnerBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<MobSpawnerBlockEntity>(new MobSpawnerBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

Item* MobSpawnerBlock::getResource(const BlockState*, Random*, int)
{
    return NULL;
}

int MobSpawnerBlock::getResourceCount(Random*)
{
    return 0;
}

void MobSpawnerBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                     int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    int experience = level->mRandom->nextInt(15) + 15 + level->mRandom->nextInt(15);
    popExperience(level, pos, experience);
}

bool MobSpawnerBlock::isSolidRender(const BlockState*) const
{
    return false;
}

EBlockRenderShape MobSpawnerBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

int MobSpawnerBlock::getRenderLayer()
{
    return 0;
}

not_null_ptr<ItemInstance> MobSpawnerBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::mob_spawner)));
}

bool MobSpawnerBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

bool MobSpawnerBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return false;
}
