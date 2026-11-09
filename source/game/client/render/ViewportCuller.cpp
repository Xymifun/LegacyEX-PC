#include "game/client/render/ViewportCuller.hpp"

#include "game/util/AABB.hpp"

#include <cmath>

ViewportCuller::Face::Face(double x, double y, double z, float yaw, float pitch)
    : mX(x),
      mY(y),
      mZ(z),
      mNormalX(0.0),
      mNormalY(0.0),
      mNormalZ(0.0),
      mDistance(0.0)
{
    double yawRadians = (double)yaw * 0.017453292519943295;
    double pitchRadians = (double)pitch * 0.017453292519943295;
    mNormalX = std::cos(yawRadians) * std::cos(pitchRadians);
    mNormalY = std::sin(pitchRadians);
    mNormalZ = std::sin(yawRadians) * std::cos(pitchRadians);
    mDistance = mNormalX * mX + mNormalY * mY + mNormalZ * mZ;
}

bool ViewportCuller::Face::inFront(double x, double y, double z, double radius)
{
    return mNormalX * x + mNormalY * y + mNormalZ * z > mDistance - radius;
}

bool ViewportCuller::Face::inFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
    double nx0 = mNormalX * x0;
    double ny0 = mNormalY * y0;
    double nz0 = mNormalZ * z0;

    if (nx0 + ny0 + nz0 > mDistance)
    {
        return true;
    }

    double nx1 = mNormalX * x1;

    if (nx1 + ny0 + nz0 > mDistance)
    {
        return true;
    }

    double ny1 = mNormalY * y1;

    if (nx0 + ny1 + nz0 > mDistance)
    {
        return true;
    }

    if (nx1 + ny1 + nz0 > mDistance)
    {
        return true;
    }

    double nz1 = mNormalZ * z1;

    if (nx0 + ny0 + nz1 > mDistance)
    {
        return true;
    }

    if (nx1 + ny0 + nz1 > mDistance)
    {
        return true;
    }

    if (nx0 + ny1 + nz1 > mDistance)
    {
        return true;
    }

    if (nx1 + ny1 + nz1 > mDistance)
    {
        return true;
    }

    return false;
}

bool ViewportCuller::Face::fullyInFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
    double nx0 = mNormalX * x0;
    double ny0 = mNormalY * y0;
    double nz0 = mNormalZ * z0;

    if (nx0 + ny0 + nz0 < mDistance)
    {
        return false;
    }

    double nx1 = mNormalX * x1;

    if (nx1 + ny0 + nz0 < mDistance)
    {
        return false;
    }

    double ny1 = mNormalY * y1;

    if (nx0 + ny1 + nz0 < mDistance)
    {
        return false;
    }

    if (nx1 + ny1 + nz0 < mDistance)
    {
        return false;
    }

    double nz1 = mNormalZ * z1;

    if (nx0 + ny0 + nz1 < mDistance)
    {
        return false;
    }

    if (nx1 + ny0 + nz1 < mDistance)
    {
        return false;
    }

    if (nx0 + ny1 + nz1 < mDistance)
    {
        return false;
    }

    if (nx1 + ny1 + nz1 < mDistance)
    {
        return false;
    }

    return true;
}

ViewportCuller::~ViewportCuller()
{
}

ViewportCuller::ViewportCuller(boost::shared_ptr<LivingEntity> entity, double partialTicks, float fov)
    : mXOff(0.0),
      mYOff(0.0),
      mZOff(0.0)
{
    double angle = (double)fov * 0.5;
    mFaces[0] = Face(0.0, 0.0, 0.0, (float)-angle, 0.0f);
    mFaces[1] = Face(0.0, 0.0, 0.0, (float)angle, 0.0f);
    mFaces[2] = Face(0.0, 0.0, 0.0, 0.0f, (float)-angle);
    mFaces[3] = Face(0.0, 0.0, 0.0, 0.0f, (float)angle);
    mFaces[4] = Face(0.0, 0.0, 0.0, -90.0f, 0.0f);
    mFaces[5] = Face(0.0, 0.0, 0.0, 90.0f, 0.0f);
}

bool ViewportCuller::isVisible(AABB* boundingBox)
{
    return cubeInFrustum(boundingBox->mMinX, boundingBox->mMinY, boundingBox->mMinZ, boundingBox->mMaxX,
                         boundingBox->mMaxY, boundingBox->mMaxZ);
}

double ViewportCuller::_max(double x, double y, double z)
{
    if (x > y)
    {
        return z < x ? z : x;
    }

    return y > z ? y : z;
}

bool ViewportCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= mXOff;
    y0 -= mYOff;
    z0 -= mZOff;
    x1 -= mXOff;
    y1 -= mYOff;
    z1 -= mZOff;

    double halfX = (x1 - x0) * 0.5;
    double halfY = (y1 - y0) * 0.5;
    double halfZ = (z1 - z0) * 0.5;

    double centerX = x0 + halfX;
    double centerY = y0 + halfY;
    double centerZ = z0 + halfZ;

    double radius = _max(halfX, halfY, halfZ) * 1.5;

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].inFront(centerX, centerY, centerZ, radius))
        {
            return false;
        }
    }

    if (!mFaces[0].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[1].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[2].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[3].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[4].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[5].inFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    return true;
}

bool ViewportCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= mXOff;
    y0 -= mYOff;
    z0 -= mZOff;
    x1 -= mXOff;
    y1 -= mYOff;
    z1 -= mZOff;

    double halfX = (x1 - x0) * 0.5;
    double halfY = (y1 - y0) * 0.5;
    double halfZ = (z1 - z0) * 0.5;

    double centerX = x0 + halfX;
    double centerY = y0 + halfY;
    double centerZ = z0 + halfZ;

    double radius = _max(halfX, halfY, halfZ) * 1.5;

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].inFront(centerX, centerY, centerZ, radius))
        {
            return false;
        }
    }

    if (!mFaces[0].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[1].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[2].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[3].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[4].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    if (!mFaces[5].fullyInFront(x0, y0, z0, x1, y1, z1))
    {
        return false;
    }

    return true;
}

void ViewportCuller::prepare(double x, double y, double z)
{
    mXOff = x;
    mYOff = y;
    mZOff = z;
}
