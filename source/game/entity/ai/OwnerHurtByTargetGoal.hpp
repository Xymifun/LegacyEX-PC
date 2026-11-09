#ifndef GAME_ENTITY_AI_OWNERHURTBYTARGETGOAL_HPP
#define GAME_ENTITY_AI_OWNERHURTBYTARGETGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/ai/TargetGoal.hpp"

class LivingEntity;
class TamableAnimal;

class OwnerHurtByTargetGoal : public TargetGoal
{
public:
    OwnerHurtByTargetGoal(TamableAnimal* tamableAnimal);

    virtual bool canUse();
    virtual void start();
    virtual std::wstring getName() { return L"OwnersHurtByTarget"; }

private:
    TamableAnimal* mTamableAnimal;
    boost::weak_ptr<LivingEntity> mOwnerLastHurtBy;
    int mTimestamp;
};

#endif
