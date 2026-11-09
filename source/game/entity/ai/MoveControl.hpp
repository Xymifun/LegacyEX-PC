#ifndef GAME_ENTITY_AI_MOVECONTROL_HPP
#define GAME_ENTITY_AI_MOVECONTROL_HPP

#include "game/entity/ai/Control.hpp"

class Mob;

class MoveControl : public Control
{
public:
    enum Operation
    {
        WAIT,
        MOVE_TO,
        STRAFE,
        JUMPING
    };

    explicit MoveControl(Mob* mob);
    virtual ~MoveControl() {}

    void copyFrom(MoveControl* other);
    void wait();
    bool hasWanted();
    double getSpeedModifier();
    void setSpeedModifier(double speedModifier);
    void strafe(float forwards, float right);
    float rotlerp(float from, float to, float max);
    virtual void setWantedPosition(double x, double y, double z, double speedModifier);
    virtual void tick();
    double getWantedX();
    double getWantedY();
    double getWantedZ();
    Operation getOperation() const;
    void setOperation(Operation operation);

    static float MIN_SPEED_SQR;

protected:
    Mob* mMob;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
    float mStrafeForwards;
    float mStrafeRight;
    Operation mOperation;
};

#endif
