#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

CauldronBlockEntity::CauldronBlockEntity()
    : BlockEntity(),
      Container(),
      mPotionId(L""),
      mHasCustomColor(false),
      mCustomColor(0),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(10, ItemInstance::EMPTY)),
      mCustomName(new CensoredString())
{
}

CauldronBlockEntity::~CauldronBlockEntity()
{
    delete[] mItems.items.data;
}

BlockEntity* CauldronBlockEntity::create()
{
    return new CauldronBlockEntity();
}

std::wstring CauldronBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return L10N::GetString(IDS_ITEM_CAULDRON);
}

std::wstring CauldronBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return L"";
}

bool CauldronBlockEntity::hasCustomName()
{
    if (!mCustomName->checkParentalFilter())
    {
        return false;
    }
    return mCustomName->raw().size() != 0;
}

std::wstring CauldronBlockEntity::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

unsigned int CauldronBlockEntity::getContainerSize()
{
    return 10;
}

bool CauldronBlockEntity::isEmpty()
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

not_null_ptr<ItemInstance> CauldronBlockEntity::getItem(unsigned int slot)
{
    if (slot < mItems.size())
    {
        return mItems[slot];
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, slot);
}

void CauldronBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot < mItems.size())
    {
        mItems[slot] = item;
    }
    BlockEntity::setChanged();
}

int CauldronBlockEntity::getMaxStackSize()
{
    return 64;
}

bool CauldronBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
    {
        return false;
    }
    return true;
}

void CauldronBlockEntity::startOpen(boost::shared_ptr<Player>)
{
}

void CauldronBlockEntity::stopOpen(boost::shared_ptr<Player>)
{
}

bool CauldronBlockEntity::canPlaceItem(int, not_null_ptr<ItemInstance>)
{
    return false;
}

int CauldronBlockEntity::getContainerData(int)
{
    return 0;
}

void CauldronBlockEntity::setContainerData(int, int)
{
}

int CauldronBlockEntity::getContainerDataCount()
{
    return 0;
}

void CauldronBlockEntity::clearContent()
{
    mItems.clear();
}

std::wstring CauldronBlockEntity::getPotionId() const
{
    return mPotionId;
}

void CauldronBlockEntity::setPotionId(const std::wstring& potionId)
{
    mPotionId = potionId;
    BlockEntity::setChanged();
}

void CauldronBlockEntity::setCustomColor(const int& color)
{
    mHasCustomColor = color != 0;
    mCustomColor = color;
    BlockEntity::setChanged();
}

int CauldronBlockEntity::getPotionColor() const
{
    return MobEffectInstance::getColorValue(&Potion::byName(mPotionId)->getEffects());
}

int CauldronBlockEntity::getCustomColor() const
{
    return mCustomColor;
}

int CauldronBlockEntity::getColor() const
{
    if (getPotionId().compare(L"") != 0)
    {
        return getPotionColor();
    }
    if (mHasCustomColor)
    {
        return getCustomColor();
    }
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
}

Potion::EType CauldronBlockEntity::getPotionType() const
{
    return mPotionType;
}
void CauldronBlockEntity::setPotionType(Potion::EType type)
{
    mPotionType = type;
}

int CauldronBlockEntity::getMixDyeColor()
{
    int rgb[3] = {0, 0, 0};
    int count = 0;
    if (mHasCustomColor)
    {
        int color = mCustomColor;
        rgb[0] = (color >> 16) & 255;
        rgb[1] = (color >> 8) & 255;
        rgb[2] = color & 255;
        count = 1;
    }
    for (int slot = 2; slot < 10; ++slot)
    {
        not_null_ptr<ItemInstance> item = getItem(slot);
        if (!item->isEmpty() && item->getItem() == Items::dye)
        {
            arrayWithLength<const float> colors = DyeColor::byItemData(item->getAuxValue())->getTextureDiffuseColors();
            int red = static_cast<int>(colors[0] * 255.0f);
            int green = static_cast<int>(colors[1] * 255.0f);
            int blue = static_cast<int>(colors[2] * 255.0f);
            int amount = item->getCount();
            rgb[0] += amount * red;
            rgb[1] += amount * green;
            rgb[2] += amount * blue;
            count += amount;
        }
    }
    if (count > 0)
    {
        return (((rgb[0] / count) << 8) + rgb[1] / count << 8) + rgb[2] / count;
    }
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
}

void CauldronBlockEntity::mixDyes()
{
    setCustomColor(getMixDyeColor());
    for (int slot = 2; slot < 10; ++slot)
    {
        setItem(slot, ItemInstance::EMPTY);
    }
}

void CauldronBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mPotionId = tag->getString(L"PotionId");
    bool splash = tag->getByte(L"IsSplash") != 0;
    if (splash)
    {
        mPotionType = Potion::SPLASH;
    }
    else
    {
        mPotionType = static_cast<Potion::EType>(tag->getShort(L"PotionType"));
    }
    mHasCustomColor = tag->contains(L"CustomColor");
    if (mHasCustomColor)
    {
        mCustomColor = tag->getInt(L"CustomColor");
    }
}

void CauldronBlockEntity::loadFromPacket(Level* level, const BlockPos& pos, CompoundTag* tag)
{
    load(tag);
    if (level->getBlockId(pos.x, pos.y, pos.z) == BlockId::CAULDRON)
    {
        level->setData(pos, level->getBlockData(pos.x, pos.y, pos.z), 2, true);
    }
}

CompoundTag* CauldronBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putString(L"PotionId", mPotionId);
    tag->putShort(L"PotionType", static_cast<short>(mPotionType));
    if (mHasCustomColor)
    {
        tag->putInt(L"CustomColor", mCustomColor);
    }
    return tag;
}

void CauldronBlockEntity::tick()
{
    BlockEntity::tick();
}

boost::shared_ptr<Packet> CauldronBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(
        new ClientboundBlockEntityDataPacket(mPos, 12, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* CauldronBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

boost::shared_ptr<BlockEntity> CauldronBlockEntity::clone()
{
    boost::shared_ptr<CauldronBlockEntity> result(new CauldronBlockEntity());
    BlockEntity::clone(result);
    result->mPotionId = mPotionId;
    result->mPotionType = mPotionType;
    result->mHasCustomColor = mHasCustomColor;
    result->mCustomColor = mCustomColor;
    result->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(),
                             mCustomName->isUUIDLocal());
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems.set(i, mItems[i]->copy());
    }
    return result;
}

void CauldronBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}
unsigned long long CauldronBlockEntity::GetType()
{
    return BlockEntityType::CAULDRON;
}
