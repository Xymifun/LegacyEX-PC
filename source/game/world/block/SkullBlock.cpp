#include "game/world/block/SkullBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockMaterialPredicate.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

bool isWitherSkull(BlockInWorld* block);

bool isWitherSkull(BlockInWorld* block)
{
    return block->getState() != NULL && block->getState()->getBlock() == Blocks::skull
           && eTYPE_DERIVED_FROM(BlockEntityType::SKULL, block->getEntity()->GetType())
           && boost::dynamic_pointer_cast<SkullBlockEntity>(block->getEntity())->getSkullType() == 1;
}

DirectionProperty* SkullBlock::FACING = NULL;
AbstractProperty<bool>* SkullBlock::NODROP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"nodrop"));

static Predicate<BlockInWorld*>* WITHER_SKULL_PREDICATE = new Predicates<BlockInWorld*>::FunctionPredicate(
    isWitherSkull);
static const AABB* SKULL_AABB = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 0.5, 0.75);
static const AABB* SKULL_NORTH_AABB = AABB::newPermanent(0.25, 0.25, 0.5, 0.75, 0.75, 1.0);
static const AABB* SKULL_SOUTH_AABB = AABB::newPermanent(0.25, 0.25, 0.0, 0.75, 0.75, 0.5);
static const AABB* SKULL_WEST_AABB = AABB::newPermanent(0.5, 0.25, 0.25, 1.0, 0.75, 0.75);
static const AABB* SKULL_EAST_AABB = AABB::newPermanent(0.0, 0.25, 0.25, 0.5, 0.75, 0.75);

SkullBlock::SkullBlock() : BaseEntityBlock(Material::DECORATION)
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(NODROP, false));
    mWitherBase = NULL;
    mWitherFull = NULL;
}

boost::shared_ptr<BlockEntity> SkullBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<SkullBlockEntity>(new SkullBlockEntity());
    entity->setLevel(level);
    return entity;
}

void SkullBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

std::wstring SkullBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_SKULL));
}

bool SkullBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool SkullBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool SkullBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

const AABB* SkullBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    default:
        return SKULL_AABB;
    case 2:
        return SKULL_NORTH_AABB;
    case 3:
        return SKULL_SOUTH_AABB;
    case 4:
        return SKULL_WEST_AABB;
    case 5:
        return SKULL_EAST_AABB;
    }
}

const BlockState* SkullBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float, float,
                                                   int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(FACING, placer->getDirection())->setValue(NODROP, false);
}

not_null_ptr<ItemInstance> SkullBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState*)
{
    int skullType = 0;
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (eTYPE_DERIVED_FROM(BlockEntityType::SKULL, entity->GetType()))
        skullType = boost::dynamic_pointer_cast<SkullBlockEntity>(entity)->getSkullType();
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, skullType));
}

void SkullBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
}

bool SkullBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player)
{
    if (player->mAbilities.mInstabuild)
    {
        state = state->setValue(NODROP, true);
        level->setBlock(pos, state, Block::UPDATE_INVISIBLE, false);
    }
    return Block::playerWillDestroy(level, pos, state, player);
}

void SkullBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide && !state->getValue(NODROP))
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (entity != NULL)
        {
            bool isSkull = eTYPE_DERIVED_FROM(BlockEntityType::SKULL, entity->GetType());
            if (isSkull)
            {
                boost::shared_ptr<SkullBlockEntity> skull = boost::dynamic_pointer_cast<SkullBlockEntity>(entity);
                not_null_ptr<ItemInstance> item = getCloneItemInstance(level, pos, state);
                Block::popResource(level, pos, item);
            }
        }
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

Item* SkullBlock::getResource(const BlockState*, Random*, int)
{
    return Items::skull;
}

BlockPattern* SkullBlock::getOrCreateWitherBase()
{
    if (mWitherBase == NULL)
    {
        static std::wstring aisle[] = {L"   ", L"###", L"~#~"};
        mWitherBase = BlockPatternBuilder::start()
                          ->aisle(aisle)
                          ->where('#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::soul_sand)))
                          ->where('~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
                          ->build(NULL, NULL);
    }
    return mWitherBase;
}

BlockPattern* SkullBlock::getOrCreateWitherFull()
{
    if (mWitherFull == NULL)
    {
        static std::wstring aisle[] = {L"^^^", L"###", L"~#~"};
        mWitherFull = BlockPatternBuilder::start()
                          ->aisle(aisle)
                          ->where('#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::soul_sand)))
                          ->where('^', WITHER_SKULL_PREDICATE)
                          ->where('~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
                          ->build(NULL, NULL);
    }
    return mWitherFull;
}

bool SkullBlock::canSpawnMob(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> item)
{
    if (item->getAuxValue() == 1 && pos.y >= 2 && level->getDifficulty() != Difficulty::PEACEFUL
        && !level->mIsClientSide)
    {
        BlockPattern::BlockPatternMatch* match = getOrCreateWitherBase()->find(level, pos);
        bool found = match != NULL;
        delete match;
        return found;
    }
    return false;
}

void SkullBlock::checkMobSpawn(Level* level, const BlockPos& pos, boost::shared_ptr<SkullBlockEntity> skull,
                               boost::shared_ptr<Player> player)
{
    if (skull->getSkullType() != 1 || pos.y < 2 || level->mIsClientSide)
        return;

    BlockPattern* pattern = getOrCreateWitherFull();
    BlockPattern::BlockPatternMatch* match = pattern->find(level, pos);
    if (match != NULL)
    {
        if (level->getDifficulty() == Difficulty::PEACEFUL)
        {
            if (player)
                player->displayClientMessage(IDS_CANT_SPAWN_IN_PEACEFUL, true);
            return;
        }

        bool canCreate = level->canCreateMore(EntityType::WITHER_BOSS, Level::eSpawnType_Egg);
        for (int i = 0; i < 3; ++i)
        {
            BlockInWorld* block = match->getBlock(i, 0, 0);
            if (!canCreate)
                level->setBlock(block->getPos(), block->getState(), Block::UPDATE_CLIENTS, false);
            else
                level->setBlock(block->getPos(), block->getState()->setValue(NODROP, true), Block::UPDATE_CLIENTS,
                                false);
        }

        for (int x = 0; x < static_cast<int>(pattern->getWidth()); ++x)
        {
            for (int y = 0; y < static_cast<int>(pattern->getHeight()); ++y)
            {
                BlockInWorld* block = match->getBlock(x, y, 0);
                if (!canCreate)
                    block->getState()->getBlock()->spawnResources(level, block->getPos(), block->getState(), 0);
                level->setBlock(block->getPos(), Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            }
        }

        if (canCreate)
        {
            boost::shared_ptr<WitherBoss> wither(new WitherBoss(level));
            BlockPos spawnPos = match->getBlock(1, 2, 0)->getPos();
            wither->moveTo(spawnPos.x + 0.5, spawnPos.y + 0.55, spawnPos.z + 0.5,
                           match->getForwards()->getAxis() == Direction::Axis::X ? 0.0f : 90.0f, 0.0f);
            wither->mYBodyRot = match->getForwards()->getAxis() == Direction::Axis::X ? 0.0f : 90.0f;
            wither->makeInvulnerable();

            std::vector<boost::shared_ptr<Entity> >* players = level->getEntitiesOfClass(
                typeid(Player), wither->mBoundingBox->grow(50.0, 50.0, 50.0));
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
            {
                boost::shared_ptr<Player> nearby = boost::static_pointer_cast<Player>(*it);
                nearby->awardStat(GenericStats::spawnWither(), GenericStats::param_spawnWither());
            }
            delete players;
            level->addEntity(wither);
        }

        for (int i = 0; i < 120; ++i)
        {
            BlockPos particlePos = match->getBlock(1, 0, 0)->getPos();
            level->addParticle(PARTICLE_SNOWBALL_POOF, particlePos.x + level->mRandom->nextDouble(),
                               (particlePos.y - 2) + level->mRandom->nextDouble() * 3.9,
                               particlePos.z + level->mRandom->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>());
        }

        for (int x = 0; x < static_cast<int>(pattern->getWidth()); ++x)
        {
            for (int y = 0; y < static_cast<int>(pattern->getHeight()); ++y)
                level->blockUpdated(match->getBlock(x, y, 0)->getPos(), Blocks::air, false);
        }
    }

    delete match;
}

const BlockState* SkullBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::from3DDataValue(data & 7))
        ->setValue(NODROP, (data & 8) > 0);
}

int SkullBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(NODROP))
        data |= 8;
    return data;
}

bool SkullBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool SkullBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

const BlockState* SkullBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* SkullBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* SkullBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, NODROP};
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape SkullBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void SkullBlock::registerIcons(IconRegister*)
{
}

TextureAtlasSprite* SkullBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return Blocks::soul_sand->getTexture(direction);
}

std::wstring SkullBlock::getTileItemIconName()
{
    return L"";
}

EBlockRenderShape SkullBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_INVISIBLE;
}
