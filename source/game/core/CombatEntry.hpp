#ifndef GAME_CORE_COMBATENTRY_HPP
#define GAME_CORE_COMBATENTRY_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/CombatTracker.hpp"

class DamageSource;
class Entity;

class CombatEntry
{
public:
    CombatEntry(const DamageSource* source, int time, float healthBeforeDamage, float damage,
                CombatTracker::eLOCATION location, float fallDistance);
    ~CombatEntry();

    bool isCombatRelated();
    const DamageSource* getSource();
    int getTime();
    CombatTracker::eLOCATION getLocation();
    float getDamage();
    float getHealthBeforeDamage();
    float getHealthAfterDamage();
    float getFallDistance();
    std::wstring getAttackerName();
    boost::shared_ptr<Entity> getAttacker();

private:
    const DamageSource* mSource;
    int mTime;
    float mDamage;
    float mHealthBeforeDamage;
    CombatTracker::eLOCATION mLocation;
    float mFallDistance;
};

#endif
