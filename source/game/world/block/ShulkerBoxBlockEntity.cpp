#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/core/LockCode.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/Level.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"

#include <algorithm>

#include "game/core/CensoredString.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"

int shulkerBoxSlots[27] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                           14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};

arrayWithLength<int> ShulkerBoxBlockEntity::SLOTS(shulkerBoxSlots, 27);

const int ShulkerBoxBlockEntity::COLUMNS = 9;
const int ShulkerBoxBlockEntity::ROWS = 3;
const float ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR = 0.5F;
const float ShulkerBoxBlockEntity::LID_ROTATION_DEGREES = 270.0F;

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity(const DyeColor* color)
    : RandomizableLockableContainerBlockEntity(),
      WorldlyContainer()
{
    _init(color);
}

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity() : RandomizableLockableContainerBlockEntity(), WorldlyContainer()
{
    _init(NULL);
}

ShulkerBoxBlockEntity::~ShulkerBoxBlockEntity()
{
    delete[] mItems.items.data;
}

BlockEntity* ShulkerBoxBlockEntity::create()
{
    return new ShulkerBoxBlockEntity();
}

void ShulkerBoxBlockEntity::_init(const DyeColor* color)
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, ItemInstance::EMPTY);
    mColor = color;
    mAnimationStatus = 0;
    mProgress = 0.0f;
    mProgressOld = 0.0f;
    mCleared = false;
    mOpenCount = 0;
    mDestroyedByCreativePlayer = false;
}

std::wstring ShulkerBoxBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

std::wstring ShulkerBoxBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

unsigned int ShulkerBoxBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool ShulkerBoxBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int ShulkerBoxBlockEntity::getMaxStackSize()
{
    return 64;
}

void ShulkerBoxBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

void ShulkerBoxBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return;
    }

    if (mOpenCount < 0)
    {
        mOpenCount = 0;
    }

    ++mOpenCount;
    mLevel->blockEvent(mPos, getBlock(), 1, mOpenCount);

    if (mOpenCount == 1)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mPos, SoundEvents::SHULKER_BOX_OPEN,
                          SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }
}

void ShulkerBoxBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return;
    }

    --mOpenCount;
    mLevel->blockEvent(mPos, getBlock(), 1, mOpenCount);

    if (mOpenCount <= 0)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mPos, SoundEvents::SHULKER_BOX_CLOSE,
                          SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }
}

void ShulkerBoxBlockEntity::clearContent()
{
    mCleared = true;
    RandomizableLockableContainerBlockEntity::clearContent();
}

AbstractContainerMenu* ShulkerBoxBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                         boost::shared_ptr<Player> player)
{
    return new ShulkerBoxMenu(inventory, boost::static_pointer_cast<ShulkerBoxBlockEntity>(shared_from_this()), player);
}

int ShulkerBoxBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::SHULKER_BOX;
}

arrayWithLength<int> ShulkerBoxBlockEntity::getSlotsForFace(const Direction* face)
{
    return SLOTS;
}

bool ShulkerBoxBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return dynamic_cast<ShulkerBoxBlock*>(Block::byItem(item->getItem())) == NULL;
}

bool ShulkerBoxBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return true;
}

int ShulkerBoxBlockEntity::getAnimationStatus()
{
    return mAnimationStatus;
}

void ShulkerBoxBlockEntity::setDestroyedByCreativePlayer(bool value)
{
    mDestroyedByCreativePlayer = value;
}

bool ShulkerBoxBlockEntity::isCleared()
{
    return mCleared;
}

bool ShulkerBoxBlockEntity::isDestroyedByCreativePlayer()
{
    return mDestroyedByCreativePlayer;
}

bool ShulkerBoxBlockEntity::shouldDrop()
{
    return !isDestroyedByCreativePlayer() || !isEmpty() || hasCustomName() || mLootTable != NULL;
}

CompoundTag* ShulkerBoxBlockEntity::saveToTag(CompoundTag* tag)
{
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems, false);
    }
    mCustomName->saveToTag(tag);
    if (!tag->contains(L"Lock") && isLocked())
    {
        getLockKeyCode()->addTag(tag);
    }
    return tag;
}

float ShulkerBoxBlockEntity::getProgress(float partialTick)
{
    return mProgressOld + (mProgress - mProgressOld) * partialTick;
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const Direction* direction)
{
    return Block::FULL_BLOCK_SHAPE->expand(0.5f * getProgress(1.0f) * direction->getStepX(),
                                           0.5f * getProgress(1.0f) * direction->getStepY(),
                                           0.5f * getProgress(1.0f) * direction->getStepZ());
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const BlockState* state)
{
    return getBoundingBox(state->getValue(DirectionalBlock::FACING));
}

AABB* ShulkerBoxBlockEntity::getTopBoundingBox(const Direction* direction)
{
    const Direction* opposite = direction->getOpposite();
    return getBoundingBox(direction)->contract(static_cast<double>(opposite->getStepX()),
                                               static_cast<double>(opposite->getStepY()),
                                               static_cast<double>(opposite->getStepZ()));
}

void ShulkerBoxBlockEntity::moveCollidedEntities()
{
    const BlockState* state = mLevel->getBlockState(getBlockPos());
    if (dynamic_cast<ShulkerBoxBlock*>(state->getBlock()) == NULL)
    {
        return;
    }
    const Direction* direction = state->getValue(DirectionalBlock::FACING);
    AABB* box = getTopBoundingBox(direction)->move(mPos);
    const std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(boost::shared_ptr<Entity>(), box,
                                                                                  NULL);
    if (!entities->empty())
    {
        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            boost::shared_ptr<Entity> entity = entities->at(i);
            if (entity->getPistonPushReaction() == 3)
            {
                continue;
            }
            double dx = 0.0;
            double dy = 0.0;
            double dz = 0.0;
            AABB* entityBox = entity->mBoundingBox;
            switch (direction->getAxis()->getId())
            {
            case Direction::Axis::eAxis_X:
                if (direction->getAxisDirection() == Direction::AxisDirection::POSITIVE)
                {
                    dx = box->mMaxX - entityBox->mMinX;
                }
                else
                {
                    dx = entityBox->mMaxX - box->mMinX;
                }
                dx += 0.01;
                break;
            case Direction::Axis::eAxis_Y:
                if (direction->getAxisDirection() == Direction::AxisDirection::POSITIVE)
                {
                    dy = box->mMaxY - entityBox->mMinY;
                }
                else
                {
                    dy = entityBox->mMaxY - box->mMinY;
                }
                dy += 0.01;
                break;
            case Direction::Axis::eAxis_Z:
                if (direction->getAxisDirection() == Direction::AxisDirection::POSITIVE)
                {
                    dz = box->mMaxZ - entityBox->mMinZ;
                }
                else
                {
                    dz = entityBox->mMaxZ - box->mMinZ;
                }
                dz += 0.01;
                break;
            }
            entity->move(eMoverType_ShulkerBox, dx * direction->getStepX(), dy * direction->getStepY(),
                         dz * direction->getStepZ(), false);
        }
    }
}

void ShulkerBoxBlockEntity::updateAnimation()
{
    mProgressOld = mProgress;
    switch (mAnimationStatus)
    {
    case ANIMATION_CLOSED:
        mProgress = 0.0f;
        break;
    case ANIMATION_OPENING:
        mProgress += 0.1f;
        if (mProgress >= 1.0f)
        {
            moveCollidedEntities();
            mAnimationStatus = ANIMATION_OPENED;
            mProgress = 1.0f;
        }
        break;
    case ANIMATION_CLOSING:
        mProgress -= 0.1f;
        if (mProgress <= 0.0f)
        {
            mAnimationStatus = ANIMATION_CLOSED;
            mProgress = 0.0f;
        }
        break;
    case ANIMATION_OPENED:
        mProgress = 1.0f;
        break;
    }
}

void ShulkerBoxBlockEntity::tick()
{
    updateAnimation();
    if (mAnimationStatus == ANIMATION_OPENING || mAnimationStatus == ANIMATION_CLOSING)
    {
        moveCollidedEntities();
    }
}

bool ShulkerBoxBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        if (param == 0)
        {
            mAnimationStatus = ANIMATION_CLOSING;
        }
        else if (param == 1)
        {
            mAnimationStatus = ANIMATION_OPENING;
        }
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ShulkerBoxBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::SHULKER_BOX, L"Items"));
}

void ShulkerBoxBlockEntity::loadFromTag(CompoundTag* tag)
{
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
    {
        if (tag->contains(L"Items", Tag::TAG_LIST))
        {
            ContainerHelper::loadAllItems(tag, mItems);
        }
    }
    mCustomName->loadFromTag(tag);
}

void ShulkerBoxBlockEntity::load(CompoundTag* tag)
{
    LockableContainerBlockEntity::load(tag);
    loadFromTag(tag);
}

CompoundTag* ShulkerBoxBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    return saveToTag(tag);
}

NonNullList<not_null_ptr<ItemInstance> > ShulkerBoxBlockEntity::getItems()
{
    return mItems;
}

void ShulkerBoxBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

const DyeColor* ShulkerBoxBlockEntity::getColor()
{
    if (mColor == NULL)
        mColor = ShulkerBoxBlock::getColorFromBlock(getBlock());
    return mColor;
}

boost::shared_ptr<Packet> ShulkerBoxBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(
        new ClientboundBlockEntityDataPacket(mPos, 10, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

boost::shared_ptr<BlockEntity> ShulkerBoxBlockEntity::clone()
{
    boost::shared_ptr<ShulkerBoxBlockEntity> result(new ShulkerBoxBlockEntity());
    BlockEntity::clone(result);
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems.set(i, mItems[i]->copy());
    }
    result->mCleared = mCleared;
    result->mOpenCount = mOpenCount;
    result->mAnimationStatus = mAnimationStatus;
    result->mProgress = mProgress;
    result->mProgressOld = mProgressOld;
    result->mColor = mColor;
    result->mDestroyedByCreativePlayer = mDestroyedByCreativePlayer;
    result->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                             mCustomName->isUUIDLocal());
    return result;
}

unsigned long long ShulkerBoxBlockEntity::GetType()
{
    return BlockEntityType::SHULKER_BOX;
}

ShulkerBoxBlockEntity& ShulkerBoxBlockEntity::operator=(const ShulkerBoxBlockEntity& other)
{
    RandomizableLockableContainerBlockEntity::operator=(other);
    mItems = other.mItems;
    mCleared = other.mCleared;
    mOpenCount = other.mOpenCount;
    mAnimationStatus = other.mAnimationStatus;
    mProgress = other.mProgress;
    mProgressOld = other.mProgressOld;
    mColor = other.mColor;
    mDestroyedByCreativePlayer = other.mDestroyedByCreativePlayer;
    return *this;
}
