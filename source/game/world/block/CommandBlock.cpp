#include "game/world/block/CommandBlock.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/CommandBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<const Direction*>* CommandBlock::FACING = NULL;
AbstractProperty<bool>* CommandBlock::CONDITIONAL = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"conditional"));

void CommandBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

CommandBlock::CommandBlock(const MaterialColor* materialColor, Mode mode)
    : BaseEntityBlock(Material::METAL, materialColor)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(CONDITIONAL, false));
    mMode = mode;
}

unsigned int CommandBlock::getMode()
{
    return static_cast<unsigned int>(mMode);
}

boost::shared_ptr<BlockEntity> CommandBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<CommandBlockEntity>(
        new CommandBlockEntity(static_cast<CommandBlock::Mode>(getMode())));
    entity->setLevel(level);
    return entity;
}

void CommandBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                   const BlockPos& fromPos)
{
    (void)state;
    (void)level;
    (void)pos;
    (void)block;
    (void)fromPos;
}

void CommandBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    (void)level;
    (void)pos;
    (void)state;
    (void)random;
}

int CommandBlock::getTickDelay(Level*)
{
    return 1;
}

bool CommandBlock::use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>,
                       InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    return false;
}

bool CommandBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int CommandBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::COMMAND, blockEntity->GetType()))
    {
        return static_cast<int>(
            boost::dynamic_pointer_cast<CommandBlockEntity>(blockEntity)->getCommandBlock()->getSuccessCount());
    }
    return Block::SIGNAL_NONE;
}

void CommandBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<LivingEntity>,
                               not_null_ptr<ItemInstance> itemInstance)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isCommandBlock = eTYPE_DERIVED_FROM(BlockEntityType::COMMAND, blockEntity->GetType());
    if (!isCommandBlock)
    {
        return;
    }
    BaseCommandBlock* commandBlock = boost::dynamic_pointer_cast<CommandBlockEntity>(blockEntity)->getCommandBlock();
    if (itemInstance->hasCustomHoverName())
    {
        commandBlock->setName(itemInstance->getHoverName(), true);
    }
}

int CommandBlock::getResourceCount(Random*)
{
    return 0;
}

EBlockRenderShape CommandBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_COMMAND_BLOCK;
}

const BlockState* CommandBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::from3DDataValue(data & 7))
        ->setValue(CONDITIONAL, (data & 8) > 0);
}

int CommandBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(CONDITIONAL))
    {
        data |= 8;
    }
    return data;
}

const BlockState* CommandBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* CommandBlock::mirror(const BlockState* state, Mirror* mirror)
{
    AbstractProperty<const Direction*>* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* CommandBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, CONDITIONAL};
    return new BlockStateDefinition(this, properties, 2);
}

const BlockState* CommandBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float, float,
                                                     int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::getDirectionFromLivingEntity(pos, placer)->getOpposite())
        ->setValue(CONDITIONAL, false);
}

bool CommandBlock::isCommandBlock(Block* block)
{
    return block == Blocks::command_block || block == Blocks::chain_command_block
           || block == Blocks::repeating_command_block;
}

TextureAtlasSprite* CommandBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction == facing)
    {
        return mBackIcon;
    }
    if (direction != facing->getOpposite())
    {
        if (state->getValue(CONDITIONAL))
        {
            return mConditionalIcon;
        }
        return mIcon;
    }
    return mFrontIcon;
}

void CommandBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
    mFrontIcon = iconRegister->registerIcon(getIconName() + L"_front");
    mBackIcon = iconRegister->registerIcon(getIconName() + L"_back");
    mConditionalIcon = iconRegister->registerIcon(getIconName() + L"_conditional");
}
