#include "game/item/BedItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static const unsigned int BED_DESCRIPTION_IDS[16] = {
    IDS_TILE_BED_WHITE,  IDS_TILE_BED_ORANGE, IDS_TILE_BED_MAGENTA, IDS_TILE_BED_LIGHT_BLUE,
    IDS_TILE_BED_YELLOW, IDS_TILE_BED_LIME,   IDS_TILE_BED_PINK,    IDS_TILE_BED_GRAY,
    IDS_TILE_BED_SILVER, IDS_TILE_BED_CYAN,   IDS_TILE_BED_PURPLE,  IDS_TILE_BED_BLUE,
    IDS_TILE_BED_BROWN,  IDS_TILE_BED_GREEN,  IDS_TILE_BED_RED,     IDS_TILE_BED_BLACK};

static const std::wstring BED_OVERLAY_ICON_NAME = L"bed_overlay";

BedItem::BedItem() : Item(), mBedIcon(NULL)
{
    setMaxDamage(0);
    setStackedByData(true);
}

InteractionResult::EInteractionResult BedItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                     const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                                     bool simulate)
{
    if (direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos footPos = pos;
    Block* block = level->getBlockState(footPos)->getBlock();
    bool footReplaceable = block->mayReplaceWithPlace(level, footPos);

    if (!footReplaceable)
    {
        footPos = footPos.above();
    }

    const Direction* facing = Direction::from2DDataValue(Mth::floor((double)(player->mYRot * 4.0f / 360.0f) + 0.5) & 3);
    BlockPos headPos = footPos.relative(facing);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(footPos, direction, itemInstance)
        || !player->mayUseItemAt(headPos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const BlockState* headState = level->getBlockState(headPos);
    bool headReplaceable = headState->getBlock()->mayReplaceWithPlace(level, headPos);
    bool footFree = level->isEmptyBlock(footPos) || level->isEmptyWaterBlock(footPos) || footReplaceable;
    bool headFree = level->isEmptyBlock(headPos) || level->isEmptyWaterBlock(headPos) || headReplaceable;

    if (!footFree || !headFree || !level->getBlockState(footPos.below())->isTopSolidBlocking()
        || !level->getBlockState(headPos.below())->isTopSolidBlocking())
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate && !level->mIsClientSide)
    {
        const BlockState* footState = Blocks::bed->defaultBlockState()
                                          ->setValue(BedBlock::OCCUPIED, false)
                                          ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facing)
                                          ->setValue<const BedBlock::Part*>(BedBlock::PART, &gBedFoot);
        level->setBlock(footPos, footState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        level->setBlock(headPos, footState->setValue<const BedBlock::Part*>(BedBlock::PART, &gBedHead),
                        Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);

        player->awardStat(GenericStats::blocksPlaced(Blocks::bed), GenericStats::param_blocksPlaced(headState, 1));

        const SoundType* soundType = footState->getBlock()->getSoundType();
        level->playSound(boost::shared_ptr<Player>(), footPos, soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks, (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f, 16.0f);

        boost::shared_ptr<BlockEntity> headEntity = level->getBlockEntity(headPos);

        if (eTYPE_DERIVED_FROM(BlockEntityType::BED, headEntity->GetType()))
        {
            boost::static_pointer_cast<BedBlockEntity>(headEntity)->setItemValues(itemInstance);
            boost::static_pointer_cast<BedBlockEntity>(headEntity)
                ->setColor(DyeColor::byBlockData(itemInstance->getAuxValue()));
        }

        boost::shared_ptr<BlockEntity> footEntity = level->getBlockEntity(footPos);

        if (eTYPE_DERIVED_FROM(BlockEntityType::BED, footEntity->GetType()))
        {
            boost::static_pointer_cast<BedBlockEntity>(footEntity)->setItemValues(itemInstance);
            boost::static_pointer_cast<BedBlockEntity>(footEntity)
                ->setColor(DyeColor::byBlockData(itemInstance->getAuxValue()));
        }

        level->blockUpdated(footPos, block, false);
        level->blockUpdated(headPos, headState->getBlock(), false);
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

TextureAtlasSprite* BedItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer == 0)
    {
        return mBedIcon;
    }

    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

bool BedItem::hasMultipleSpriteLayers()
{
    return true;
}

void BedItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mBedIcon = iconRegister->registerIcon(BED_OVERLAY_ICON_NAME);
}

int BedItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    unsigned int auxValue = (unsigned int)itemInstance->getAuxValue();
    if (auxValue >= 16)
    {
        auxValue = 0;
    }
    return (int)BED_DESCRIPTION_IDS[auxValue];
}
