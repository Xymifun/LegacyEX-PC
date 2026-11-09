#ifndef GAME_ENTITY_MOB_ENTITYPOS_HPP
#define GAME_ENTITY_MOB_ENTITYPOS_HPP

#include <boost/shared_ptr.hpp>

class Entity;

class EntityPos
{
public:
    EntityPos(double x, double y, double z, float yRot, float xRot);
    EntityPos(double x, double y, double z);
    EntityPos(float yRot, float xRot);
    EntityPos lerp(boost::shared_ptr<Entity> entity, float partialTicks);

    double mX;
    double mY;
    double mZ;
    float mYRot;
    float mXRot;
    bool mHasRot;
    bool mHasPos;
};

#endif
