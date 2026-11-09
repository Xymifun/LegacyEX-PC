#ifndef GAME_WORLD_BLOCK_LOCKABLECONTAINERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_LOCKABLECONTAINERBLOCKENTITY_HPP

#include <string>

#include "game/world/block/BlockEntity.hpp"
#include "game/item/LockableContainer.hpp"

class CompoundTag;
class LockCode;

class LockableContainerBlockEntity : public BlockEntity, public LockableContainer
{
public:
    LockableContainerBlockEntity();
    virtual ~LockableContainerBlockEntity();

    virtual std::wstring getDisplayName();
    virtual bool isLocked();
    virtual LockCode* getLockKeyCode();
    virtual void setLocked(LockCode* lockCode);
    virtual void unlock();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);

protected:
    LockCode* mLockKeyCode;
};

#endif
