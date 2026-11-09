#include "game/entity/ai/OpenDoorGoal.hpp"

#include "game/entity/mob/Mob.hpp"
#include "game/world/block/DoorBlock.hpp"

OpenDoorGoal::OpenDoorGoal(Mob* mob, bool closeDoor) : DoorInteractGoal(mob), mCloseDoorTimer(0)
{
    mMob = mob;
    mCloseDoor = closeDoor;
}

OpenDoorGoal::~OpenDoorGoal()
{
}

bool OpenDoorGoal::canContinueToUse()
{
    return mCloseDoor && mCloseDoorTimer > 0 && DoorInteractGoal::canContinueToUse();
}

void OpenDoorGoal::start()
{
    mCloseDoorTimer = 20;
    static_cast<DoorBlock*>(mDoorBlock)->setOpen(mMob->mLevel, mDoorPos, true);
}

void OpenDoorGoal::stop()
{
    if (!mCloseDoor)
        return;

    static_cast<DoorBlock*>(mDoorBlock)->setOpen(mMob->mLevel, mDoorPos, false);
}

void OpenDoorGoal::tick()
{
    --mCloseDoorTimer;
    DoorInteractGoal::tick();
}
