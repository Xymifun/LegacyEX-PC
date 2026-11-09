#ifndef GAME_ENTITY_AI_LOOKCONTROL_HPP
#define GAME_ENTITY_AI_LOOKCONTROL_HPP

#include "game/entity/ai/Control.hpp"

#include <boost/shared_ptr.hpp>

class Entity;
class Mob;
class Vec3;

class LookControl : public Control
{
public:
    explicit LookControl(Mob* mob);
    virtual ~LookControl() {}
    virtual void tick();

    void setLookAt(boost::shared_ptr<Entity> entity, float yMaxRotationIncrease, float xMaxRotationIncrease);
    void setLookAt(double x, double y, double z, float yMaxRotationIncrease, float xMaxRotationIncrease);
    void setLookRotation(const Vec3* rotation, float yMaxRotationIncrease, float xMaxRotationIncrease);
    float rotlerp(float from, float to, float maxDelta);
    bool isHasWanted();
    double getWantedX();
    double getWantedY();
    double getWantedZ();
    float getYMax();
    float getXMax();

protected:
    Mob* mMob;
    float mYMax;
    float mXMax;
    bool mHasWanted;
    bool mHasWantedRotation;
    Vec3* mWantedRotation;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
};

#endif
