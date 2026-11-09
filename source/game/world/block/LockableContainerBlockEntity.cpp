#include "game/world/block/LockableContainerBlockEntity.hpp"

#include "game/core/LockCode.hpp"
#include "game/data/CompoundTag.hpp"

LockableContainerBlockEntity::LockableContainerBlockEntity()
{
    mLockKeyCode = LockCode::NO_LOCK;
}

LockableContainerBlockEntity::~LockableContainerBlockEntity()
{
}

std::wstring LockableContainerBlockEntity::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return getName();
}

bool LockableContainerBlockEntity::isLocked()
{
    return mLockKeyCode != NULL && !mLockKeyCode->isEmpty();
}

LockCode* LockableContainerBlockEntity::getLockKeyCode()
{
    return mLockKeyCode;
}

void LockableContainerBlockEntity::setLocked(LockCode* lockCode)
{
    mLockKeyCode = lockCode;
}

void LockableContainerBlockEntity::unlock()
{
    mLockKeyCode = LockCode::NO_LOCK;
}

void LockableContainerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mLockKeyCode = LockCode::fromTag(tag);
}

CompoundTag* LockableContainerBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    if (mLockKeyCode != NULL)
    {
        mLockKeyCode->addTag(tag);
    }
    return tag;
}
