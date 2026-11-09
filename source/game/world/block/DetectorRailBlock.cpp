#include "game/world/block/DetectorRailBlock.hpp"

#include <cstring>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BaseCommandBlock.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<BaseRailBlock::RailShape*>* DetectorRailBlock::SHAPE = NULL;
BooleanProperty* DetectorRailBlock::POWERED = static_cast<BooleanProperty*>(BooleanProperty::create(L"powered"));

void DetectorRailBlock::blockStaticCtor()
{
    std::wstring name;
    name = L"shape";
    Predicates<BaseRailBlock::RailShape*>::FunctionPredicate* filter =
        new Predicates<BaseRailBlock::RailShape*>::FunctionPredicate(railpredicate);
    EnumProperty<BaseRailBlock::RailShape*>* shape;
    {
        boost::unordered_set<BaseRailBlock::RailShape*> values;
        for (int i = 0; i < 10; ++i)
            values.insert(RailShape::VALUES[i]);
        shape = EnumProperty<BaseRailBlock::RailShape*>::create(name, typeid(BaseRailBlock::RailShape), values, filter);
    }
    DetectorRailBlock::SHAPE = shape;
}

DetectorRailBlock::DetectorRailBlock() : BaseRailBlock(true)
{
    DerivedInit();
    setTicking(true);
    registerDefaultState(mStateDefinition->any()
                             ->setValue(DetectorRailBlock::POWERED, false)
                             ->setValue(DetectorRailBlock::SHAPE, BaseRailBlock::RailShape::NORTH_SOUTH));
    memset(mIcons, 0, sizeof(mIcons));
}

int DetectorRailBlock::getTickDelay(Level*)
{
    return 20;
}

bool DetectorRailBlock::isSignalSource(const BlockState*)
{
    return true;
}

void DetectorRailBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                     const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide)
    {
        if (!state->getValue(DetectorRailBlock::POWERED))
            checkPressed(level, pos, state);
    }
}

void DetectorRailBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void DetectorRailBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide)
    {
        if (state->getValue(DetectorRailBlock::POWERED) == true)
            checkPressed(level, pos, state);
    }
}

int DetectorRailBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(DetectorRailBlock::POWERED) ? Block::SIGNAL_MAX : Block::SIGNAL_NONE;
}

AABB* DetectorRailBlock::getSearchBB(const BlockPos& pos)
{
    float inset = 0.2f;
    return AABB::newTemp(pos.x + inset, pos.y, pos.z + inset, pos.x + 1 - inset, pos.y + 1 - inset, pos.z + 1 - inset);
}

std::vector<boost::shared_ptr<Entity> >* DetectorRailBlock::getInteractingMinecartOfType(
    Level* level, const BlockPos& pos, const std::type_info& type,
    const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    const AABB* box = getSearchBB(pos);
    if (predicate == NULL)
        return level->getEntitiesOfClass(type, box);
    return level->getEntitiesOfClass(type, box, predicate, false);
}

void DetectorRailBlock::checkPressed(Level* level, const BlockPos& pos, const BlockState* state)
{
    bool powered = state->getValue(DetectorRailBlock::POWERED);
    std::vector<boost::shared_ptr<Entity> >* minecarts = getInteractingMinecartOfType(level, pos, typeid(Minecart),
                                                                                      NULL);
    bool occupied = !minecarts->empty();
    if (occupied && !powered)
    {
        level->setBlock(pos, state->setValue(DetectorRailBlock::POWERED, true), Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    if (!occupied && powered)
    {
        level->setBlock(pos, state->setValue(DetectorRailBlock::POWERED, false), Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    if (occupied)
        level->addToTickNextTick(pos, this, getTickDelay(level));
    level->updateNeighbourForOutputSignal(pos, this);
    delete minecarts;
}

void DetectorRailBlock::updatePowerToConnected(Level* level, const BlockPos& pos, const BlockState* state, bool)
{
    BaseRailBlock::Rail rail(level, pos, state);
    std::vector<BlockPos>* connections = rail.getConnections();
    for (std::vector<BlockPos>::const_iterator it = connections->begin(); it != connections->end(); ++it)
    {
        const BlockState* neighbor = level->getBlockState(*it);
        if (neighbor != NULL)
            neighbor->neighborChanged(level, *it, neighbor->getBlock(), pos);
    }
}

int DetectorRailBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&,
                                       const Direction* direction)
{
    return state->getValue(DetectorRailBlock::POWERED) && direction == Direction::UP ? Block::SIGNAL_MAX
                                                                                     : Block::SIGNAL_NONE;
}

void DetectorRailBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    BaseRailBlock::onPlace(level, pos, state);
    checkPressed(level, pos, state);
}

const AbstractProperty<BaseRailBlock::RailShape*>* DetectorRailBlock::getShapeProperty()
{
    return DetectorRailBlock::SHAPE;
}

bool DetectorRailBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int DetectorRailBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    const BlockState* railState = level->getBlockState(pos);
    if (!railState->getValue(DetectorRailBlock::POWERED))
        return Block::SIGNAL_NONE;

    std::vector<boost::shared_ptr<Entity> >* commandCarts = getInteractingMinecartOfType(
        level, pos, typeid(MinecartCommandBlock), NULL);
    if (!commandCarts->empty())
    {
        int successCount;
        {
            boost::shared_ptr<MinecartCommandBlock> commandCart = boost::static_pointer_cast<MinecartCommandBlock>(
                commandCarts->at(0));
            successCount = commandCart->getCommandBlock()->getSuccessCount();
        }
        delete commandCarts;
        return successCount;
    }
    delete commandCarts;

    std::vector<boost::shared_ptr<Entity> >* containerCarts = getInteractingMinecartOfType(
        level, pos, typeid(Minecart), EntitySelector::CONTAINER_ENTITY_SELECTOR);
    if (!containerCarts->empty())
    {
        boost::shared_ptr<Entity> containerCart = containerCarts->at(0);
        delete containerCarts;
        return AbstractContainerMenu::getRedstoneSignalFromContainer(
            boost::dynamic_pointer_cast<Container>(containerCart));
    }
    delete containerCarts;
    return Block::SIGNAL_NONE;
}

const BlockState* DetectorRailBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(DetectorRailBlock::SHAPE, BaseRailBlock::RailShape::byData(data & 7))
        ->setValue(DetectorRailBlock::POWERED, (data & 8) > 0);
}

int DetectorRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(DetectorRailBlock::SHAPE)->getData();
    if (state->getValue(DetectorRailBlock::POWERED))
        data |= 8;
    return data;
}

const BlockState* DetectorRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 2:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 3:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 4:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 5:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 6:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 7:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 8:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 9:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_WEST);
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 0:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::EAST_WEST);
        case 1:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_SOUTH);
        case 2:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 3:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 4:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 5:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 6:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 7:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 8:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 9:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_WEST);
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 0:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::EAST_WEST);
        case 1:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_SOUTH);
        case 2:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 3:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 4:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 5:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 6:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 7:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 8:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 9:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_EAST);
        }
    default:
        break;
    }

    return state;
}

const BlockState* DetectorRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    BaseRailBlock::RailShape* currentShape = state->getValue(DetectorRailBlock::SHAPE);
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        switch (currentShape->getData())
        {
        case 4:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 5:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 6:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 7:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 8:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 9:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_EAST);
        default:
            return Block::mirror(state, mirror);
        }
    case Mirror::eTYPE_FrontBack:
        switch (currentShape->getData())
        {
        case 2:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 3:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 6:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 7:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 8:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 9:
            return state->setValue(DetectorRailBlock::SHAPE, RailShape::NORTH_WEST);
        default:
            return Block::mirror(state, mirror);
        }
    default:
        return Block::mirror(state, mirror);
    }
}

BlockStateDefinition* DetectorRailBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {DetectorRailBlock::SHAPE, DetectorRailBlock::POWERED};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DetectorRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(DetectorRailBlock::POWERED) ? mIcons[1] : mIcons[0];
}

void DetectorRailBlock::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(L"detectorRail");
    mIcons[1] = iconRegister->registerIcon(L"detectorRail_on");
}
