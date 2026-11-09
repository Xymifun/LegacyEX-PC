#ifndef GAME_CLIENT_RENDER_VIEWPORTCULLER_HPP
#define GAME_CLIENT_RENDER_VIEWPORTCULLER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Culler.hpp"

class AABB;
class LivingEntity;

class ViewportCuller : public Culler
{
public:
    class Face
    {
    public:
        Face() {}
        Face(double x, double y, double z, float yaw, float pitch);

        bool inFront(double x, double y, double z, double radius);
        bool inFront(double x0, double y0, double z0, double x1, double y1, double z1);
        bool fullyInFront(double x0, double y0, double z0, double x1, double y1, double z1);

        double mX;
        double mY;
        double mZ;
        double mNormalX;
        double mNormalY;
        double mNormalZ;
        double mDistance;
    };

    ViewportCuller(boost::shared_ptr<LivingEntity> entity, double partialTicks, float fov);
    virtual ~ViewportCuller();

    virtual bool isVisible(AABB* boundingBox);
    virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual void prepare(double x, double y, double z);

    double _max(double x, double y, double z);

    Face mFaces[6];
    double mXOff;
    double mYOff;
    double mZOff;
};

#endif
