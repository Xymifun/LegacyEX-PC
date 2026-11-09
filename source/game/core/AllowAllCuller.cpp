#include "game/core/AllowAllCuller.hpp"

#include "game/util/AABB.hpp"

AllowAllCuller::~AllowAllCuller()
{
}

bool AllowAllCuller::isVisible(AABB* box)
{
    return true;
}

bool AllowAllCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return true;
}

bool AllowAllCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return true;
}

void AllowAllCuller::prepare(double x, double y, double z)
{
}
