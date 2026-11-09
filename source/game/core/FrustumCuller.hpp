#ifndef GAME_CORE_FRUSTUMCULLER_HPP
#define GAME_CORE_FRUSTUMCULLER_HPP

#include "game/core/Culler.hpp"

class AABB;
class FrustumData;

class FrustumCuller : public Culler
{
public:
    FrustumCuller(FrustumData* frustumData);
    virtual ~FrustumCuller() {}

    virtual bool isVisible(AABB* boundingBox);
    virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual void prepare(double x, double y, double z);

    FrustumData* frustumData;
    double xOffset;
    double yOffset;
    double zOffset;
};

#endif
