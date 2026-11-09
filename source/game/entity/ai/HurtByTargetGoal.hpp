#ifndef GAME_ENTITY_AI_HURTBYTARGETGOAL_HPP
#define GAME_ENTITY_AI_HURTBYTARGETGOAL_HPP

#include "game/entity/ai/TargetGoal.hpp"
#include "game/world/biome/arrayWithLength.hpp"

#include <boost/shared_ptr.hpp>

class LivingEntity;
class PathfinderMob;

class HurtByTargetGoal : public TargetGoal
{
public:
    HurtByTargetGoal(PathfinderMob* mob, bool alertSameType, arrayWithLength<unsigned long long> entityTypes);
    virtual ~HurtByTargetGoal();

    virtual bool canUse();
    void alertOthers();
    virtual void start();
    virtual void alertOther(boost::shared_ptr<PathfinderMob> mob, boost::shared_ptr<LivingEntity> target);
    virtual std::wstring getName() { return L"HurtByTarget"; }

protected:
    bool mAlertSameType;
    int mTimestamp;
    arrayWithLength<unsigned long long> mEntityTypes;
};

#endif
