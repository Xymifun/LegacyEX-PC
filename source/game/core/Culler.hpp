#ifndef GAME_CORE_CULLER_HPP
#define GAME_CORE_CULLER_HPP

class AABB;

class Culler
{
public:
    virtual ~Culler() {}

    virtual bool isVisible(AABB* boundingBox) = 0;
    virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1) = 0;
    virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1) = 0;
    virtual void prepare(double x, double y, double z) = 0;
};

#endif
