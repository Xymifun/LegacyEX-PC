#include "game/item/SpawnEggItem.hpp"

#include <algorithm>

#include "game/core/StringUtils.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Difficulty.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/AABB.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/Dimension.hpp"
#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HitResult.hpp"

static const std::wstring TAG_ENTITY = L"EntityTag";

SpawnEggItem::SpawnEggItem() : Item()
{
    setMaxStackSize(64);
    setStackedByData(true);
    mOverlayIcon = NULL;
}

ResourceLocation SpawnEggItem::getNamedIdFrom(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL || !tag->contains(TAG_ENTITY, Tag::TAG_COMPOUND))
    {
        return ResourceLocation(std::wstring(L""));
    }

    CompoundTag* entityTag = tag->getCompound(TAG_ENTITY);
    if (entityTag->contains(L"id", Tag::TAG_STRING) == 1)
    {
        std::wstring id = entityTag->getString(L"id");
        ResourceLocation result(id);
        if (id.find(L':') == std::wstring::npos)
        {
            entityTag->putString(L"id", result.toString());
        }
        return ResourceLocation(result);
    }
    return ResourceLocation(std::wstring(L""));
}

std::wstring SpawnEggItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    std::wstring name = getDescription();
    int displayId = EntityIO::getNameDisplayId(getNamedIdFrom(itemInstance).toString());
    if (displayId != -1 && displayId != -2)
    {
        name = replaceAll(name, L"{*CREATURE*}", L10N::GetString(displayId));
    }
    else
    {
        name = replaceAll(name, L"{*CREATURE*}", L"");
    }
    return name;
}

double SpawnEggItem::getYOffset(Level* level, const BlockPos& pos)
{
    AABB* box = AABB::newPermanent(pos)->expand(0.0, -1.0, 0.0);
    std::vector<AABB*>* collisions = level->getCollisionAABBs(boost::shared_ptr<Entity>(), box, false, false, false);
    if (collisions->empty())
    {
        return 0.0;
    }
    double maxY = box->mMinY;
    for (std::vector<AABB*>::iterator it = collisions->begin(); it != collisions->end(); ++it)
    {
        maxY = std::max((*it)->mMaxY, maxY);
    }
    return maxY - (double)pos.y;
}

boost::shared_ptr<Entity> SpawnEggItem::canSpawn(const ResourceLocation& entityType, Level* level, int* error)
{
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(entityType, level);
    if (!entity)
        return boost::shared_ptr<Entity>();

    unsigned long long type = entity->GetType();
    if (type == EntityType::PHANTOM)
    {
        if (level->getDifficulty() == Difficulty::PEACEFUL)
        {
            *error = 12;
            return boost::shared_ptr<Entity>();
        }
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 10;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::DOLPHIN)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 9;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::SHULKER)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 7;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::BAT)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 3;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::MUSHROOM_COW)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 5;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::VILLAGER)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 8;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::WOLF)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 4;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (type == EntityType::CHICKEN)
    {
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 2;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (eTYPE_DERIVED_FROM(EntityType::WATER_ANIMAL, entity->GetType()))
    {
        type = entity->GetType();
        if ((type & EntityType::ENEMY) != 0 && type != EntityType::SHULKER
            && level->getDifficulty() == Difficulty::PEACEFUL)
        {
            *error = 12;
            return boost::shared_ptr<Entity>();
        }
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 11;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (eTYPE_DERIVED_FROM(EntityType::CREATURE_CATEGORY, entity->GetType()))
    {
        type = entity->GetType();
        if ((type & EntityType::ENEMY) != 0 && type != EntityType::SHULKER
            && level->getDifficulty() == Difficulty::PEACEFUL)
        {
            *error = 12;
            return boost::shared_ptr<Entity>();
        }
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 1;
            return boost::shared_ptr<Entity>();
        }
    }
    else if (eTYPE_DERIVED_FROM(EntityType::ENEMY, entity->GetType()))
    {
        if (level->getDifficulty() == Difficulty::PEACEFUL)
        {
            *error = 12;
            return boost::shared_ptr<Entity>();
        }
        type = entity->GetType();
        if ((type & EntityType::ENEMY) != 0 && type != EntityType::SHULKER
            && level->getDifficulty() == Difficulty::PEACEFUL)
        {
            *error = 12;
            return boost::shared_ptr<Entity>();
        }
        if (!level->canCreateMore(type, Level::eSpawnType_Egg))
        {
            *error = 7;
            return boost::shared_ptr<Entity>();
        }
    }
    else
    {
        return boost::shared_ptr<Entity>();
    }
    return entity;
}

boost::shared_ptr<Entity> SpawnEggItem::getMob(Level* level, const ResourceLocation& entityType, double x, double y,
                                               double z, int* error, const std::wstring& customName)
{
    if (entityType.toString().compare(L"") == 0 || EntityIO::sSpawnInfo.find(entityType) == EntityIO::sSpawnInfo.end())
        return boost::shared_ptr<Entity>();

    boost::shared_ptr<Entity> entity;
    entity = canSpawn(entityType, level, error);
    if (entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
        bool isShulker = eTYPE_DERIVED_FROM(EntityType::SHULKER, mob->GetType());
        if (isShulker && level->getBlock(BlockPos(x, y, z)) == Blocks::soul_sand)
            y += 0.125;
        entity->moveTo(x, y, z, Mth::wrapDegrees(level->mRandom->nextFloat() * 360.0f), 0.0f);
        entity->setDespawnProtected();
        mob->mYHeadRot = mob->mYRot;
        mob->mYBodyRot = mob->mYRot;
        mob->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(mob)), NULL, customName);
    }
    return entity;
}

boost::shared_ptr<Entity> SpawnEggItem::spawnMobAt(Level* level, const ResourceLocation& entityType, double x, double y,
                                                   double z, int* error, const std::wstring& customName)
{
    boost::shared_ptr<Entity> entity = getMob(level, entityType, x, y, z, error, customName);
    if (entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
        level->addEntity(mob);
        mob->playAmbientSound();
    }
    return entity;
}

void SpawnEggItem::updateCustomEntityTag(Level*, boost::shared_ptr<Player>, not_null_ptr<ItemInstance>,
                                         boost::shared_ptr<Entity>)
{
}

void SpawnEggItem::DisplaySpawnError(boost::shared_ptr<Player> player, int error)
{
    if (!player)
        return;

    switch (error)
    {
    case 1:
        player->displayClientMessage(IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED, true);
        return;
    case 2:
        player->displayClientMessage(IDS_MAX_CHICKENS_SPAWNED, true);
        return;
    case 3:
        player->displayClientMessage(IDS_MAX_BATS_SPAWNED, true);
        return;
    case 4:
        player->displayClientMessage(IDS_MAX_WOLVES_SPAWNED, true);
        return;
    case 5:
        player->displayClientMessage(IDS_MAX_MOOSHROOMS_SPAWNED, true);
        return;
    case 7:
        player->displayClientMessage(IDS_MAX_ENEMIES_SPAWNED, true);
        return;
    case 8:
        player->displayClientMessage(IDS_MAX_VILLAGERS_SPAWNED, true);
        return;
    case 12:
        player->displayClientMessage(IDS_CANT_SPAWN_IN_PEACEFUL, true);
        return;
    case 9:
        player->displayClientMessage(IDS_MAX_DOLPHIN_SPAWNED, true);
        return;
    case 10:
        player->displayClientMessage(IDS_MAX_PHANTOM_SPAWNED, true);
        return;
    case 11:
        player->displayClientMessage(IDS_MAX_WATER_ANIMALS_SPAWNED, true);
        break;
    default:
        break;
    }
}

InteractionResult::EInteractionResult SpawnEggItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                          const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                          const Direction* direction, float hitX, float hitY,
                                                          float hitZ, bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (level->mIsClientSide)
        return InteractionResult::eInteractionResult_Success;

    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
        return InteractionResult::eInteractionResult_Fail;

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    BlockPos spawnPos = pos.relative(direction);
    double yOffset = getYOffset(level, spawnPos);
    int error = 0;
    if (block == Blocks::mob_spawner)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isSpawner = eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType());
        if (isSpawner)
        {
            BaseMobSpawner* spawner = boost::static_pointer_cast<MobSpawnerBlockEntity>(blockEntity)->getSpawner();
            spawner->setEntityId(getNamedIdFrom(itemInstance));
            blockEntity->setChanged();
            level->sendBlockUpdated(pos, state, state, 3, true);
            if (!player->mAbilities.mInstabuild)
                itemInstance->shrink(1);

            boost::shared_ptr<Entity> entity = getMob(level, getNamedIdFrom(itemInstance), spawnPos.x + 0.5,
                                                      spawnPos.y + yOffset, spawnPos.z, &error, L"");
            if (entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
                boost::static_pointer_cast<Mob>(entity)->playAmbientSound();
            return InteractionResult::eInteractionResult_Success;
        }
    }

    boost::shared_ptr<Entity> entity = spawnMobAt(level, getNamedIdFrom(itemInstance), spawnPos.x + 0.5,
                                                  spawnPos.y + yOffset, spawnPos.z + 0.5, &error, L"");
    if (simulate)
        return entity ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Fail;

    if (entity)
    {
        if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()) && itemInstance->hasCustomHoverName())
        {
            boost::static_pointer_cast<Mob>(entity)->setCustomName(itemInstance->getHoverNameUnfiltered(),
                                                                   itemInstance->getOwnerUUID(),
                                                                   itemInstance->wasCreatedByRestrictedPlayer());
        }
        updateCustomEntityTag(level, player, itemInstance, entity);
        if (!player->mAbilities.mInstabuild)
            itemInstance->shrink(1);
    }
    else
    {
        DisplaySpawnError(player, error);
    }
    return InteractionResult::eInteractionResult_Success;
}

InteractionResultHolder SpawnEggItem::use(Level* level, boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    if (hitResult != NULL && !level->mDimension->inPlayerConstraints(BlockPos(hitResult->mLocation)))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    const BlockPos& pos = hitResult->getBlockPos();
    bool inWater = false;
    if (hitResult != NULL && level->getBlockState(pos)->getBlock()->instanceof(EBLOCK_TYPE_LIQUID)
        && level->getBlockState(pos)->getBlock()->mMaterial == Material::WATER)
    {
        inWater = true;
    }

    if (level->mIsClientSide)
    {
        delete hitResult;
        return InteractionResultHolder(inWater ? InteractionResult::eInteractionResult_Success
                                               : InteractionResult::eInteractionResult_Pass,
                                       itemInstance);
    }

    if (hitResult == NULL)
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);

    if (hitResult->mType != HitResult::BLOCK)
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (!level->getBlockState(pos)->getBlock()->instanceof(EBLOCK_TYPE_LIQUID))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (!level->mayInteract(player, pos, NULL) || !player->mayUseItemAt(pos, hitResult->mDirection, itemInstance))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    int error;
    boost::shared_ptr<Entity> entity = spawnMobAt(level, getNamedIdFrom(itemInstance), pos.x, pos.y, pos.z, &error,
                                                  L"");
    if (entity)
    {
        if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()) && itemInstance->hasCustomHoverName())
        {
            entity->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                  itemInstance->wasCreatedByRestrictedPlayer());
        }
        updateCustomEntityTag(level, player, itemInstance, entity);
        if (!player->mAbilities.mInstabuild)
            itemInstance->shrink(1);
        player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }

    DisplaySpawnError(player, error);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
}

void SpawnEggItem::addSpawnEntity(not_null_ptr<ItemInstance> itemInstance, const ResourceLocation& entityType)
{
    CompoundTag* tag = itemInstance->hasTag() ? itemInstance->getTag() : new CompoundTag();
    CompoundTag* entityTag = new CompoundTag();
    entityTag->putString(L"id", entityType.toString());
    tag->put(TAG_ENTITY, entityTag);
    itemInstance->setTag(tag);
}

bool SpawnEggItem::hasMultipleSpriteLayers()
{
    return true;
}

TextureAtlasSprite* SpawnEggItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer > 0)
    {
        return mOverlayIcon;
    }
    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

void SpawnEggItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mOverlayIcon = iconRegister->registerIcon(getIconName() + L"_overlay");
}

unsigned long long SpawnEggItem::fjGetType(const not_null_ptr<ItemInstance>& itemInstance)
{
    ResourceLocation id = getNamedIdFrom(itemInstance);
    if (EntityIO::isValidEntityType(id))
    {
        return EntityIO::byString(id.toString());
    }
    return 0;
}

bool SpawnEggItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    const BlockPos& pos = hitResult->getBlockPos();
    if (hitResult != NULL && level->getBlockState(pos)->getBlock()->instanceof(EBLOCK_TYPE_LIQUID)
        && level->getBlockState(pos)->getBlock()->mMaterial == Material::WATER)
    {
        return true;
    }
    return false;
}

unsigned int SpawnEggItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (TestUse((Level*)tooltipData.m_pLevel, (boost::shared_ptr<Player>&)tooltipData.m_pPlayer,
                InteractionHand::eInteractionHand_MainHand))
    {
        return IDS_TOOLTIPS_PLACE;
    }
    return IDS_NONE;
}
