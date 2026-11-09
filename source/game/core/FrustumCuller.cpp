#include "game/core/FrustumCuller.hpp"

#include "game/core/Frustum.hpp"
#include "game/core/FrustumData.hpp"
#include "game/util/AABB.hpp"

FrustumCuller::FrustumCuller(FrustumData* frustumData)
{
    this->frustumData = frustumData == NULL ? Frustum::getFrustum() : frustumData;
}

bool FrustumCuller::isVisible(AABB* boundingBox)
{
    return cubeInFrustum(boundingBox->mMinX, boundingBox->mMinY, boundingBox->mMinZ, boundingBox->mMaxX,
                         boundingBox->mMaxY, boundingBox->mMaxZ);
}

bool FrustumCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return frustumData->cubeInFrustum(x0 - xOffset, y0 - yOffset, z0 - zOffset, x1 - xOffset, y1 - yOffset,
                                      z1 - zOffset);
}

bool FrustumCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return frustumData->cubeFullyInFrustum(x0 - xOffset, y0 - yOffset, z0 - zOffset, x1 - xOffset, y1 - yOffset,
                                           z1 - zOffset);
}

void FrustumCuller::prepare(double x, double y, double z)
{
    xOffset = x;
    yOffset = y;
    zOffset = z;
}
