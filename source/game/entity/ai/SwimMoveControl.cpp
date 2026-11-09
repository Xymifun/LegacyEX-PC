#include "game/entity/ai/SwimMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Vec3.hpp"

SwimMoveControl::SwimMoveControl(Mob* mob) : MoveControl(mob)
{
    mMob = mob;
    setSpeedModifier(0.5);
}

SwimMoveControl::~SwimMoveControl()
{
}

void SwimMoveControl::tick()
{
    if (!mMob->isSwimming())
    {
        MoveControl::tick();
        return;
    }

    if (!hasWanted())
    {
        mMob->setSpeed(0.0f);
        mMob->setMoving(false);
        return;
    }

    mOperation = WAIT;

    double dx = getWantedX() - mMob->getPos(0.0f)->x;
    double dz = getWantedZ() - mMob->getPos(0.0f)->z;
    double dy = getWantedY() - mMob->getPos(0.0f)->y;
    double distance = Mth::sqrt(dy * dy + dx * dx + dz * dz);
    dy = dy / distance;
    if (distance >= MIN_SPEED_SQR)
    {
        double horizontalSqr = dx * dx + dz * dz;
        float wantedYRot = 0.0f;
        if (horizontalSqr > 0.10000000149011612)
        {
            wantedYRot = (float)(atan2(dz, dx) * Mth::RAD_TO_DEG) - 90.0f;
        }
        float yRot = Mth::clampRotateDegrees(mMob->mYRot, wantedYRot, 30.0f);
        mMob->mYRot = yRot;
        mMob->mYBodyRot = yRot;

        double attributeSpeed = mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        float speed = (float)(getSpeedModifier() * attributeSpeed);
        mMob->setSpeed(speed);

        float pitch = (float)atan2(dy, static_cast<double>(Mth::sqrt(horizontalSqr)));
        Mob* mob = mMob;
        Vec3* rotation = Vec3::newTemp((double)(pitch * (float)-(double)Mth::RAD_TO_DEG), (double)mob->mYRot, 0.0);
        LookControl* lookControl = static_cast<LookControl*>(mob->getLookControl());
        lookControl->setLookRotation(rotation, 5.0f, 10.0f);

        float sinXRot = Mth::sin(mMob->mXRot * Mth::DEG_TO_RAD);
        float cosXRot = Mth::cos(mMob->mXRot * Mth::DEG_TO_RAD);
        mMob->setZza((float)(cosXRot * speed));
        mMob->setYya(-(float)(sinXRot * speed));

        lookControl = static_cast<LookControl*>(mMob->getLookControl());
        mob = mMob;
        Vec3* target = Vec3::newTemp(mob->getPos(0.0f)->x + dx / distance * 2.0,
                                     mob->getAttachPos(Entity::ENTITY_LOCATION_HEAD, 0.0f)->y + dy / distance,
                                     mMob->getPos(0.0f)->z + dz / distance * 2.0);
        Vec3* wanted = Vec3::newTemp(lookControl->getWantedX(), lookControl->getWantedY(), lookControl->getWantedZ());
        if (!lookControl->isHasWanted())
        {
            wanted->x = target->x;
            wanted->y = target->y;
            wanted->z = target->z;
        }
        lookControl->setLookAt((target->x - wanted->x) * 0.125 + wanted->x, (target->y - wanted->y) * 0.125 + wanted->y,
                               (target->z - wanted->z) * 0.125 + wanted->z, 10.0f, 40.0f);
        mMob->setMoving(true);
    }
}
