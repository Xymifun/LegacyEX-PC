#ifndef GAME_ENTITY_AI_OCELOTATTACKGOAL_HPP
#define GAME_ENTITY_AI_OCELOTATTACKGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class LivingEntity;
class Level;
class Mob;

class OcelotAttackGoal : public Goal
{
public:
    OcelotAttackGoal(Mob* mob);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"OcelotAttack"; }

protected:
    Level* mLevel;
    Mob* mMob;
    boost::weak_ptr<LivingEntity> mTarget;
    int mAttackTime;
    float mValue;
    bool mFlag;
};

#endif
