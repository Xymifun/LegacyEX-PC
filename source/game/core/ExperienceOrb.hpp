#ifndef GAME_CORE_EXPERIENCEORB_HPP
#define GAME_CORE_EXPERIENCEORB_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DamageSource;
class Level;

class ExperienceOrb : public Entity
{
public:
    ExperienceOrb(Level* level, double x, double y, double z, int value);
    ExperienceOrb(Level* level);
    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual bool makeStepSound();
    virtual void burn(int damage);
    virtual bool updateInWaterState();
    virtual int getLightColor();
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool shouldRender(double distanceSqr);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool isAttackable();

    static int getExperienceValue(int value);
    void _init();
    int xpToDurability(int value);
    int durabilityToXp(int value);
    int getValue();
    unsigned char getIcon();
    static Entity* create(Level* level);

    int mTickCount;

private:
    int mAge;
    int mPickupDelay;
    int mHealth;
    int mValue;
    boost::shared_ptr<Player> mFollowingPlayer;
    int mFollowingTime;
};

#endif
