#ifndef GAME_CORE_FRUSTUMDATA_HPP
#define GAME_CORE_FRUSTUMDATA_HPP

#include "game/world/biome/arrayWithLength.hpp"

class AABB;

class FrustumData
{
public:
    FrustumData();
    ~FrustumData();
    double discriminant(float* plane, double x, double y, double z);
    bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    bool pointInFrustum(float x, float y, float z);
    bool sphereInFrustum(float x, float y, float z, float radius);
    bool isVisible(AABB* boundingBox);
    float** frustum;
    arrayWithLength<float> proj;
    arrayWithLength<float> modl;
    arrayWithLength<float> clip;
};

#endif
