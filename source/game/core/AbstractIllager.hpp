#ifndef GAME_CORE_ABSTRACTILLAGER_HPP
#define GAME_CORE_ABSTRACTILLAGER_HPP

#include "game/entity/mob/Monster.hpp"

class Level;

enum EIllagerArmPose
{
    ILLAGER_ARM_POSE_CROSSED = 0,
    ILLAGER_ARM_POSE_ATTACKING = 1,
    ILLAGER_ARM_POSE_SPELLCASTING = 2,
    ILLAGER_ARM_POSE_BOW_AND_ARROW = 3
};

class AbstractIllager : public Monster
{
public:
    AbstractIllager(Level* level);
    virtual ~AbstractIllager() {}

    static void staticCtor();
    virtual void defineSynchedData();
    bool getIllagerFlag(int flag);
    void setIllagerFlag(int flag, bool value);
    virtual EMobType getMobType();
    virtual int getArmPose();

protected:
    static const EntityDataAccessor<unsigned char>* DATA_ILLAGER_FLAGS;
};

#endif
