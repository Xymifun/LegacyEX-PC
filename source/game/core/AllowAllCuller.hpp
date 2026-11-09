#ifndef GAME_CORE_ALLOWALLCULLER_HPP
#define GAME_CORE_ALLOWALLCULLER_HPP

#include "game/core/Culler.hpp"

class AABB;

class AllowAllCuller : public Culler
{
public:
    virtual ~AllowAllCuller();

    virtual bool isVisible(AABB* box);
    virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual void prepare(double x, double y, double z);
};

#endif
