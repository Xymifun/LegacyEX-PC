#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"

EnchantmentTableEntity::EnchantmentTableEntity() : BlockEntity(), InteractionObject()
{
    mRandom = new Random();
    mTime = 0;
    mFlip = 0.0F;
    mFlipOld = 0.0F;
    mFlipTarget = 0.0F;
    mFlipSpeed = 0.0F;
    mOpen = 0.0F;
    mOpenOld = 0.0F;
    mRot = 0.0F;
    mRotOld = 0.0F;
    mRotTarget = 0.0F;
    mCustomName = boost::shared_ptr<CensoredString>(new CensoredString());
}

EnchantmentTableEntity::~EnchantmentTableEntity()
{
    delete mRandom;
}

BlockEntity* EnchantmentTableEntity::create()
{
    return new EnchantmentTableEntity();
}

std::wstring EnchantmentTableEntity::getName()
{
    if (hasCustomName())
        return mCustomName->get();
    return std::wstring(L"");
}

std::wstring EnchantmentTableEntity::getCustomName()
{
    if (hasCustomName())
        return mCustomName->get();
    return std::wstring(L"");
}

bool EnchantmentTableEntity::hasCustomName()
{
    if (mCustomName->checkParentalFilter() == 0)
        return false;
    return !mCustomName->raw().empty();
}

std::wstring EnchantmentTableEntity::getDisplayName()
{
    if (hasCustomName())
    {
        return getName();
    }
    return getName();
}

AbstractContainerMenu* EnchantmentTableEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                          boost::shared_ptr<Player> player)
{
    return new EnchantmentMenu(inventory, mLevel, mPos);
}

int EnchantmentTableEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::ENCHANTMENT;
}

InteractionObject::NameData EnchantmentTableEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isUUIDLocal(),
                                       mCustomName->isChatCensored());
}

void EnchantmentTableEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored,
                                           bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

CompoundTag* EnchantmentTableEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    return tag;
}

void EnchantmentTableEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
}

void EnchantmentTableEntity::tick()
{
    mOpenOld = mOpen;
    mRotOld = mRot;
    boost::shared_ptr<Player> player = mLevel->getNearestPlayer((double)((float)mPos.x + 0.5F),
                                                                (double)((float)mPos.y + 0.5F),
                                                                (double)((float)mPos.z + 0.5F), 3.0, false, -1.0);

    if (player.get() != NULL)
    {
        Entity* entity = (Entity*)player.get();
        double dx = entity->mX - (double)((float)mPos.x + 0.5F);
        double dz = entity->mZ - (double)((float)mPos.z + 0.5F);
        mRotTarget = (float)std::atan2(dz, dx);
        mOpen += 0.1F;
        if (mOpen < 0.5F || mRandom->nextInt(40) == 0)
        {
            float previousTarget = mFlipTarget;
            do
            {
                mFlipTarget += (float)(mRandom->nextInt(4) - mRandom->nextInt(4));
            } while (previousTarget == mFlipTarget);
        }
    }
    else
    {
        mRotTarget += 0.02F;
        mOpen -= 0.1F;
    }

    float rot = mRot;
    while (rot >= 3.1415927F)
        rot -= 6.2831855F;
    while (rot < -3.1415927F)
        rot += 6.2831855F;
    while (mRotTarget >= 3.1415927F)
        mRotTarget -= 6.2831855F;
    while (mRotTarget < -3.1415927F)
        mRotTarget += 6.2831855F;
    float delta = mRotTarget - rot;
    while (delta >= 3.1415927F)
        delta -= 6.2831855F;
    while (delta < -3.1415927F)
        delta += 6.2831855F;
    mRot = delta * 0.4F + rot;
    mOpen = Mth::clamp(mOpen, 0.0F, 1.0F);
    ++mTime;
    mFlipOld = mFlip;
    float flipDelta = Mth::clamp((mFlipTarget - mFlip) * 0.4F, -0.2F, 0.2F);
    mFlipSpeed += (flipDelta - mFlipSpeed) * 0.9F;
    mFlip += mFlipSpeed;
}

boost::shared_ptr<BlockEntity> EnchantmentTableEntity::clone()
{
    boost::shared_ptr<EnchantmentTableEntity> copy(new EnchantmentTableEntity());
    BlockEntity::clone(copy);
    copy->mTime = mTime;
    copy->mFlip = mFlip;
    copy->mFlipOld = mFlipOld;
    copy->mFlipTarget = mFlipTarget;
    copy->mFlipSpeed = mFlipSpeed;
    copy->mOpen = mOpen;
    copy->mOpenOld = mOpenOld;
    copy->mRot = mRot;
    copy->mRotOld = mRotOld;
    copy->mRotTarget = mRotTarget;
    copy->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                           mCustomName->isUUIDLocal());
    return copy;
}

unsigned long long EnchantmentTableEntity::GetType()
{
    return BlockEntityType::ENCHANTMENT_TABLE_ENTITY;
}
