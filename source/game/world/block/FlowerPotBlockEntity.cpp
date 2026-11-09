#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

FlowerPotBlockEntity::FlowerPotBlockEntity(Item* item, int data) : BlockEntity(), mPlantItem(item), mItemData(data)
{
}

FlowerPotBlockEntity::FlowerPotBlockEntity() : BlockEntity(), mPlantItem(NULL), mItemData(0)
{
}

BlockEntity* FlowerPotBlockEntity::create()
{
    return new FlowerPotBlockEntity();
}

Item* FlowerPotBlockEntity::getPlantItem()
{
    return mPlantItem;
}

not_null_ptr<ItemInstance> FlowerPotBlockEntity::getPlantItemInstance()
{
    return mPlantItem == NULL ? ItemInstance::EMPTY
                              : not_null_ptr<ItemInstance>(new ItemInstance(mPlantItem, 1, mItemData));
}

void FlowerPotBlockEntity::setItemInstance(const not_null_ptr<ItemInstance>& item)
{
    mPlantItem = item->getItem();
    mItemData = item->getAuxValue();
}

int FlowerPotBlockEntity::getItemData()
{
    return mItemData;
}

void FlowerPotBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
}

CompoundTag* FlowerPotBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"Item", Item::VALIDgetId(mPlantItem));
    tag->putInt(L"Data", mItemData);
    return tag;
}

void FlowerPotBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    if (tag->contains(L"Item", Tag::TAG_STRING))
    {
        mPlantItem = Item::byString(tag->getString(L"Item"));
    }
    else if (tag->getInt(L"Item") != 0)
    {
        mPlantItem = Item::byId(tag->getInt(L"Item"));
    }
    mItemData = tag->getInt(L"Data");
}

boost::shared_ptr<Packet> FlowerPotBlockEntity::getUpdatePacket()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return boost::shared_ptr<Packet>(
        new ClientboundBlockEntityDataPacket(mPos, 5, mLevel->mDimension->getType()->getId(), tag));
}

CompoundTag* FlowerPotBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

boost::shared_ptr<BlockEntity> FlowerPotBlockEntity::clone()
{
    boost::shared_ptr<FlowerPotBlockEntity> result(new FlowerPotBlockEntity());
    BlockEntity::clone(result);
    result->mPlantItem = mPlantItem;
    result->mItemData = mItemData;
    return result;
}

unsigned long long FlowerPotBlockEntity::GetType()
{
    return BlockEntityType::FLOWER_POT;
}
