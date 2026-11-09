#include "game/world/block/ComparatorBlock.hpp"
#include "game/core/Abilities.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/ComparatorBlockEntity.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<ComparatorBlock::ComparatorMode*>* ComparatorBlock::MODE = NULL;
static AbstractProperty<bool>* sPoweredProperty = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));
ComparatorBlock::ComparatorMode* ComparatorBlock::ComparatorMode::COMPARE = new ComparatorBlock::ComparatorMode(
    L"compare");
ComparatorBlock::ComparatorMode* ComparatorBlock::ComparatorMode::SUBTRACT = new ComparatorBlock::ComparatorMode(
    L"subtract");
static ComparatorBlock::ComparatorMode* sModes[2] = {ComparatorBlock::ComparatorMode::COMPARE,
                                                     ComparatorBlock::ComparatorMode::SUBTRACT};

ComparatorBlock::ComparatorMode::ComparatorMode(const std::wstring& name) : mName(name)
{
}
std::wstring ComparatorBlock::ComparatorMode::toString() const
{
    return mName;
}
std::wstring ComparatorBlock::ComparatorMode::getSerializedName() const
{
    return mName;
}

ComparatorBlock::EntitySelector::EntitySelector(const Direction* direction) : mDirection(direction)
{
}
bool ComparatorBlock::EntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && entity->getDirection() == mDirection;
}

ComparatorBlock::ComparatorBlock(bool on) : DiodeBlock(on)
{
    DerivedInit();

    const BlockState* state = mStateDefinition->any()
                                  ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::NORTH)
                                  ->setValue(sPoweredProperty, false)
                                  ->setValue(MODE, ComparatorMode::COMPARE);
    registerDefaultState(state);

    mBlockEntity = true;
}
boost::shared_ptr<BlockEntity> ComparatorBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<ComparatorBlockEntity>(new ComparatorBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

void ComparatorBlock::blockStaticCtor()
{
    std::wstring name(L"mode");
    EnumProperty<ComparatorMode*>* mode;
    const Predicate<ComparatorMode*>* filter = Predicates<ComparatorMode*>::alwaysTrue;
    {
        boost::unordered_set<ComparatorMode*> values;
        for (int i = 0; i < 2; ++i)
            values.insert(sModes[i]);
        mode = EnumProperty<ComparatorMode*>::create(name, typeid(ComparatorMode*), values, filter);
    }
    MODE = mode;
}

std::wstring ComparatorBlock::getName()
{
    return L10N::GetString(IDS_ITEM_COMPARATOR);
}
Item* ComparatorBlock::getResource(const BlockState*, Random*, int)
{
    return Items::comparator;
}
not_null_ptr<ItemInstance> ComparatorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::comparator));
}
int ComparatorBlock::getTurnOnDelay(const BlockState*)
{
    return 2;
}

const BlockState* ComparatorBlock::getOnBlock(const BlockState* state)
{
    bool powered = state->getValue(sPoweredProperty);
    ComparatorMode* mode = state->getValue(MODE);
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    return Blocks::powered_comparator->defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facing)
        ->setValue(sPoweredProperty, powered)
        ->setValue(MODE, mode);
}

const BlockState* ComparatorBlock::getOffBlock(const BlockState* state)
{
    bool powered = state->getValue(sPoweredProperty);
    ComparatorMode* mode = state->getValue(MODE);
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    return Blocks::unpowered_comparator->defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facing)
        ->setValue(sPoweredProperty, powered)
        ->setValue(MODE, mode);
}

bool ComparatorBlock::isOn(const BlockState* state)
{
    return mOn || state->getValue(sPoweredProperty);
}

int ComparatorBlock::getOutputSignal(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    bool isComparator = eTYPE_DERIVED_FROM(BlockEntityType::COMPARATOR, blockEntity->GetType());
    if (isComparator)
    {
        boost::shared_ptr<ComparatorBlockEntity> comparator = boost::dynamic_pointer_cast<ComparatorBlockEntity>(
            blockEntity);
        return comparator->getOutputSignal();
    }
    return SIGNAL_NONE;
}

int ComparatorBlock::calculateOutputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(MODE) == ComparatorMode::SUBTRACT)
    {
        int output = getInputSignal(level, pos, state) - getAlternateSignal(level, pos, state);
        if (SIGNAL_NONE > output)
            output = SIGNAL_NONE;
        return output;
    }
    return getInputSignal(level, pos, state);
}

bool ComparatorBlock::shouldTurnOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    int input = getInputSignal(level, pos, state);
    if (input >= SIGNAL_MAX)
        return true;
    if (input == SIGNAL_NONE)
        return false;
    int alternate = getAlternateSignal(level, pos, state);
    if (alternate == SIGNAL_NONE)
        return true;
    return input >= alternate;
}

boost::shared_ptr<ItemFrame> ComparatorBlock::getItemFrame(Level* level, const Direction* direction,
                                                           const BlockPos& pos)
{
    EntitySelector selector(direction);
    AABB* box = AABB::newTemp(pos.x, pos.y, pos.z, pos.x + 1, pos.y + 1, pos.z + 1);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(ItemFrame), box, &selector,
                                                                                  false);
    if (entities->size() == 1)
    {
        boost::shared_ptr<ItemFrame> frame = boost::dynamic_pointer_cast<ItemFrame>(entities->at(0));
        delete entities;
        return frame;
    }
    delete entities;
    return boost::shared_ptr<ItemFrame>();
}

int ComparatorBlock::getInputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    int signal = DiodeBlock::getInputSignal(level, pos, state);
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos inputPos = pos.relative(direction);
    const BlockState* inputState = level->getBlockState(inputPos);
    if (inputState->hasAnalogOutputSignal())
    {
        signal = inputState->getAnalogOutputSignal(level, inputPos);
    }
    else if (signal < SIGNAL_MAX && inputState->isSolidBlockingCubeAndNotSignalSource())
    {
        inputPos = inputPos.relative(direction);
        inputState = level->getBlockState(inputPos);
        if (inputState->hasAnalogOutputSignal())
        {
            signal = inputState->getAnalogOutputSignal(level, inputPos);
        }
        else if (inputState->getMaterial() == Material::AIR)
        {
            boost::shared_ptr<ItemFrame> frame = getItemFrame(level, direction, inputPos);
            if (frame)
            {
                signal = frame->getAnalogOutput();
            }
        }
    }
    return signal;
}

bool ComparatorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                          InteractionHand::EInteractionHand, const Direction*, float, float, float, bool simulate)
{
    if (!player->mAbilities.mMayBuild)
    {
        return false;
    }

    const BlockState* newState = state->cycle(MODE);
    float pitch = newState->getValue(MODE) == ComparatorMode::SUBTRACT ? 0.55f : 0.5f;
    level->playSound(player, pos, SoundEvents::COMPARATOR_CLICK, SoundSource::eSoundSource_Blocks, 0.3f, pitch, 16.0f);

    if (!simulate)
    {
        level->setBlock(pos, newState, Block::UPDATE_CLIENTS, false);
        refreshOutputState(level, pos, newState);
    }

    return true;
}

void ComparatorBlock::checkTickOnNeighbor(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->isBlockToBeTickedAt(pos, this))
        return;
    int newOutput = calculateOutputSignal(level, pos, state);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isComparator = eTYPE_DERIVED_FROM(BlockEntityType::COMPARATOR, blockEntity->GetType());
    int oldOutput = isComparator ? boost::dynamic_pointer_cast<ComparatorBlockEntity>(blockEntity)->getOutputSignal()
                                 : SIGNAL_NONE;
    if (!(newOutput == oldOutput && isOn(state) == shouldTurnOn(level, pos, state)))
    {
        if (shouldPrioritize(level, pos, state))
        {
            level->addToTickNextTick(pos, this, 2, -1);
        }
        else
        {
            level->addToTickNextTick(pos, this, 2, 0);
        }
    }
}

void ComparatorBlock::refreshOutputState(Level* level, const BlockPos& pos, const BlockState* state)
{
    int output = calculateOutputSignal(level, pos, state);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isComparator = eTYPE_DERIVED_FROM(BlockEntityType::COMPARATOR, blockEntity->GetType());
    int oldOutput = SIGNAL_NONE;
    if (isComparator)
    {
        boost::shared_ptr<ComparatorBlockEntity> comparator = boost::dynamic_pointer_cast<ComparatorBlockEntity>(
            blockEntity);
        oldOutput = comparator->getOutputSignal();
        comparator->setOutputSignal(output);
    }
    if (!(oldOutput == output && state->getValue(MODE) != ComparatorMode::COMPARE))
    {
        bool shouldOn = shouldTurnOn(level, pos, state);
        bool currentOn = isOn(state);
        int flags = gApp.GetGameHostOption(eGameHostOption_MiniGame) ? 0x24 : 2;
        if (currentOn && !shouldOn)
        {
            level->setBlock(pos, state->setValue(sPoweredProperty, false), flags, false);
        }
        else if (!currentOn && shouldOn)
        {
            level->setBlock(pos, state->setValue(sPoweredProperty, true), flags, false);
        }
        updateNeighborsInFront(level, pos, state);
    }
}

void ComparatorBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (mOn)
    {
        level->setBlock(pos, getOffBlock(state)->setValue(sPoweredProperty, true), Block::UPDATE_INVISIBLE, false);
    }
    refreshOutputState(level, pos, state);
}

void ComparatorBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    DiodeBlock::onPlace(level, pos, state);
    level->setBlockEntity(pos, newBlockEntity(level, 0));
}

void ComparatorBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    level->removeBlockEntity(pos);
    updateNeighborsInFront(level, pos, state);
}

bool ComparatorBlock::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId,
                                   int eventParam)
{
    Block::triggerEvent(state, level, pos, eventId, eventParam);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (!blockEntity)
    {
        return false;
    }
    return blockEntity->triggerEvent(eventId, eventParam);
}

const BlockState* ComparatorBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::from2DDataValue(data))
        ->setValue(sPoweredProperty, (data & 8) > 0)
        ->setValue(MODE, (data & 4) > 0 ? ComparatorMode::SUBTRACT : ComparatorMode::COMPARE);
}

int ComparatorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
    if (state->getValue(sPoweredProperty))
        data |= 8;
    if (state->getValue(MODE) == ComparatorMode::SUBTRACT)
        data |= 4;
    return data;
}

unsigned int ComparatorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ComparatorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(
        HorizontalDirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(HorizontalDirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* ComparatorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = HorizontalDirectionalBlock::FACING_PROPERTY;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* ComparatorBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HorizontalDirectionalBlock::FACING_PROPERTY, MODE, sPoweredProperty};
    return new BlockStateDefinition(this, properties, 3);
}

const BlockState* ComparatorBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                        float hitX, float hitY, float hitZ, int auxValue,
                                                        boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, placer->getDirection()->getOpposite())
        ->setValue(sPoweredProperty, false)
        ->setValue(MODE, ComparatorMode::COMPARE);
}

EBlockRenderShape ComparatorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_COMPARATOR;
}

bool ComparatorBlock::TestUse()
{
    return true;
}
TextureAtlasSprite* ComparatorBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::DOWN)
    {
        return state->getValue(sPoweredProperty) ? Blocks::redstone_torch->getTexture(direction)
                                                 : Blocks::unlit_redstone_torch->getTexture(direction);
    }
    if (direction == Direction::UP)
    {
        return mIcon;
    }
    return Blocks::stone_slab->getTexture(Direction::UP);
}
