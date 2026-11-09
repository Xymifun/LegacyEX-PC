#include "game/world/block/SkullBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/SkullBlock.hpp"

SkullBlockEntity::SkullBlockEntity() : BlockEntity()
{
    mSkullType = -1;
    mRotation = 0;
    mExtraType = L"";
    mMouthAnimation = 0;
    mPowered = false;
}

BlockEntity* SkullBlockEntity::create()
{
    return new SkullBlockEntity();
}

int SkullBlockEntity::getSkullType()
{
    return mSkullType;
}

int SkullBlockEntity::ValidateSkullType(int type)
{
    return type >= 0 && type < 6 ? type : 0;
}

void SkullBlockEntity::setSkullType(int type, const std::wstring& extraType)
{
    mSkullType = ValidateSkullType(type);
    mExtraType = extraType;
}

void SkullBlockEntity::setRotation(int rotation)
{
    mRotation = rotation;
}

CompoundTag* SkullBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putByte(L"SkullType", (unsigned char)(mSkullType & 255));
    tag->putByte(L"Rot", (unsigned char)(mRotation & 255));
    tag->putString(L"ExtraType", mExtraType);
    return tag;
}

void SkullBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mSkullType = ValidateSkullType((int)tag->getByte(L"SkullType"));
    mRotation = (int)tag->getByte(L"Rot");
    if (tag->contains(L"ExtraType"))
    {
        mExtraType = tag->getString(L"ExtraType");
    }
}

void SkullBlockEntity::tick()
{
    if (mSkullType == 5)
    {
        if (mLevel->hasNeighborSignal(mPos))
        {
            if (!mPowered)
            {
                mLevel->updateObservingBlocksAt(mPos, getBlock());
            }
            mPowered = true;
            ++mMouthAnimation;
            return;
        }
        if (mPowered)
        {
            mLevel->updateObservingBlocksAt(mPos, getBlock());
        }
        mPowered = false;
    }
}

float SkullBlockEntity::getMouthAnimation(float partialTick)
{
    if (mPowered)
    {
        return (float)mMouthAnimation + partialTick;
    }
    return (float)mMouthAnimation;
}

boost::shared_ptr<Packet> SkullBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<ClientboundBlockEntityDataPacket>(
        new ClientboundBlockEntityDataPacket(mPos, 4, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* SkullBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

bool SkullBlockEntity::isRenderReady()
{
    return mSkullType >= 0 && !isRemoved();
}

int SkullBlockEntity::getRotation()
{
    return mRotation;
}

void SkullBlockEntity::rotate(Rotation* rotation)
{
    if (mLevel == NULL)
        return;

    const BlockState* state = mLevel->getBlockState(getBlockPos());
    if (state->getValue(SkullBlock::FACING) == Direction::UP)
    {
        mRotation = rotation->rotate(mRotation, 16);
    }
}

void SkullBlockEntity::mirror(Mirror* mirror)
{
    if (mLevel == NULL)
        return;

    const BlockState* state = mLevel->getBlockState(getBlockPos());
    if (state->getValue(SkullBlock::FACING) == Direction::UP)
    {
        mRotation = mirror->mirror(mRotation, 16);
    }
}

boost::shared_ptr<BlockEntity> SkullBlockEntity::clone()
{
    boost::shared_ptr<SkullBlockEntity> result(new SkullBlockEntity());
    BlockEntity::clone(result);
    result->mSkullType = ValidateSkullType(mSkullType);
    result->mRotation = mRotation;
    result->mExtraType = mExtraType;
    return result;
}

unsigned long long SkullBlockEntity::GetType()
{
    return BlockEntityType::SKULL;
}
