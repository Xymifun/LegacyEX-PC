#include "game/item/BannerItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/StandingBannerBlock.hpp"
#include "game/world/block/WallBannerBlock.hpp"
#include "game/world/generation/Player.hpp"

BannerItem::BannerItem() : BlockItem(Blocks::standing_banner)
{
    mMaxStackSize = 16;
    setStackedByData(true);
    setMaxDamage(0);
}

BannerItem::~BannerItem()
{
}

const DyeColor* BannerItem::getColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    return DyeColor::byItemData(itemInstance->getAuxValue() & 15);
}

InteractionResult::EInteractionResult BannerItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate)
{
    BlockPos placePos = pos;
    const BlockState* clickedState = level->getBlockState(placePos);
    bool replaceable = clickedState->getBlock()->mayReplaceWithPlace(level, placePos);

    if (direction == Direction::DOWN)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!clickedState->getMaterial()->isSolid() && !replaceable)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (replaceable && direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    placePos = placePos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance) || !Blocks::standing_banner->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (level->mIsClientSide)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (replaceable)
    {
        placePos = placePos.below();
    }

    if (direction == Direction::UP)
    {
        int rotation = Mth::floor((double)((player->mYRot + 180.0f) * 16.0f / 360.0f) + 0.5) & 15;
        level->setBlock(placePos,
                        Blocks::standing_banner->defaultBlockState()->setValue(StandingBannerBlock::ROTATION, rotation),
                        Block::UPDATE_ALL, false);
    }
    else
    {
        level->setBlock(placePos, Blocks::wall_banner->defaultBlockState()->setValue(BannerBlock::FACING, direction),
                        Block::UPDATE_ALL, false);
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(placePos);
    if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::BANNER, blockEntity->GetType()))
    {
        const SoundType* soundType = mBlock->getSoundType();
        level->playSound(boost::shared_ptr<Player>(), placePos, soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks, (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f, 16.0f);

        boost::static_pointer_cast<BannerBlockEntity>(blockEntity)->setItemValues(itemInstance, false);
        itemInstance->shrink(1);
        return InteractionResult::eInteractionResult_Success;
    }

    player->displayClientMessage((int)IDS_MAX_BANNERS, true);
    return InteractionResult::eInteractionResult_Fail;
}

std::wstring BannerItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    const DyeColor* color = getColor(itemInstance);
    int nameId = BannerPattern::BASE->getDisplayName(color->getBlockData());
    return L10N::GetString(nameId);
}

void BannerItem::appendHoverTextFromBannerBlockEntityTag(not_null_ptr<ItemInstance> itemInstance,
                                                         std::vector<HtmlString>* lines)
{
    CompoundTag* tag = static_cast<CompoundTag*>(itemInstance->getTagElement(BLOCK_ENTITY_TAG));
    if (tag == NULL || !tag->contains(BannerBlockEntity::TAG_PATTERNS))
    {
        return;
    }

    ListTag* patterns = tag->getList(BannerBlockEntity::TAG_PATTERNS, Tag::TAG_COMPOUND);
    for (int i = 0; i < patterns->size() && i < 6; ++i)
    {
        CompoundTag* patternTag = patterns->getCompound(i);
        const DyeColor* color = DyeColor::byItemData(patternTag->getInt(BannerBlockEntity::TAG_COLOR));
        BannerPattern* pattern = BannerPattern::byHash(patternTag->getString(BannerBlockEntity::TAG_PATTERN));
        if (pattern != NULL)
        {
            const wchar_t* text = L10N::GetString(pattern->getDisplayName(color->getBlockData()));
            lines->push_back(HtmlString(std::wstring(text), eMinecraftColour_NOTSET, false, false, 0));
        }
    }
}

void BannerItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced)
{
    appendHoverTextFromBannerBlockEntityTag(itemInstance, lines);
}

ItemInstance* BannerItem::makeBanner(const DyeColor* color, ListTag* patterns)
{
    ItemInstance* item = new ItemInstance(Items::banner, 1, color->getItemData());
    if (patterns != NULL && !patterns->isEmpty())
    {
        ListTag* copiedPatterns = patterns->copy();
        CompoundTag* tag = static_cast<CompoundTag*>(item->getOrCreateTagElement(BLOCK_ENTITY_TAG));
        tag->put(BannerBlockEntity::TAG_PATTERNS, copiedPatterns);
    }
    return item;
}

int BannerItem::getColor(not_null_ptr<ItemInstance> itemInstance, int layer)
{
    if (layer == 0)
    {
        return 0xFFFFFF;
    }
    const DyeColor* color = getColor(itemInstance);
    int diffuseColor = color->getTextureDiffuseColor();
    return diffuseColor;
}

int BannerItem::getFirstPatternName(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = static_cast<CompoundTag*>(itemInstance->getTagElement(BLOCK_ENTITY_TAG));
    if (tag != NULL)
    {
        if (tag->contains(BannerBlockEntity::TAG_PATTERNS))
        {
            ListTag* patterns = tag->getList(BannerBlockEntity::TAG_PATTERNS, Tag::TAG_COMPOUND);
            if (patterns->size() != 0)
            {
                CompoundTag* patternTag = patterns->getCompound(0);
                const DyeColor* color = DyeColor::byItemData(patternTag->getInt(BannerBlockEntity::TAG_COLOR));
                BannerPattern* pattern = BannerPattern::byHash(patternTag->getString(BannerBlockEntity::TAG_PATTERN));
                if (pattern != NULL)
                    return pattern->getDisplayName(color->getBlockData());
            }
        }
    }

    const DyeColor* color = NULL;
    if (tag != NULL && tag->contains(BannerBlockEntity::TAG_BASE))
    {
        color = DyeColor::byItemData(tag->getInt(BannerBlockEntity::TAG_BASE));
    }
    else
    {
        color = DyeColor::byItemData(itemInstance->getAuxValue());
    }
    return BannerPattern::BASE->getDisplayName(color->getBlockData());
}
