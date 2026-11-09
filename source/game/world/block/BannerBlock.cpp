#include "game/world/block/BannerBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/L10N.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"

DirectionProperty* BannerBlock::FACING = NULL;

BannerBlock::BannerBlock() : BaseEntityBlock(Material::WOOD)
{
}

boost::shared_ptr<BlockEntity> BannerBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<BannerBlockEntity>(new BannerBlockEntity());
    entity->setLevel(level);
    return entity;
}

void BannerBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

std::wstring BannerBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_WALL_BANNER));
}

const AABB* BannerBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool BannerBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool BannerBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return true;
}

bool BannerBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool BannerBlock::isPossibleToRespawnInThis()
{
    return true;
}

Item* BannerBlock::getResource(const BlockState*, Random*, int)
{
    return Items::banner;
}

not_null_ptr<ItemInstance> BannerBlock::getItemInstanceFromBlockEntity(Level* level, const BlockPos& pos,
                                                                       const BlockState*)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<BannerBlockEntity> bannerEntity = boost::dynamic_pointer_cast<BannerBlockEntity>(blockEntity);
    if (bannerEntity != NULL)
        return bannerEntity->getItem();
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BannerBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    not_null_ptr<ItemInstance> item = getItemInstanceFromBlockEntity(level, pos, state);
    return item->isEmpty()
               ? not_null_ptr<ItemInstance>(boost::shared_ptr<ItemInstance>(new ItemInstance(Items::banner)))
               : item;
}

void BannerBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                 int lootBonus)
{
    not_null_ptr<ItemInstance> item = getItemInstanceFromBlockEntity(level, pos, state);
    if (item->isEmpty())
        Block::spawnResources(level, pos, state, chance, lootBonus);
    else
        Block::popResource(level, pos, item);
}

bool BannerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return !touchesCactus(level, pos) && Block::mayPlace(level, pos);
}

EPistonPushReaction BannerBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}

void BannerBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                not_null_ptr<ItemInstance> tool)
{
    boost::shared_ptr<BannerBlockEntity> bannerEntity = boost::dynamic_pointer_cast<BannerBlockEntity>(blockEntity);
    if (bannerEntity != NULL)
    {
        not_null_ptr<ItemInstance> item = bannerEntity->getItem();
        Block::popResource(level, pos, item);
    }
    else
    {
        BaseEntityBlock::playerDestroy(level, player, pos, state, boost::shared_ptr<BlockEntity>(), tool);
    }
}

EBlockFaceShape BannerBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
