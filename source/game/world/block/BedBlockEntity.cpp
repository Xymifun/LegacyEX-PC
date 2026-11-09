#include "game/world/block/BedBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BedBlock.hpp"

BedBlockEntity::BedBlockEntity() : BlockEntity(), mColor(DyeColor::RED)
{
}

void BedBlockEntity::setColor(const DyeColor* color)
{
    mColor = color;
    setChanged();
}

void BedBlockEntity::setItemValues(not_null_ptr<ItemInstance> item)
{
    setColor(DyeColor::byBlockData(item->getAuxValue()));
}

const DyeColor* BedBlockEntity::getColor()
{
    return mColor;
}

not_null_ptr<ItemInstance> BedBlockEntity::getItem()
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, mColor->getBlockData()));
}

boost::shared_ptr<BlockEntity> BedBlockEntity::clone()
{
    boost::shared_ptr<BedBlockEntity> result(new BedBlockEntity());
    BlockEntity::clone(result);
    result->mColor = mColor;
    return result;
}

void BedBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    if (tag->contains(L"color"))
    {
        mColor = DyeColor::byBlockData(tag->getInt(L"color"));
    }
}

CompoundTag* BedBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"color", mColor->getBlockData());
    return tag;
}

CompoundTag* BedBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

boost::shared_ptr<Packet> BedBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<ClientboundBlockEntityDataPacket>(
        new ClientboundBlockEntityDataPacket(mPos, 11, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

bool BedBlockEntity::isHeadPiece()
{
    return BedBlock::isHeadPiece(getData());
}

unsigned long long BedBlockEntity::GetType()
{
    return BlockEntityType::BED;
}

BlockEntity* BedBlockEntity::create()
{
    return new BedBlockEntity();
}
