#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Container.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/NonNullList.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Containers.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

ShulkerBoxBlock::ShulkerBoxBlock(const DyeColor* color) : BaseEntityBlock(Material::STONE, MaterialColor::NONE)
{
    DerivedInit();
    mColor = color;
    registerDefaultState(mStateDefinition->any()->setValue(DirectionalBlock::FACING, Direction::UP));
}

boost::shared_ptr<BlockEntity> ShulkerBoxBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> box(new ShulkerBoxBlockEntity(mColor));
    box->setLevel(level);
    return box;
}

bool ShulkerBoxBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool ShulkerBoxBlock::isViewBlocking(const BlockState*)
{
    return true;
}
bool ShulkerBoxBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool ShulkerBoxBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}
EBlockRenderShape ShulkerBoxBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ENTITY;
}

bool ShulkerBoxBlock::isEntityOnChest(Level* level, const AABB* box)
{
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(LivingEntity), box);
    bool onChest = entities->size() != 0;
    delete entities;
    return onChest;
}

bool ShulkerBoxBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                          InteractionHand::EInteractionHand, const Direction* direction, float, float, float, bool)
{
    if (level->mIsClientSide)
        return true;
    if (player->isSpectator())
        return true;
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(
        level->getBlockEntity(pos));
    if (box.get() == NULL)
        return false;
    const Direction* facing = state->getValue(DirectionalBlock::FACING);
    bool canOpen = true;
    if (box->getAnimationStatus() == 0)
    {
        AABB* shape = FULL_BLOCK_SHAPE
                          ->expand(0.5f * facing->getStepX(), 0.5f * facing->getStepY(), 0.5f * facing->getStepZ())
                          ->contract(facing->getStepX(), facing->getStepY(), facing->getStepZ());
        BlockPos adjacent = pos.relative(facing);
        AABB* expanded = shape->move(adjacent);
        canOpen = !level->hasBlockCubes(expanded) && !isEntityOnChest(level, expanded);
    }
    if (canOpen)
        player->openContainer(box);
    return true;
}

const BlockState* ShulkerBoxBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float,
                                                        float, float, int, boost::shared_ptr<LivingEntity>)
{
    return defaultBlockState()->setValue(DirectionalBlock::FACING, direction);
}

BlockStateDefinition* ShulkerBoxBlock::createBlockStateDefinition()
{
    const Property* properties[] = {DirectionalBlock::FACING};
    return new BlockStateDefinition(this, properties, 1);
}

int ShulkerBoxBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING)->get3DDataValue();
}

const BlockState* ShulkerBoxBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    return defaultBlockState()->setValue(DirectionalBlock::FACING, facing);
}

bool ShulkerBoxBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState*,
                                        boost::shared_ptr<Player> player)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    bool isShulkerBox = false;
    if (entity != NULL)
    {
        unsigned long long type = entity->GetType();
        isShulkerBox = type == BlockEntityType::SHULKER_BOX;
    }
    if (isShulkerBox)
    {
        boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::static_pointer_cast<ShulkerBoxBlockEntity>(entity);
        box->setDestroyedByCreativePlayer(player->mAbilities.mInstabuild);
        box->unpackLootTable(player);
    }
    return false;
}

void ShulkerBoxBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                     int lootBonus)
{
}

void ShulkerBoxBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(
            level->getBlockEntity(pos));
        if (box.get() != NULL)
            box->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                               itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
    }
}

void ShulkerBoxBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(entity);
    if (box.get() != NULL)
    {
        if (!box.get()->isCleared() && box.get()->shouldDrop())
        {
            not_null_ptr<ItemInstance> item(new ItemInstance(Item::byBlock(this)));
            CompoundTag* rootTag = new CompoundTag();
            CompoundTag* entityTag = new CompoundTag();
            rootTag->put(BlockItem::BLOCK_ENTITY_TAG, box->saveToTag(entityTag));
            item->setTag(rootTag);
            if (box->hasCustomName())
            {
                item->setHoverName(box->getName());
                box->setCustomName(L"", L"", false, true);
            }
            Block::popResource(level, pos, item);
        }
        level->updateNeighbourForOutputSignal(pos, state->getBlock());
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

void ShulkerBoxBlock::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                      std::vector<HtmlString>* lines, bool advanced)
{
    Block::appendHoverText(itemInstance, player, lines, advanced);

    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL && tag->contains(BlockItem::BLOCK_ENTITY_TAG, Tag::TAG_COMPOUND))
    {
        CompoundTag* blockEntityTag = tag->getCompound(BlockItem::BLOCK_ENTITY_TAG);
        if (blockEntityTag->contains(L"LootTable", Tag::TAG_STRING))
        {
            lines->push_back(HtmlString(L"???????", eMinecraftColour_NOTSET, false, false, 0));
        }

        if (blockEntityTag->contains(L"Items", Tag::TAG_LIST))
        {
            NonNullList<not_null_ptr<ItemInstance> > items = NonNullList<not_null_ptr<ItemInstance> >::withSize(
                27, ItemInstance::EMPTY);
            ContainerHelper::loadAllItems(blockEntityTag, items);

            int displayedItems = 0;
            int totalItems = 0;
            for (unsigned int index = 0; index < items.size(); ++index)
            {
                not_null_ptr<ItemInstance> item = items[index];
                if (!item->isEmpty())
                {
                    ++totalItems;
                    if (displayedItems > 4)
                        continue;
                    ++displayedItems;
                    lines->push_back(HtmlString(formatwstr(L"%ls x%d", item->getHoverName().c_str(), item->getCount()),
                                                eMinecraftColour_NOTSET, false, false, 0));
                }
            }

            delete[] items.items.data;

            if (totalItems - displayedItems > 0)
            {
                std::wstring moreText = L10N::GetString(IDS_SHULKER_BOX_MORE);
                moreText = replaceAll(moreText, L"%s", L"%d");
                HtmlString moreLine(formatwstr(moreText.c_str(), totalItems - displayedItems), eMinecraftColour_NOTSET,
                                    true, false, 0);
                lines->push_back(moreLine);
            }
        }
    }
}

EPistonPushReaction ShulkerBoxBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}

const AABB* ShulkerBoxBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(
        levelSource->getBlockEntity(pos));
    if (box.get() != NULL)
        return box->getBoundingBox(state);
    return FULL_BLOCK_SHAPE;
}

bool ShulkerBoxBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int ShulkerBoxBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromContainer(
        boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos)));
}

not_null_ptr<ItemInstance> ShulkerBoxBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                                 const BlockState* state)
{
    not_null_ptr<ItemInstance> item = Block::getCloneItemInstance(level, pos, state);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(
        level->getBlockEntity(pos));
    CompoundTag* tag = box->saveToTag(new CompoundTag());
    if (!tag->isEmpty())
        item->addTagElement(BlockItem::BLOCK_ENTITY_TAG, tag);
    return item;
}

const DyeColor* ShulkerBoxBlock::getDyeColor()
{
    return mColor;
}

const DyeColor* ShulkerBoxBlock::getColorFromBlock(Block* block)
{
    ShulkerBoxBlock* shulker = dynamic_cast<ShulkerBoxBlock*>(block);
    if (shulker != NULL)
        return shulker->getDyeColor();
    return DyeColor::PURPLE;
}

Block* ShulkerBoxBlock::getBlockByColor(const DyeColor* color)
{
    switch (color->getBlockData())
    {
    case 0:
        return Blocks::white_shulker_box;
    case 1:
        return Blocks::orange_shulker_box;
    case 2:
        return Blocks::magenta_shulker_box;
    case 3:
        return Blocks::light_blue_shulker_box;
    case 4:
        return Blocks::yellow_shulker_box;
    case 5:
        return Blocks::lime_shulker_box;
    case 6:
        return Blocks::pink_shulker_box;
    case 7:
        return Blocks::gray_shulker_box;
    case 8:
        return Blocks::silver_shulker_box;
    case 9:
        return Blocks::cyan_shulker_box;
    case 10:
    default:
        return Blocks::purple_shulker_box;
    case 11:
        return Blocks::blue_shulker_box;
    case 12:
        return Blocks::brown_shulker_box;
    case 13:
        return Blocks::green_shulker_box;
    case 14:
        return Blocks::red_shulker_box;
    case 15:
        return Blocks::black_shulker_box;
    }
}

not_null_ptr<ItemInstance> ShulkerBoxBlock::getColoredItemInstance(const DyeColor* color)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getBlockByColor(color)));
}

const BlockState* ShulkerBoxBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(DirectionalBlock::FACING,
                           rotation->rotate(state->getTypedBoxed(DirectionalBlock::FACING)->getValue()));
}

const BlockState* ShulkerBoxBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->rotate(mirror->getRotation(state->getValue(DirectionalBlock::FACING)));
}

EBlockFaceShape ShulkerBoxBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                   const BlockPos& pos, const Direction* direction)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    const Direction* facing = state->getValue(DirectionalBlock::FACING);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(
        levelSource->getBlockEntity(pos));
    if (box.get() != NULL)
    {
        int animationStatus = box->getAnimationStatus();
        if (animationStatus == 0
            || (animationStatus == 2 && (facing == direction->getOpposite() || facing == direction)))
        {
            return BLOCK_FACE_SOLID;
        }
    }
    return BLOCK_FACE_UNDEFINED;
}

unsigned int ShulkerBoxBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_OPEN;
    return Block::GetInteractTooltip(tooltipData);
}
