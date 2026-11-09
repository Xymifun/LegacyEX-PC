#include "game/world/block/FarmBlock.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/StemBlock.hpp"

AbstractProperty<int>* FarmBlock::MOISTURE = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"moisture", 0, 7));
static const AABB* gFarmShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.9375, 1.0);

FarmBlock::FarmBlock() : Block(Material::DIRT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(MOISTURE, 0));
    setTicking(true);
    setLightBlock(0xff);
}

const AABB* FarmBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gFarmShape;
}
bool FarmBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool FarmBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void FarmBlock::turnToDirt(Level* level, const BlockPos& pos)
{
    const BlockState* dirt = Blocks::dirt->defaultBlockState();
    level->setBlockAndUpdate(pos, dirt);

    AABB* box = dirt->getClipAABB(level, pos)->move(pos);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(boost::shared_ptr<Entity>(), box, NULL);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity>& entity = *it;
        entity->teleportTo(entity->mX, box->mMaxY, entity->mZ);
    }
}

void FarmBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    int moisture = state->getValue(MOISTURE);
    if (isNearWater(level, pos) || level->isRainingAt(pos.above()))
    {
        if (moisture < 7)
            level->setBlock(pos, state->setValue(MOISTURE, 7), Block::UPDATE_CLIENTS, false);
    }
    else if (moisture > 0)
        level->setBlock(pos, state->setValue(MOISTURE, moisture - 1), Block::UPDATE_CLIENTS, false);
    else if (!isUnderCrops(level, pos))
        turnToDirt(level, pos);
}

void FarmBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    if (!level->mIsClientSide && level->mRandom->nextFloat() < fallDistance - 0.5f)
    {
        bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType());
        if (isLiving)
        {
            bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());
            if (isPlayer || level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
            {
                if (entity->mBbWidth * entity->mBbWidth * entity->mBbHeight > 0.512f)
                    turnToDirt(level, pos);
            }
        }
    }
    Block::fallOn(level, pos, entity, fallDistance);
}

bool FarmBlock::isUnderCrops(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlockState(pos.above())->getBlock();
    return dynamic_cast<CropBlock*>(block) != NULL || dynamic_cast<StemBlock*>(block) != NULL;
}

bool FarmBlock::isNearWater(Level* level, const BlockPos& pos)
{
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(pos.offset(-4, 0, -4), pos.offset(4, 1, 4));
    while (positions->hasNext())
    {
        BlockPos current = *positions->next();
        if (level->getBlockState(current)->getBlock()->mMaterial == Material::WATER)
        {
            delete positions;
            return true;
        }
    }
    delete positions;
    return false;
}

void FarmBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
    if (level->getBlockState(pos.above())->getBlock()->mMaterial->isSolid())
        turnToDirt(level, pos);
}

void FarmBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    if (level->getBlockState(pos.above())->getMaterial()->isSolid())
        turnToDirt(level, pos);
}

bool FarmBlock::shouldRenderFace(const BlockState* state, LevelSource* source, const BlockPos& pos,
                                 const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_Down:
        return Block::shouldRenderFace(state, source, pos, direction);
    case Direction::eDirection_Up:
        return true;
    case Direction::eDirection_North:
    case Direction::eDirection_South:
    case Direction::eDirection_West:
    case Direction::eDirection_East:
    {
        const BlockState* neighbour = source->getBlockState(pos.relative(direction));
        Block* block = neighbour->getBlock();
        return !(neighbour->isSolidRender() || block == Blocks::farmland || block == Blocks::grass_path);
    }
    }
    return Block::shouldRenderFace(state, source, pos, direction);
}

Item* FarmBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT), random, lootBonus);
}

const BlockState* FarmBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(MOISTURE, data & 7);
}

int FarmBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(MOISTURE);
}
bool FarmBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

not_null_ptr<ItemInstance> FarmBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(Item::byBlock(Blocks::dirt), 1, getSpawnResourcesAuxValue(state)));
}

BlockStateDefinition* FarmBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {MOISTURE};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape FarmBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                             const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* FarmBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
        return state->getValue(MOISTURE) > 0 ? mWetIcon : mDryIcon;
    return Blocks::dirt->getTexture(direction);
}

void FarmBlock::registerIcons(IconRegister* iconRegister)
{
    mWetIcon = iconRegister->registerIcon(L"farmland_wet");
    mDryIcon = iconRegister->registerIcon(L"farmland_dry");
}
