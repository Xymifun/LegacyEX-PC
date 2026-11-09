#include "game/world/block/LogBlock.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"
#include "game/core/StringIds.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

std::wstring LogBlock::TEXTURE_NAMES[6] = {std::wstring(L"oak"),    std::wstring(L"spruce"), std::wstring(L"birch"),
                                           std::wstring(L"jungle"), std::wstring(L"acacia"), std::wstring(L"big_oak")};
LogBlock::LogAxis* LogBlock::LogAxis::values[4] = {new LogBlock::LogAxis(L"x"), new LogBlock::LogAxis(L"y"),
                                                   new LogBlock::LogAxis(L"z"), new LogBlock::LogAxis(L"none")};
LogBlock::LogAxis* LogBlock::X = LogBlock::LogAxis::values[0];
LogBlock::LogAxis* LogBlock::Y = LogBlock::LogAxis::values[1];
LogBlock::LogAxis* LogBlock::Z = LogBlock::LogAxis::values[2];
LogBlock::LogAxis* LogBlock::NONE = LogBlock::LogAxis::values[3];
EnumProperty<LogBlock::LogAxis*>* LogBlock::AXIS = NULL;
static const int gLogDescriptionIds[6] = {IDS_TILE_LOG_OAK,    IDS_TILE_LOG_SPRUCE, IDS_TILE_LOG_BIRCH,
                                          IDS_TILE_LOG_JUNGLE, IDS_TILE_LOG_ACACIA, IDS_TILE_LOG_DARK_OAK};

LogBlock::LogAxis::LogAxis(const std::wstring& name) : mName(name)
{
}

unsigned int LogBlock::LogAxis::getId() const
{
    for (unsigned int i = 0; i < 3; ++i)
    {
        if (values[i] == this)
            return i;
    }
    return 3;
}

Direction::Axis* LogBlock::LogAxis::getAxis()
{
    switch (getId())
    {
    case 0:
        return Direction::Axis::X;
    case 1:
        return Direction::Axis::Y;
    case 2:
        return Direction::Axis::Z;
    default:
        return NULL;
    }
}

std::wstring LogBlock::LogAxis::toString() const
{
    return mName;
}

std::wstring LogBlock::LogAxis::getSerializedName() const
{
    return mName;
}

LogBlock::LogAxis* LogBlock::LogAxis::fromAxis(Direction::Axis* axis)
{
    switch (axis->getId())
    {
    case Direction::Axis::eAxis_X:
        return LogBlock::X;
    case Direction::Axis::eAxis_Y:
        return LogBlock::Y;
    case Direction::Axis::eAxis_Z:
        return LogBlock::Z;
    default:
        return LogBlock::NONE;
    }
}

void LogBlock::blockStaticCtor()
{
    std::wstring name(L"axis");
    const Predicate<LogAxis*>* filter = Predicates<LogAxis*>::alwaysTrue;
    EnumProperty<LogAxis*>* axis;
    {
        boost::unordered_set<LogAxis*> values;
        for (unsigned int i = 0; i < 4; ++i)
            values.insert(LogAxis::values[i]);
        axis = EnumProperty<LogAxis*>::create(name, typeid(LogAxis*), values, filter);
    }
    AXIS = axis;
}

LogBlock::LogBlock() : RotatedPillarBlock(Material::WOOD)
{
    setDestroyTime(2.0f);
    setSoundType(SoundType::WOOD);
}

void LogBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->hasChunksAt(pos.offset(-5, -5, -5), pos.offset(5, 5, 5)))
        return;

    Iterator<BlockPos*>* positions = BlockPos::between(pos.offset(-4, -4, -4), pos.offset(4, 4, 4));
    while (positions->hasNext())
    {
        const BlockPos neighbour = *positions->next();
        const BlockState* neighbourState = level->getBlockState(neighbour);
        if (neighbourState->getBlock()->mMaterial == Material::LEAVES
            && !neighbourState->getValue(LeafBlock::CHECK_DECAY))
        {
            level->setBlock(neighbour, neighbourState->setValue(LeafBlock::CHECK_DECAY, true), Block::UPDATE_INVISIBLE,
                            false);
        }
    }
    delete positions;
}

const BlockState* LogBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                 float hitX, float hitY, float hitZ, int auxValue,
                                                 boost::shared_ptr<LivingEntity> placer)
{
    return getBlockState(auxValue)->setValue(AXIS, LogAxis::fromAxis(direction->getAxis()));
}

const BlockState* LogBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
    case Rotation::eTYPE_CounterClockwise90:
    {
        LogAxis* axis = state->getValue(AXIS);
        switch (axis->getId())
        {
        case 0:
            return state->setValue(AXIS, Z);
        case 2:
            return state->setValue(AXIS, X);
        }
    }
    }
    return state;
}

int LogBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = auxValue == -1 ? defaultBlockState() : getBlockState(auxValue);
    int woodType = getWoodType(state);
    return gLogDescriptionIds[woodType];
}

bool LogBlock::isStrippable()
{
    return true;
}

const BlockState* LogBlock::getStrippedBlock(const BlockState* state)
{
    const BlockState* stripped = NULL;
    switch (getWoodType(state))
    {
    case 0:
        stripped = Blocks::stripped_log_oak->defaultBlockState();
        break;
    case 1:
        stripped = Blocks::stripped_log_spruce->defaultBlockState();
        break;
    case 2:
        stripped = Blocks::stripped_log_birch->defaultBlockState();
        break;
    case 3:
        stripped = Blocks::stripped_log_jungle->defaultBlockState();
        break;
    }
    if (stripped != NULL)
    {
        switch (state->getValue(AXIS)->getId())
        {
        case 0:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::X);
            break;
        case 1:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y);
            break;
        case 2:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Z);
            break;
        default:
            return stripped;
        }
        return stripped;
    }
    return state;
}

unsigned int LogBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int id = Item::getId(tooltipData.m_pItemInstance->getItem());
    if ((id == 0x102 || id == 0x10f || id == 0x113 || id == 0x117 || id == 0x11e)
        && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
        && !Minecraft::InMiniGame(eMiniGameId_Any, true))
        return IDS_TOOLTIPS_STRIP;
    return Block::GetInteractTooltip(tooltipData);
}

TextureAtlasSprite* LogBlock::getTypeTexture(const BlockState* state)
{
    return mTypeTextures[getWoodType(state)];
}

TextureAtlasSprite* LogBlock::getTopTexture(const BlockState* state)
{
    return mTopTextures[getWoodType(state)];
}

void LogBlock::registerIcons(IconRegister* iconRegister)
{
    int i;
    for (i = 0; i < 6; ++i)
    {
        mTypeTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i]);
        mTopTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i] + L"_top");
    }
}

Direction::Axis* LogBlock::getAxis(const BlockState* state)
{
    return state->getValue(AXIS)->getAxis();
}
