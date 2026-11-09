#include "game/entity/ai/LlamaFollowCaravanGoal.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <typeinfo>
#include <vector>

#include "game/core/GenericStats.hpp"
#include "game/core/Llama.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

LlamaFollowCaravanGoal::LlamaFollowCaravanGoal(Llama* llama, double speedModifier)
    : mLlama(llama),
      mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(1);
}

LlamaFollowCaravanGoal::~LlamaFollowCaravanGoal()
{
}

bool LlamaFollowCaravanGoal::firstIsLeashed(Llama* llama, int& distance, boost::shared_ptr<Entity>& leashHolder)
{
    if (distance <= 8 && llama->inCaravan())
    {
        ++distance;
        if (llama->getCaravanHead()->isLeashed())
        {
            leashHolder = llama->getCaravanHead()->getLeashHolder();
            return true;
        }

        return firstIsLeashed(llama->getCaravanHead().get(), distance, leashHolder);
    }

    return false;
}

bool LlamaFollowCaravanGoal::canUse()
{
    if (mLlama == NULL || mLlama->isLeashed() || mLlama->inCaravan())
    {
        return false;
    }

    const std::type_info& type = typeid(*mLlama);
    std::vector<boost::shared_ptr<Entity> >* llamas = mLlama->mLevel->getEntitiesOfClass(
        type, mLlama->mBoundingBox->grow(9.0, 4.0, 9.0));

    boost::shared_ptr<Llama> closest;
    double closestDistance = 1.7976931348623157e+308;

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = llamas->begin(); it != llamas->end(); ++it)
    {
        boost::shared_ptr<Llama> candidate = boost::static_pointer_cast<Llama>(*it);
        if (!candidate->inCaravan() || candidate->hasCaravanTail())
        {
            continue;
        }

        double distance = mLlama->distanceToSqr(candidate);
        if (distance > closestDistance)
        {
            continue;
        }

        closestDistance = distance;
        closest = candidate;
    }

    if (!closest)
    {
        for (it = llamas->begin(); it != llamas->end(); ++it)
        {
            boost::shared_ptr<Llama> candidate = boost::static_pointer_cast<Llama>(*it);
            if (!candidate->isLeashed())
            {
                continue;
            }

            if (candidate->hasCaravanTail())
            {
                continue;
            }

            double distance = mLlama->distanceToSqr(candidate);
            if (distance > closestDistance)
            {
                continue;
            }

            closestDistance = distance;
            closest = candidate;
        }
    }

    delete llamas;

    if (!closest)
    {
        return false;
    }

    if (closestDistance < 4.0)
    {
        return false;
    }

    boost::shared_ptr<Entity> leashHolder;
    int distance = 1;
    if (!closest->isLeashed() && !firstIsLeashed(closest.get(), distance, leashHolder))
    {
        return false;
    }

    mLlama->joinCaravan(closest);
    ++distance;

    if (leashHolder && eTYPE_DERIVED_FROM(EntityType::PLAYER, leashHolder->GetType()) && distance >= 5)
    {
        boost::static_pointer_cast<Player>(leashHolder)
            ->awardStat(GenericStats::soIGotThatGoingForMe(), GenericStats::param_soIGotThatGoingForMe(distance));
    }

    return true;
}

bool LlamaFollowCaravanGoal::canContinueToUse()
{
    if (mLlama == NULL)
    {
        return false;
    }

    boost::shared_ptr<Entity> leashHolder;
    int distance = 0;
    if (!mLlama->inCaravan() || !mLlama->getCaravanHead()->isAlive() || !firstIsLeashed(mLlama, distance, leashHolder)
        || mLlama->isLeashed())
    {
        return false;
    }

    double distanceToHead = mLlama->distanceToSqr(mLlama->getCaravanHead());
    if (distanceToHead > 676.0)
    {
        if (mSpeedModifier <= 3.0)
        {
            mSpeedModifier *= 1.2;
            mDistanceToStop = 40;
            return true;
        }

        if (mDistanceToStop == 0)
        {
            return false;
        }
    }

    if (mDistanceToStop > 0)
    {
        --mDistanceToStop;
    }

    return true;
}

void LlamaFollowCaravanGoal::stop()
{
    if (mLlama != NULL)
    {
        mLlama->leaveCaravan();
    }

    mSpeedModifier = 2.1;
}

void LlamaFollowCaravanGoal::tick()
{
    if (mLlama == NULL || !mLlama->inCaravan())
    {
        return;
    }

    boost::shared_ptr<Llama> caravanHead = mLlama->getCaravanHead();
    double distance = mLlama->distanceTo(caravanHead);
    float stopDistance = 2.0f;
    Vec3* direction = Vec3::newTemp(caravanHead->mX - mLlama->mX, caravanHead->mY - mLlama->mY,
                                    caravanHead->mZ - mLlama->mZ)
                          ->normalize()
                          ->scale(std::max(distance - stopDistance, 0.0));
    mLlama->getNavigation()->moveTo(mLlama->mX + direction->x, mLlama->mY + direction->y, mLlama->mZ + direction->z,
                                    mSpeedModifier);
}

std::wstring LlamaFollowCaravanGoal::getName()
{
    return std::wstring(L"Follow Caravan");
}
