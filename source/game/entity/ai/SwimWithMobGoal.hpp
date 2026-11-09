#ifndef GAME_ENTITY_AI_SWIMWITHMOBGOAL_HPP
#define GAME_ENTITY_AI_SWIMWITHMOBGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPos;
class Entity;
class Mob;
class Vec3;

class SwimWithMobGoal : public Goal
{
public:
    SwimWithMobGoal(Mob* mob, float speedModifier, float maxDistance, int searchDistance,
                    arrayWithLength<unsigned long long> entityTypes);
    virtual ~SwimWithMobGoal();
    bool canSwimWith(boost::shared_ptr<Entity> entity);
    bool setWantedMob();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    BlockPos GetTargetPosition(boost::shared_ptr<Entity> entity, Vec3* direction, double distance, double offset);
    virtual void tick();
    virtual std::wstring getName() { return L"SwimWithMob"; }

private:
    boost::weak_ptr<Entity> mWantedMob;
    int mCooldown;
    int mState;
    arrayWithLength<unsigned long long> mEntityTypes;
    Mob* mMob;
    float mSpeedModifier;
    float mMaxDistance;
    int mSearchDistance;
};

#endif
