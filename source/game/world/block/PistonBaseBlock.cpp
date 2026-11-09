#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/BlockId.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/generation/PistonStructureResolver.hpp"
#include "game/entity/mob/LivingEntity.hpp"

AbstractProperty<const Direction*>* PistonBaseBlock::FACING = NULL;
AbstractProperty<bool>* PistonBaseBlock::EXTENDED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"extended"));
const std::wstring PistonBaseBlock::TEXTURE_SIDE = L"piston_side";
const std::wstring PistonBaseBlock::TEXTURE_TOP = L"piston_top";
const std::wstring PistonBaseBlock::TEXTURE_TOP_STICKY = L"piston_top_sticky";
const std::wstring PistonBaseBlock::TEXTURE_BOTTOM = L"piston_bottom";
const std::wstring PistonBaseBlock::TEXTURE_INNER_TOP = L"piston_inner_top";
const float PistonBaseBlock::PLATFORM_THICKNESS = 0.25f;
static const AABB* PISTON_EXTENDED_DOWN = AABB::newPermanent(0.0, 0.25, 0.0, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_UP = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0);
static const AABB* PISTON_EXTENDED_NORTH = AABB::newPermanent(0.0, 0.0, 0.25, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_SOUTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.75);
static const AABB* PISTON_EXTENDED_WEST = AABB::newPermanent(0.25, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_EAST = AABB::newPermanent(0.0, 0.0, 0.0, 0.75, 1.0, 1.0);

PistonBaseBlock::PistonBaseBlock(bool sticky) : DirectionalBlock(Material::PISTON)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::UP)->setValue(EXTENDED, false));
    mSticky = sticky;
    setSoundType(SoundType::STONE);
    setDestroyTime(0.5f);
}

void PistonBaseBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

bool PistonBaseBlock::isViewBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED);
}

int PistonBaseBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return 5;
}

const AABB* PistonBaseBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getValue(EXTENDED))
    {
        switch (state->getValue(FACING)->get3DDataValue())
        {
        case Direction::eDirection_Down:
            return PISTON_EXTENDED_DOWN;
        case Direction::eDirection_North:
            return PISTON_EXTENDED_NORTH;
        case Direction::eDirection_South:
            return PISTON_EXTENDED_SOUTH;
        case Direction::eDirection_West:
            return PISTON_EXTENDED_WEST;
        case Direction::eDirection_East:
            return PISTON_EXTENDED_EAST;
        default:
            return PISTON_EXTENDED_UP;
        }
    }
    return FULL_BLOCK_SHAPE;
}

bool PistonBaseBlock::isTopSolidBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED) || state->getValue(FACING) == Direction::DOWN;
}

void PistonBaseBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                        const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                        boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, state->getShape(level, pos));
}

bool PistonBaseBlock::isSolidRender(const BlockState*) const
{
    return false;
}

int PistonBaseBlock::getNeighborSignal(Level* level, const BlockPos& pos, const Direction* direction)
{
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* side = *it;
        if (side != direction && level->hasSignal(pos.relative(side), side))
            return true;
    }

    if (level->hasSignal(pos, Direction::DOWN))
        return true;

    BlockPos above = pos.above();
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* side = *it;
        if (side != Direction::DOWN && level->hasSignal(above.relative(side), side))
        {
            const BlockState* neighbourState = level->getBlockState(above.relative(side));
            Block* neighbour = level->getBlock(above.relative(side));
            if (!neighbour->mBlockEntity)
                return true;
            if (neighbourState->getPistonPushReaction() == PISTON_PUSH_NORMAL)
                return false;
            return true;
        }
    }
    return false;
}

bool PistonBaseBlock::IsPushRestrictedByLevelHeight(Level* level, const BlockPos& pos, const Direction* direction)
{
    int y = pos.relative(direction).y;
    return y >= level->getMaxBuildHeight();
}

int PistonBaseBlock::isPushable(const BlockState* state, Level* level, const BlockPos& pos, const Direction* direction,
                                bool allowDestroy, const Direction* motionDirection)
{
    Block* block = state->getBlock();
    if (block == Blocks::obsidian)
        return false;
    if (!level->getWorldBorder()->isWithinBounds(pos) || pos.y < 0 || (direction == Direction::DOWN && pos.y == 0)
        || IsPushRestrictedByLevelHeight(level, pos, direction))
        return false;

    int minXZ = -(level->mDimension->getXZSize() * 16) / 2;
    int maxXZ = ((level->mDimension->getXZSize() << 4) / 2) - 1;
    if (!(pos.x > minXZ && pos.x < maxXZ && pos.z > minXZ && pos.z < maxXZ))
        return false;

    if (block == Blocks::piston || block == Blocks::sticky_piston)
    {
        if (level->getBlockState(pos)->getValue(EXTENDED))
            return false;
    }
    else
    {
        if (Block::getId(block) == BlockId::CHEST || block == Blocks::trapped_chest)
            return block->mayPlace(level, pos.relative(direction));
        if (state->getDestroySpeed(level, pos) == Block::INDESTRUCTIBLE)
            return false;
        switch (state->getPistonPushReaction())
        {
        case PISTON_PUSH_DESTROY:
            return allowDestroy;
        case PISTON_PUSH_BLOCK:
            return false;
        case PISTON_PUSH_PUSH_ONLY:
            return direction == motionDirection;
        }
    }
    return true;
}

void PistonBaseBlock::checkIfExtend(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (getNeighborSignal(level, pos, facing))
    {
        if (!state->getValue(EXTENDED))
        {
            PistonStructureResolver resolver(level, pos, facing, true);
            if (resolver.resolve())
                level->blockEvent(pos, this, 0, facing->get3DDataValue());
        }
    }
    else if (state->getValue(EXTENDED))
    {
        level->blockEvent(pos, this, 1, facing->get3DDataValue());
    }
}

void PistonBaseBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance>)
{
    level->setBlock(pos, state->setValue(FACING, Direction::getDirectionFromLivingEntity(pos, placer)),
                    Block::UPDATE_CLIENTS, false);
    if (!level->mIsClientSide)
        checkIfExtend(level, pos, state);
}

void PistonBaseBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                      const BlockPos&)
{
    if (!level->mIsClientSide)
        checkIfExtend(level, pos, state);
}

void PistonBaseBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide && level->getBlockEntity(pos) == NULL)
        checkIfExtend(level, pos, state);
}

const Direction* PistonBaseBlock::getFacingFromPlayer(Level*, const BlockPos& pos,
                                                      boost::shared_ptr<LivingEntity> placer)
{
    if (Mth::abs(static_cast<float>(placer->mX) - (static_cast<float>(pos.x) + 0.5f)) < 2.5f
        && Mth::abs(static_cast<float>(placer->mZ) - (static_cast<float>(pos.z) + 0.5f)) < 2.5f)
    {
        double eyeY = placer->mY + placer->getEyeHeight();
        if (eyeY - static_cast<double>(pos.y) > 2.0)
            return Direction::UP;
        if (static_cast<double>(pos.y) - eyeY > 0.0)
            return Direction::DOWN;
    }
    return placer->getDirection()->getOpposite();
}

const BlockState* PistonBaseBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction*, float,
                                                        float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState();
    return state->setValue(FACING, getFacingFromPlayer(level, pos, placer))->setValue(EXTENDED, false);
}

bool PistonBaseBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

int PistonBaseBlock::moveBlocks(Level* level, const BlockPos& pos, const Direction* direction, bool extending)
{
    if (!extending)
        level->removeBlock(pos.relative(direction));

    PistonStructureResolver resolver(level, pos, direction, extending);
    if (!resolver.resolve())
        return false;

    BlockPos headPos = pos.relative(direction);
    bool headInWater = level->getLiquidBlockState(headPos)->getMaterial() == Material::WATER
                       || level->getLiquidBlockState(headPos.above())->getMaterial() == Material::WATER;

    std::vector<BlockPos>& toPush = resolver.getToPush();
    std::vector<const BlockState*> pushStates;
    for (unsigned int i = 0; i < toPush.size(); ++i)
    {
        const BlockPos& pushPos = toPush.at(i);
        pushStates.push_back(level->getBlockState(pushPos)->fillVirtualBlockStateProperties(level, pushPos));
    }

    std::vector<BlockPos>& toDestroy = resolver.getToDestroy();
    int remaining = toPush.size() + toDestroy.size();
    const BlockState** movedStates = new const BlockState*[remaining];
    const Direction* moveDirection = extending ? direction : direction->getOpposite();

    for (int i = toDestroy.size() - 1; i >= 0; --i)
    {
        BlockPos destroyPos = toDestroy.at(i);
        const BlockState* state = level->getBlockState(destroyPos);
        state->getBlock()->spawnResources(level, destroyPos, level->getBlockState(destroyPos), 0);
        level->setBlock(destroyPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        level->setExtraBlock(destroyPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        movedStates[--remaining] = state;
    }

    for (int i = toPush.size() - 1; i >= 0; --i)
    {
        BlockPos pushPos = toPush.at(i);
        const BlockState* state = level->getBlockState(pushPos);
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pushPos);
        CompoundTag* blockEntityTag = blockEntity != NULL ? blockEntity->getUpdateTagForPistonMovement() : NULL;
        level->setBlock(pushPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->setExtraBlock(pushPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->removeBlock(pushPos);
        pushPos = pushPos.relative(moveDirection);
        level->setBlock(pushPos,
                        Blocks::piston_extension->defaultBlockState()->setValue(PistonMagicBlock::FACING, direction),
                        Block::UPDATE_INVISIBLE, false);
        level->setBlockEntity(pushPos, PistonMagicBlock::newMovingBlockEntity(pushStates.at(i), direction, extending,
                                                                              false, blockEntityTag));
        movedStates[--remaining] = state;
    }

    if (extending)
    {
        PistonHeadBlock::PistonType* type = mSticky ? pistonTypeSticky : pistonTypeNormal;
        const BlockState* headState = Blocks::piston_head->defaultBlockState()
                                          ->setValue(PistonHeadBlock::FACING, direction)
                                          ->setValue(PistonHeadBlock::TYPE, type);
        const BlockState* movingState = Blocks::piston_extension->defaultBlockState()
                                            ->setValue(PistonMagicBlock::FACING, direction)
                                            ->setValue(PistonMagicBlock::TYPE,
                                                       mSticky ? pistonTypeSticky : pistonTypeNormal);
        level->setBlock(headPos, movingState, Block::UPDATE_INVISIBLE, false);
        if (headInWater)
            level->setExtraBlock(headPos, Blocks::water->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        else
            level->setExtraBlock(headPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        level->setBlockEntity(headPos, PistonMagicBlock::newMovingBlockEntity(headState, direction, true, true, NULL));
    }

    for (int i = toDestroy.size() - 1; i >= 0; --i)
        level->updateNeighborsAt(toDestroy.at(i), movedStates[remaining++]->getBlock(), false);

    for (int i = toPush.size() - 1; i >= 0; --i)
        level->updateNeighborsAt(toPush.at(i), movedStates[remaining++]->getBlock(), false);

    if (extending)
    {
        level->updateNeighborsAt(headPos, Blocks::piston_head, false);
        level->neighborChanged(pos.below(), Blocks::piston_head, pos);
    }

    delete[] movedStates;
    return true;
}

bool PistonBaseBlock::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId,
                                   int eventParam)
{
    const Direction* facing = state->getValue(FACING);
    if (!level->mIsClientSide)
    {
        int extend = getNeighborSignal(level, pos, facing);
        if (extend && eventId == 1)
        {
            level->setBlock(pos, state->setValue(EXTENDED, true), Block::UPDATE_CLIENTS, false);
            return false;
        }
        if (!extend && eventId == 0)
            return false;
    }

    if (eventId == 0)
    {
        PIXBeginNamedEvent(0.0F, "Create push\n");
        if (!moveBlocks(level, pos, facing, true))
            return false;
        level->setBlock(pos, state->setValue(EXTENDED, true), Block::UPDATE_ALL, false);
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::PISTON_EXTEND, SoundSource::eSoundSource_Blocks,
                         0.5f, level->mRandom->nextFloat() * 0.25f + 0.6f, 16.0f);
        PIXEndNamedEvent();
    }
    else if (eventId == 1)
    {
        PIXBeginNamedEvent(0.0F, "Contract phase A\n");
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos.relative(facing));
        if (blockEntity != NULL && boost::dynamic_pointer_cast<PistonMovingBlockEntity>(blockEntity) != NULL)
            boost::dynamic_pointer_cast<PistonMovingBlockEntity>(blockEntity)->finalTick();

        level->setBlock(pos,
                        Blocks::piston_extension->defaultBlockState()
                            ->setValue(PistonMagicBlock::FACING, facing)
                            ->setValue(PistonMagicBlock::TYPE, mSticky ? pistonTypeSticky : pistonTypeNormal),
                        Block::UPDATE_ALL, false);
        level->setBlockEntity(
            pos, PistonMagicBlock::newMovingBlockEntity(getBlockState(eventParam), facing, false, true, NULL));
        PIXEndNamedEvent();

        if (mSticky)
        {
            PIXBeginNamedEvent(0.0F, "Contract sticky phase A\n");
            BlockPos pullPos = pos.offset(facing->getStepX() * 2, facing->getStepY() * 2, facing->getStepZ() * 2);
            const BlockState* pullState = level->getBlockState(pullPos);
            Block* pullBlock = pullState->getBlock();
            bool pulledMovingBlock = false;
            PIXEndNamedEvent();
            if (pullBlock == Blocks::piston_extension)
            {
                PIXBeginNamedEvent(0.0F, "Contract sticky phase B\n");
                boost::shared_ptr<BlockEntity> pullEntity = level->getBlockEntity(pullPos);
                if (pullEntity != NULL && boost::dynamic_pointer_cast<PistonMovingBlockEntity>(pullEntity) != NULL)
                {
                    boost::shared_ptr<PistonMovingBlockEntity> movingEntity =
                        boost::dynamic_pointer_cast<PistonMovingBlockEntity>(pullEntity);
                    if (movingEntity->getDirection() == facing && movingEntity->isExtending())
                    {
                        movingEntity->finalTick();
                        pulledMovingBlock = true;
                    }
                }
                PIXEndNamedEvent();
            }

            PIXBeginNamedEvent(0.0F, "Contract sticky phase C\n");
            if (!pulledMovingBlock && pullState->getMaterial() != Material::AIR
                && isPushable(pullState, level, pullPos, facing->getOpposite(), false, facing)
                && (pullState->getPistonPushReaction() == PISTON_PUSH_NORMAL || pullBlock == Blocks::piston
                    || pullBlock == Blocks::sticky_piston))
            {
                moveBlocks(level, pos, facing, false);
            }
            PIXEndNamedEvent();
        }
        else
        {
            level->removeBlock(pos.relative(facing));
        }

        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::PISTON_CONTRACT,
                         SoundSource::eSoundSource_Blocks, 0.5f, level->mRandom->nextFloat() * 0.15f + 0.6f, 16.0f);
    }
    return true;
}

bool PistonBaseBlock::isCubeShaped(const BlockState*)
{
    return false;
}

const Direction* PistonBaseBlock::getFacing(int data)
{
    int facing = data & 7;
    if (facing > 5)
        return NULL;
    return Direction::from3DDataValue(facing);
}

const BlockState* PistonBaseBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(FACING, getFacing(data))->setValue(EXTENDED, (data & 8) > 0);
}

const BlockState* PistonBaseBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* PistonBaseBlock::mirror(const BlockState* state, Mirror* mirror)
{
    AbstractProperty<const Direction*>* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

int PistonBaseBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(EXTENDED))
        data |= 8;
    return data;
}

BlockStateDefinition* PistonBaseBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, EXTENDED};
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape PistonBaseBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                   const BlockPos& pos, const Direction* direction)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    if (state->getValue(FACING) == direction->getOpposite() || !state->getValue(EXTENDED))
    {
        return BLOCK_FACE_SOLID;
    }
    return BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction == facing)
    {
        const AABB* shape = getShape(state, NULL, BlockPos());
        if (state->getValue(EXTENDED) || shape->mMinX > 0.0 || shape->mMinY > 0.0 || shape->mMinZ > 0.0
            || shape->mMaxX < 1.0 || shape->mMaxY < 1.0 || shape->mMaxZ < 1.0)
            return mInnerTopIcon;
        return mTopIcon;
    }

    if (direction == facing->getOpposite())
        return mBottomIcon;
    return mIcon;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const std::wstring& name)
{
    if (name == TEXTURE_SIDE)
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mIcon;
    if (name == TEXTURE_TOP)
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mTopIcon;
    if (name == TEXTURE_TOP_STICKY)
        return static_cast<PistonBaseBlock*>(Blocks::sticky_piston)->mTopIcon;
    if (name == TEXTURE_INNER_TOP)
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mInnerTopIcon;
    return NULL;
}

void PistonBaseBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(TEXTURE_SIDE);
    mTopIcon = iconRegister->registerIcon(mSticky ? TEXTURE_TOP_STICKY : TEXTURE_TOP);
    mInnerTopIcon = iconRegister->registerIcon(TEXTURE_INNER_TOP);
    mBottomIcon = iconRegister->registerIcon(TEXTURE_BOTTOM);
}

EBlockRenderShape PistonBaseBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_PISTON_BASE;
}

bool PistonBaseBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return true;
}
