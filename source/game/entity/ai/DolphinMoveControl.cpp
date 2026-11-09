#include "game/entity/ai/DolphinMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

DolphinMoveControl::DolphinMoveControl(Dolphin* dolphin)
    : MoveControl(dolphin),
      mMaxYRotChange(85.0f),
      mMaxXRotChange(10.0f)
{
}

DolphinMoveControl::~DolphinMoveControl()
{
}

void DolphinMoveControl::tick()
{
    Mob* mob = mMob;
    if (mob->isInWater())
    {
        mob->mYd += 0.005;
    }

    if (mOperation != MOVE_TO || mob->getNavigation()->isDone())
    {
        mob->setSpeed(0.0f);
        mob->setXxa(0.0f);
        mob->setYya(0.0f);
        mob->setZza(0.0f);
        mob->setMoving(false);
        return;
    }

    mob->setMoving(true);

    double dx = mWantedX - mob->mX;
    double dy = mWantedY + 0.5 - mob->mY;
    double dz = mWantedZ - mob->mZ;
    if (dx * dx + dy * dy + dz * dz < MIN_SPEED_SQR)
    {
        mMob->setZza(0.0f);
        return;
    }

    mob->mYRot = rotlerp(mob->mYRot, (float)atan2(dz, dx) * 57.295776f - 90.0f, mMaxXRotChange);
    mob->mYBodyRot = mob->mYRot;
    mob->mYHeadRot = mob->mYRot;

    float speed = (float)(mSpeedModifier * mob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());

    if (mob->isInWater())
    {
        mob->setSpeed(speed * 0.02f);

        float yDist = (float)dy;
        float horizontal = Mth::sqrt(dx * dx + dz * dz);
        float wantedXRot = Mth::wrapDegrees(-atan2f(yDist, horizontal * 57.295776f));
        wantedXRot = Mth::clamp(wantedXRot, -mMaxYRotChange, mMaxYRotChange);
        mob->mXRot = rotlerp(mob->mXRot, wantedXRot, 5.0f);
        float zza = Mth::cos(mob->mXRot * Mth::DEG_TO_RAD) * speed;
        mob->setZza(zza);
        mob->setYya((float)dy * speed);
    }
    else
    {
        mob->setSpeed(speed * 0.1f);
    }
}
