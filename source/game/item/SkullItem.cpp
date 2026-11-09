#include "game/item/SkullItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static const int SKULL_DESCRIPTION_IDS[] = {IDS_ITEM_SKULL_SKELETON,  IDS_ITEM_SKULL_WITHER,  IDS_ITEM_SKULL_ZOMBIE,
                                            IDS_ITEM_SKULL_CHARACTER, IDS_ITEM_SKULL_CREEPER, IDS_ITEM_SKULL_DRAGON};

SkullItem::SkullItem()
{
    setMaxDamage(0);
    setStackedByData(true);
}

InteractionResult::EInteractionResult SkullItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                       const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                       const Direction* direction, float hitX, float hitY, float hitZ,
                                                       bool simulate)
{
    if (direction == Direction::DOWN)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos placePos = pos;
    if (!level->getBlockState(placePos)->getBlock()->mayReplaceWithPlace(level, placePos))
    {
        if (!level->getBlockState(placePos)->getMaterial()->isSolid())
        {
            return InteractionResult::eInteractionResult_Fail;
        }
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance) || !Blocks::skull->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (level->mIsClientSide)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (!simulate)
    {
        level->setBlock(placePos, Blocks::skull->defaultBlockState()->setValue(SkullBlock::FACING, direction),
                        Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);

        int rotation = 0;
        if (direction == Direction::UP)
        {
            double rotationSteps = player->mYRot * 16.0f / 360.0f;
            rotation = Mth::floor(rotationSteps + 0.5) & 15;
        }

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(placePos);
        if (eTYPE_DERIVED_FROM(BlockEntityType::SKULL, blockEntity->GetType()))
        {
            boost::shared_ptr<SkullBlockEntity> skull = boost::static_pointer_cast<SkullBlockEntity>(blockEntity);
            skull->setSkullType(itemInstance->getAuxValue(), L"");
            skull->setRotation(rotation);
            static_cast<SkullBlock*>(Blocks::skull)->checkMobSpawn(level, placePos, skull, player);
        }

        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

int SkullItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int SkullItem::getDescriptionId(int auxValue)
{
    if ((unsigned int)auxValue >= 6)
    {
        auxValue = 0;
    }
    return SKULL_DESCRIPTION_IDS[auxValue];
}

int SkullItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();
    if ((unsigned int)auxValue >= 6)
    {
        auxValue = 0;
    }
    return SKULL_DESCRIPTION_IDS[auxValue];
}

std::wstring SkullItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getAuxValue() == 3 && itemInstance->hasTag())
    {
        if (itemInstance->getTag()->contains(L"SkullOwner", Tag::TAG_STRING))
        {
            return itemInstance->getTag()->getString(L"SkullOwner");
        }
        else if (itemInstance->getTag()->contains(L"SkullOwner", Tag::TAG_COMPOUND))
        {
            CompoundTag* owner = itemInstance->getTag()->getCompound(L"SkullOwner");
            if (owner->contains(L"Name", Tag::TAG_STRING))
            {
                return owner->getString(L"Name");
            }
        }
    }
    return Item::getName(itemInstance);
}

bool SkullItem::verifyTagAfterLoad(CompoundTag* tag)
{
    return true;
}
