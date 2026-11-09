#include "game/world/block/VineBlock.hpp"

#include "game/core/FoliageColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

BooleanProperty* VineBlock::UP = static_cast<BooleanProperty*>(BooleanProperty::create(L"up"));
BooleanProperty* VineBlock::NORTH = static_cast<BooleanProperty*>(BooleanProperty::create(L"north"));
BooleanProperty* VineBlock::EAST = static_cast<BooleanProperty*>(BooleanProperty::create(L"east"));
BooleanProperty* VineBlock::SOUTH = static_cast<BooleanProperty*>(BooleanProperty::create(L"south"));
BooleanProperty* VineBlock::WEST = static_cast<BooleanProperty*>(BooleanProperty::create(L"west"));
BooleanProperty* VineBlock::ALL_FACES[5] = {UP, NORTH, SOUTH, WEST, EAST};
const AABB* VineBlock::UP_AABB = AABB::newPermanent(0.0, 0.9375, 0.0, 1.0, 1.0, 1.0);
const AABB* VineBlock::WEST_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.0625, 1.0, 1.0);
const AABB* VineBlock::EAST_AABB = AABB::newPermanent(0.9375, 0.0, 0.0, 1.0, 1.0, 1.0);
const AABB* VineBlock::NORTH_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.0625);
const AABB* VineBlock::SOUTH_AABB = AABB::newPermanent(0.0, 0.0, 0.9375, 1.0, 1.0, 1.0);

VineBlock::VineBlock() : Block(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(UP, false)
                             ->setValue(NORTH, false)
                             ->setValue(EAST, false)
                             ->setValue(SOUTH, false)
                             ->setValue(WEST, false));
    setTicking(true);
}

const AABB* VineBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL_AABB;
}

const AABB* VineBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actualState = state->fillVirtualBlockStateProperties(levelSource, pos);
    int faces = 0;
    const AABB* fullShape = Block::FULL_BLOCK_SHAPE;
    const AABB* shape = fullShape;
    if (actualState->getValue(UP))
    {
        shape = UP_AABB;
        ++faces;
    }
    if (actualState->getValue(NORTH))
    {
        shape = NORTH_AABB;
        ++faces;
    }
    if (actualState->getValue(EAST))
    {
        shape = EAST_AABB;
        ++faces;
    }
    if (actualState->getValue(SOUTH))
    {
        shape = SOUTH_AABB;
        ++faces;
    }
    if (actualState->getValue(WEST))
    {
        shape = WEST_AABB;
        ++faces;
    }
    return faces == 1 ? shape : fullShape;
}

const BlockState* VineBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    BlockPos above = pos.above();
    return state->setValue(UP, levelSource->getBlockState(above)->getBlockFaceShape(levelSource, above, Direction::DOWN)
                                   == BLOCK_FACE_SOLID);
}

bool VineBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

bool VineBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool VineBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

bool VineBlock::isExceptionBlockForAttaching(Block* block)
{
    return (block != NULL && dynamic_cast<ShulkerBoxBlock*>(block) != NULL) || block == Blocks::beacon
           || block == Blocks::cauldron || block == Blocks::glass || block == Blocks::stained_glass
           || block == Blocks::piston || block == Blocks::sticky_piston || block == Blocks::piston_head
           || block == Blocks::trapdoor || block == Blocks::spruce_trapdoor || block == Blocks::birch_trapdoor
           || block == Blocks::jungle_trapdoor || block == Blocks::acacia_trapdoor
           || block == Blocks::dark_oak_trapdoor;
}

bool VineBlock::isAcceptableNeighbor(Level* level, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = level->getBlockState(pos);
    return state->getBlockFaceShape(level, pos, direction) == BLOCK_FACE_SOLID
           && !isExceptionBlockForAttaching(state->getBlock());
}

bool VineBlock::canAttachBelow(Block* block)
{
    return block == Blocks::air || block == Blocks::water || block == Blocks::vine || block == Blocks::torch
           || block == Blocks::redstone_torch || block == Blocks::unlit_redstone_torch;
}

bool VineBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    Block* aboveBlock = level->getBlockState(pos.above())->getBlock();
    return isAcceptableNeighbor(level, pos.relative(direction->getOpposite()), direction)
           && (canAttachBelow(aboveBlock) || isAcceptableNeighbor(level, pos.above(), Direction::UP));
}

bool VineBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return direction != Direction::DOWN && direction != Direction::UP && canAttachTo(level, pos, direction);
}

AbstractProperty<bool>* VineBlock::getPropertyForFace(const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_Up:
        return UP;
    case Direction::eDirection_North:
        return NORTH;
    case Direction::eDirection_South:
        return SOUTH;
    case Direction::eDirection_West:
        return WEST;
    case Direction::eDirection_East:
        return EAST;
    default:
        return NULL;
    }
}

int VineBlock::countFaces(const BlockState* state)
{
    int faces = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (state->getValue(ALL_FACES[i]))
            ++faces;
    }
    return faces;
}

bool VineBlock::updateSurvival(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* oldState = state;
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        AbstractProperty<bool>* property = getPropertyForFace(direction);
        if (state->getValue(property) && !canAttachTo(level, pos, direction->getOpposite()))
        {
            const BlockState* aboveState = level->getBlockState(pos.above());
            if (aboveState->getBlock() != this || !aboveState->getValue(property))
            {
                state = state->setValue(property, false);
            }
        }
    }
    if (countFaces(state) == 0)
    {
        return false;
    }
    if (oldState != state)
    {
        level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
    }
    return true;
}

int VineBlock::getColor() const
{
    return FoliageColor::getDefaultColor();
}

int VineBlock::getColor(const BlockState* state)
{
    return FoliageColor::getDefaultColor();
}

int VineBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return getColor(levelSource, pos);
}

int VineBlock::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBiome(pos)->getFoliageColor(pos);
}

void VineBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                const BlockPos& fromPos)
{
    if (!level->mIsClientSide && !updateSurvival(level, pos, state))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

void VineBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->mIsClientSide && level->mRandom->nextInt(4) == 0)
    {
        int range = 4;
        int remaining = 5;
        bool crowded = false;
        for (int dx = -range; dx <= range; ++dx)
        {
            for (int dz = -range; dz <= range; ++dz)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (level->getBlockState(pos.offset(dx, dy, dz))->getBlock() == this)
                    {
                        --remaining;
                        if (remaining <= 0)
                        {
                            crowded = true;
                            break;
                        }
                    }
                }
                if (crowded)
                    break;
            }
            if (crowded)
                break;
        }

        const Direction* direction = Direction::getRandomDirection(*random);
        BlockPos above = pos.above();
        if (direction == Direction::UP && pos.y < 255 && level->isEmptyBlock(above))
        {
            const BlockState* newState = state;
            arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
            for (unsigned int i = 0; i < directions.length; ++i)
            {
                const Direction* face = directions.data[i];
                if (random->nextBoolean() && canAttachTo(level, above, face->getOpposite()))
                {
                    newState = newState->setValue(getPropertyForFace(face), true);
                }
                else
                {
                    newState = newState->setValue(getPropertyForFace(face), false);
                }
            }
            if (newState->getValue(NORTH) || newState->getValue(EAST) || newState->getValue(SOUTH)
                || newState->getValue(WEST))
            {
                level->setBlock(above, newState, Block::UPDATE_CLIENTS, false);
            }
        }
        else if (direction->getAxis()->isHorizontal() && !state->getValue(getPropertyForFace(direction)))
        {
            if (!crowded)
            {
                BlockPos target = pos.relative(direction);
                const BlockState* targetState = level->getBlockState(target);
                if (targetState->getBlock()->getCraftingMaterial() == Material::AIR)
                {
                    const Direction* clockWise = direction->getClockWise();
                    const Direction* counterClockWise = direction->getCounterClockWise();
                    bool hasClockWise = state->getValue(getPropertyForFace(clockWise));
                    bool hasCounterClockWise = state->getValue(getPropertyForFace(counterClockWise));
                    BlockPos clockWisePos = target.relative(clockWise);
                    BlockPos counterClockWisePos = target.relative(counterClockWise);
                    if (hasClockWise && canAttachTo(level, clockWisePos.relative(clockWise), clockWise))
                    {
                        level->setBlock(target, defaultBlockState()->setValue(getPropertyForFace(clockWise), true),
                                        Block::UPDATE_CLIENTS, false);
                    }
                    else if (hasCounterClockWise
                             && canAttachTo(level, counterClockWisePos.relative(counterClockWise), counterClockWise))
                    {
                        level->setBlock(target,
                                        defaultBlockState()->setValue(getPropertyForFace(counterClockWise), true),
                                        Block::UPDATE_CLIENTS, false);
                    }
                    else if (hasClockWise && level->isEmptyBlock(clockWisePos)
                             && canAttachTo(level, clockWisePos, direction))
                    {
                        level->setBlock(
                            clockWisePos,
                            defaultBlockState()->setValue(getPropertyForFace(direction->getOpposite()), true),
                            Block::UPDATE_CLIENTS, false);
                    }
                    else if (hasCounterClockWise && level->isEmptyBlock(counterClockWisePos)
                             && canAttachTo(level, counterClockWisePos, direction))
                    {
                        level->setBlock(
                            counterClockWisePos,
                            defaultBlockState()->setValue(getPropertyForFace(direction->getOpposite()), true),
                            Block::UPDATE_CLIENTS, false);
                    }
                }
                else if (targetState->getBlockFaceShape(level, target, direction) == BLOCK_FACE_SOLID)
                {
                    level->setBlock(pos, state->setValue(getPropertyForFace(direction), true), Block::UPDATE_CLIENTS,
                                    false);
                }
            }
        }
        else if (pos.y > 1)
        {
            BlockPos below = pos.below();
            const BlockState* belowState = level->getBlockState(below);
            Block* belowBlock = belowState->getBlock();
            if (belowBlock->getCraftingMaterial() == Material::AIR)
            {
                arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->getFaces();
                for (unsigned int i = 0; i < directions.length; ++i)
                {
                    if (random->nextBoolean())
                    {
                        state = state->setValue(getPropertyForFace(directions.data[i]), false);
                    }
                }
                if (state->getValue(NORTH) || state->getValue(EAST) || state->getValue(SOUTH) || state->getValue(WEST))
                {
                    level->setBlock(below, state, Block::UPDATE_CLIENTS, false);
                }
            }
            else if (belowBlock == this)
            {
                const BlockState* newState = belowState;
                arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
                for (unsigned int i = 0; i < directions.length; ++i)
                {
                    AbstractProperty<bool>* property = getPropertyForFace(directions.data[i]);
                    if (random->nextBoolean() && state->getValue(property))
                    {
                        newState = newState->setValue(property, true);
                    }
                }
                if (newState->getValue(NORTH) || newState->getValue(EAST) || newState->getValue(SOUTH)
                    || newState->getValue(WEST))
                {
                    level->setBlock(below, newState, Block::UPDATE_CLIENTS, false);
                }
            }
        }
    }
}

const BlockState* VineBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                  float hitX, float hitY, float hitZ, int auxValue,
                                                  boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState()
                                  ->setValue(UP, false)
                                  ->setValue(NORTH, false)
                                  ->setValue(EAST, false)
                                  ->setValue(SOUTH, false)
                                  ->setValue(WEST, false);
    if (direction->getAxis()->isHorizontal())
    {
        return state->setValue(getPropertyForFace(direction->getOpposite()), true);
    }
    return state;
}

Item* VineBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::air;
}

int VineBlock::getResourceCount(Random* random)
{
    return 0;
}

void VineBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                              const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                              not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        player->awardStat(GenericStats::blocksMined(this), GenericStats::param_blocksMined(state, 1));
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::vine, 1, 0)));
        return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

int VineBlock::getRenderLayer()
{
    return 0;
}

const BlockState* VineBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(SOUTH, (data & 1) > 0)
        ->setValue(WEST, (data & 2) > 0)
        ->setValue(NORTH, (data & 4) > 0)
        ->setValue(EAST, (data & 8) > 0);
}

int VineBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (state->getValue(VineBlock::SOUTH))
        data |= 1;
    if (state->getValue(VineBlock::WEST))
        data |= 2;
    if (state->getValue(VineBlock::NORTH))
        data |= 4;
    if (state->getValue(VineBlock::EAST))
        data |= 8;
    return data;
}

BlockStateDefinition* VineBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {UP, NORTH, EAST, SOUTH, WEST};
    return new BlockStateDefinition(this, properties, 5);
}

const BlockState* VineBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        return state->setValue(NORTH, state->getValue(SOUTH))
            ->setValue(EAST, state->getValue(WEST))
            ->setValue(SOUTH, state->getValue(NORTH))
            ->setValue(WEST, state->getValue(EAST));
    case Rotation::eTYPE_CounterClockwise90:
        return state->setValue(NORTH, state->getValue(EAST))
            ->setValue(EAST, state->getValue(SOUTH))
            ->setValue(SOUTH, state->getValue(WEST))
            ->setValue(WEST, state->getValue(NORTH));
    case Rotation::eTYPE_Clockwise90:
        return state->setValue(NORTH, state->getValue(WEST))
            ->setValue(EAST, state->getValue(NORTH))
            ->setValue(SOUTH, state->getValue(EAST))
            ->setValue(WEST, state->getValue(SOUTH));
    default:
        return state;
    }
}

const BlockState* VineBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        return state->setValue(NORTH, state->getValue(SOUTH))->setValue(SOUTH, state->getValue(NORTH));
    case Mirror::eTYPE_FrontBack:
        return state->setValue(EAST, state->getValue(WEST))->setValue(WEST, state->getValue(EAST));
    default:
        return Block::mirror(state, mirror);
    }
}

EBlockFaceShape VineBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                             const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape VineBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_VINE;
}

bool VineBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return !Minecraft::InMiniGame(eMiniGameId_Any, true);
}

bool VineBlock::waterSpreadCausesSpawn()
{
    return true;
}
