#include "game/world/block/PistonHeadBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/generation/Player.hpp"

AbstractProperty<const Direction*>* PistonHeadBlock::FACING = NULL;
AbstractProperty<PistonHeadBlock::PistonType*>* PistonHeadBlock::TYPE = NULL;
AbstractProperty<bool>* PistonHeadBlock::SHORT = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"short"));

PistonHeadBlock::PistonType* pistonTypeNormal = new PistonHeadBlock::PistonType(L"normal");
PistonHeadBlock::PistonType* pistonTypeSticky = new PistonHeadBlock::PistonType(L"sticky");
static const AABB* pistonHeadShapeEast = AABB::newPermanent(0.75, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* pistonHeadShapeWest = AABB::newPermanent(0.0, 0.0, 0.0, 0.25, 1.0, 1.0);
static const AABB* pistonHeadShapeSouth = AABB::newPermanent(0.0, 0.0, 0.75, 1.0, 1.0, 1.0);
static const AABB* pistonHeadShapeNorth = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.25);
static const AABB* pistonHeadShapeUp = AABB::newPermanent(0.0, 0.75, 0.0, 1.0, 1.0, 1.0);
static const AABB* pistonHeadShapeDown = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0);
static const AABB* pistonArmShapeUp = AABB::newPermanent(0.375, -0.25, 0.375, 0.625, 0.75, 0.625);
static const AABB* pistonArmShapeDown = AABB::newPermanent(0.375, 0.25, 0.375, 0.625, 1.25, 0.625);
static const AABB* pistonArmShapeSouth = AABB::newPermanent(0.375, 0.375, -0.25, 0.625, 0.625, 0.75);
static const AABB* pistonArmShapeNorth = AABB::newPermanent(0.375, 0.375, 0.25, 0.625, 0.625, 1.25);
static const AABB* pistonArmShapeEast = AABB::newPermanent(0.25, 0.375, 0.375, 1.25, 0.625, 0.625);
static const AABB* pistonArmShapeWest = AABB::newPermanent(-0.25, 0.375, 0.375, 0.75, 0.625, 0.625);
static const AABB* pistonShortArmShapeUp = AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.75, 0.625);
static const AABB* pistonShortArmShapeDown = AABB::newPermanent(0.375, 0.25, 0.375, 0.625, 1.0, 0.625);
static const AABB* pistonShortArmShapeSouth = AABB::newPermanent(0.375, 0.375, 0.0, 0.625, 0.625, 0.75);
static const AABB* pistonShortArmShapeNorth = AABB::newPermanent(0.375, 0.375, 0.25, 0.625, 0.625, 1.0);
static const AABB* pistonShortArmShapeEast = AABB::newPermanent(0.0, 0.375, 0.375, 0.75, 0.625, 0.625);
static const AABB* pistonShortArmShapeWest = AABB::newPermanent(0.25, 0.375, 0.375, 1.0, 0.625, 0.625);

PistonHeadBlock::PistonType::PistonType(const std::wstring& name) : mName(name)
{
}

std::wstring PistonHeadBlock::PistonType::toString() const
{
    return mName;
}

std::wstring PistonHeadBlock::PistonType::getSerializedName() const
{
    return mName;
}

void PistonHeadBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
    std::wstring name(L"type");
    Predicate<PistonType*>* predicate = Predicates<PistonType*>::alwaysTrue;
    AbstractProperty<PistonType*>* type;
    {
        boost::unordered_set<PistonType*> values;
        values.insert(pistonTypeNormal);
        values.insert(pistonTypeSticky);
        type = EnumProperty<PistonType*>::create(name, typeid(PistonType*), values, predicate);
    }
    TYPE = type;
}

PistonHeadBlock::PistonHeadBlock() : Block(Material::PISTON)
{
    DerivedInit();
    setSoundType(SoundType::STONE);
    setDestroyTime(0.5f);
    mOverrideTopTexture = NULL;
}

const AABB* PistonHeadBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* facing = state->getValue(FACING);
    unsigned int data = facing->get3DDataValue();
    switch (data)
    {
    case 1:
        return pistonHeadShapeUp;
    case 2:
        return pistonHeadShapeNorth;
    case 3:
        return pistonHeadShapeSouth;
    case 4:
        return pistonHeadShapeWest;
    case 5:
        return pistonHeadShapeEast;
    default:
        return pistonHeadShapeDown;
    }
}

const AABB* PistonHeadBlock::getArmShape(const BlockState* state)
{
    bool shortened = state->getValue(SHORT);
    const Direction* facing = state->getValue(FACING);
    switch (facing->get3DDataValue())
    {
    case 1:
        return shortened ? pistonShortArmShapeUp : pistonArmShapeUp;
    case 2:
        return shortened ? pistonShortArmShapeNorth : pistonArmShapeNorth;
    case 3:
        return shortened ? pistonShortArmShapeSouth : pistonArmShapeSouth;
    case 4:
        return shortened ? pistonShortArmShapeWest : pistonArmShapeWest;
    case 5:
        return shortened ? pistonShortArmShapeEast : pistonArmShapeEast;
    default:
        return shortened ? pistonShortArmShapeDown : pistonArmShapeDown;
    }
}

void PistonHeadBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                        const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                        boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, state->getShape(level, pos));
    addCollisionAABB(pos, entityBox, collidingBoxes, getArmShape(state));
}

bool PistonHeadBlock::isTopSolidBlocking(const BlockState* state)
{
    return state->getValue(FACING) == Direction::UP;
}

bool PistonHeadBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                        boost::shared_ptr<Player> player)
{
    bool removedBase = false;
    if (player->mAbilities.mInstabuild)
    {
        const Direction* facing = state->getValue(FACING);
        if (facing != NULL)
        {
            BlockPos basePos = pos.relative(facing->getOpposite());
            Block* block = level->getBlockState(basePos)->getBlock();
            if (block == Blocks::piston || block == Blocks::sticky_piston)
            {
                level->removeBlock(basePos);
                removedBase = true;
            }
        }
    }
    return Block::playerWillDestroy(level, pos, state, player) || removedBase;
}

void PistonHeadBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    BlockPos basePos = pos.relative(state->getValue(FACING)->getOpposite());
    const BlockState* baseState = level->getBlockState(basePos);
    Block* baseBlock = baseState->getBlock();
    if (baseBlock == Blocks::piston || baseState->getBlock() == Blocks::sticky_piston)
    {
        if (baseState->getValue(PistonBaseBlock::EXTENDED))
        {
            baseState->getBlock()->spawnResources(level, basePos, baseState, 0);
            level->removeBlock(basePos);
        }
    }
}

bool PistonHeadBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool PistonHeadBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool PistonHeadBlock::mayPlace(Level*, const BlockPos&)
{
    return false;
}
bool PistonHeadBlock::mayPlace(Level*, const BlockPos&, const Direction*)
{
    return false;
}
int PistonHeadBlock::getResourceCount(Random*)
{
    return 0;
}

void PistonHeadBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                      const BlockPos& fromPos)
{
    BlockPos basePos = pos.relative(state->getValue(FACING)->getOpposite());
    const BlockState* baseState = level->getBlockState(basePos);
    if (baseState->getBlock() == Blocks::piston || baseState->getBlock() == Blocks::sticky_piston)
        baseState->getBlock()->neighborChanged(baseState, level, basePos, block, fromPos);
    else
        level->removeBlock(pos);
}

bool PistonHeadBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}

const Direction* PistonHeadBlock::getFacing(int data)
{
    int value = data & 7;
    if (value > 5)
        return NULL;
    return Direction::from3DDataValue(value);
}

not_null_ptr<ItemInstance> PistonHeadBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(state->getValue(TYPE) == pistonTypeSticky ? Blocks::sticky_piston : Blocks::piston));
}

const BlockState* PistonHeadBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue<const Direction*>(FACING, getFacing(data))
        ->setValue<PistonType*>(TYPE, (data & 8) > 0 ? pistonTypeSticky : pistonTypeNormal);
}

int PistonHeadBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(TYPE) == pistonTypeSticky)
        data |= 8;
    return data;
}

bool PistonHeadBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

const BlockState* PistonHeadBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* PistonHeadBlock::mirror(const BlockState* state, Mirror* mirror)
{
    AbstractProperty<const Direction*>* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* PistonHeadBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, TYPE, SHORT};
    return new BlockStateDefinition(this, properties, 3);
}

EBlockFaceShape PistonHeadBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&,
                                                   const Direction* direction)
{
    return direction == state->getValue(FACING) ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* PistonHeadBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction == facing)
    {
        if (mOverrideTopTexture != NULL)
            return mOverrideTopTexture;
        if (state->getValue(TYPE) == pistonTypeSticky)
            return PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_TOP_STICKY);
        return PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_TOP);
    }
    if (facing->get3DDataValue() < 6 && direction == facing->getOpposite())
        return PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_TOP);
    return PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_SIDE);
}

void PistonHeadBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape PistonHeadBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_PISTON_EXTENSION;
}
