#include "game/core/FrustumData.hpp"

#include "game/util/AABB.hpp"

static float** s_frustum;

FrustumData::FrustumData()
{
    frustum = new float*[6];

    for (int i = 0; i < 6; i++)
    {
        frustum[i] = new float[4];
    }

    proj = arrayWithLength<float>(16, true);
    modl = arrayWithLength<float>(16, true);
    clip = arrayWithLength<float>(16, true);
}

FrustumData::~FrustumData()
{
    delete[] proj.data;
    delete[] modl.data;
    delete[] clip.data;

    for (int i = 0; i < 6; i++)
    {
        delete[] frustum[i];
    }

    delete[] s_frustum;
}

double FrustumData::discriminant(float* plane, double x, double y, double z)
{
    return (double)plane[0] * x + (double)plane[1] * y + (double)plane[2] * z + (double)plane[3];
}

bool FrustumData::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    for (int i = 0; i < 6; i++)
    {
        float* plane = frustum[i];

        if (discriminant(plane, x0, y0, z0) <= 0.0 || discriminant(plane, x1, y0, z0) <= 0.0
            || discriminant(plane, x0, y1, z0) <= 0.0 || discriminant(plane, x1, y1, z0) <= 0.0
            || discriminant(plane, x0, y0, z1) <= 0.0 || discriminant(plane, x1, y0, z1) <= 0.0
            || discriminant(plane, x0, y1, z1) <= 0.0 || discriminant(plane, x1, y1, z1) <= 0.0)
        {
            return false;
        }
    }

    return true;
}

bool FrustumData::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    for (int i = 0; i < 6; i++)
    {
        float* plane = frustum[i];

        if (discriminant(plane, x0, y0, z0) <= 0.0 && discriminant(plane, x1, y0, z0) <= 0.0
            && discriminant(plane, x0, y1, z0) <= 0.0 && discriminant(plane, x1, y1, z0) <= 0.0
            && discriminant(plane, x0, y0, z1) <= 0.0 && discriminant(plane, x1, y0, z1) <= 0.0
            && discriminant(plane, x0, y1, z1) <= 0.0 && discriminant(plane, x1, y1, z1) <= 0.0)
        {
            return false;
        }
    }

    return true;
}

bool FrustumData::pointInFrustum(float x, float y, float z)
{
    for (int i = 0; i < 6; i++)
    {
        if (discriminant(frustum[i], x, y, z) <= 0.0)
        {
            return false;
        }
    }
    return true;
}

bool FrustumData::sphereInFrustum(float x, float y, float z, float radius)
{
    for (int i = 0; i < 6; i++)
    {
        if (discriminant(frustum[i], x, y, z) <= (double)-radius)
        {
            return false;
        }
    }
    return true;
}

bool FrustumData::isVisible(AABB* boundingBox)
{
    return cubeInFrustum(boundingBox->mMinX, boundingBox->mMinY, boundingBox->mMinZ, boundingBox->mMaxX,
                         boundingBox->mMaxY, boundingBox->mMaxZ);
}
