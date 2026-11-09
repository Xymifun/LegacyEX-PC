#include "game/core/Frustum.hpp"

#include <cmath>

#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"

static Frustum* theFrustum = new Frustum();

Frustum::Frustum()
{
    _proj = MemoryTracker::createFloatBuffer(16);
    _modl = MemoryTracker::createFloatBuffer(16);
    _clip = MemoryTracker::createFloatBuffer(16);
}

Frustum::~Frustum()
{
    delete _proj;
    delete _modl;
    delete _clip;
}

void Frustum::normalizePlane(float* plane)
{
    float magnitude = std::sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);

    plane[0] /= magnitude;
    plane[1] /= magnitude;
    plane[2] /= magnitude;
    plane[3] /= magnitude;
}

void Frustum::calculateFrustum()
{
    _proj->clear();
    _modl->clear();
    _clip->clear();

    GlStateManager::getMatrix(1, _proj);
    GlStateManager::getMatrix(0, _modl);

    arrayWithLength<float> proj(theFrustum->proj.data, theFrustum->proj.length);
    arrayWithLength<float> modl = theFrustum->modl;

    _proj->flip()->limit(16);
    _proj->get(&proj);
    _modl->flip()->limit(16);
    _modl->get(&modl);

    clip.data[0] = modl.data[0] * proj.data[0] + modl.data[1] * proj.data[4] + modl.data[2] * proj.data[8]
                   + modl.data[3] * proj.data[12];
    clip.data[1] = modl.data[0] * proj.data[1] + modl.data[1] * proj.data[5] + modl.data[2] * proj.data[9]
                   + modl.data[3] * proj.data[13];
    clip.data[2] = modl.data[0] * proj.data[2] + modl.data[1] * proj.data[6] + modl.data[2] * proj.data[10]
                   + modl.data[3] * proj.data[14];
    clip.data[3] = modl.data[0] * proj.data[3] + modl.data[1] * proj.data[7] + modl.data[2] * proj.data[11]
                   + modl.data[3] * proj.data[15];
    clip.data[4] = modl.data[4] * proj.data[0] + modl.data[5] * proj.data[4] + modl.data[6] * proj.data[8]
                   + modl.data[7] * proj.data[12];
    clip.data[5] = modl.data[4] * proj.data[1] + modl.data[5] * proj.data[5] + modl.data[6] * proj.data[9]
                   + modl.data[7] * proj.data[13];
    clip.data[6] = modl.data[4] * proj.data[2] + modl.data[5] * proj.data[6] + modl.data[6] * proj.data[10]
                   + modl.data[7] * proj.data[14];
    clip.data[7] = modl.data[4] * proj.data[3] + modl.data[5] * proj.data[7] + modl.data[6] * proj.data[11]
                   + modl.data[7] * proj.data[15];
    clip.data[8] = modl.data[8] * proj.data[0] + modl.data[9] * proj.data[4] + modl.data[10] * proj.data[8]
                   + modl.data[11] * proj.data[12];
    clip.data[9] = modl.data[8] * proj.data[1] + modl.data[9] * proj.data[5] + modl.data[10] * proj.data[9]
                   + modl.data[11] * proj.data[13];
    clip.data[10] = modl.data[8] * proj.data[2] + modl.data[9] * proj.data[6] + modl.data[10] * proj.data[10]
                    + modl.data[11] * proj.data[14];
    clip.data[11] = modl.data[8] * proj.data[3] + modl.data[9] * proj.data[7] + modl.data[10] * proj.data[11]
                    + modl.data[11] * proj.data[15];
    clip.data[12] = modl.data[12] * proj.data[0] + modl.data[13] * proj.data[4] + modl.data[14] * proj.data[8]
                    + modl.data[15] * proj.data[12];
    clip.data[13] = modl.data[12] * proj.data[1] + modl.data[13] * proj.data[5] + modl.data[14] * proj.data[9]
                    + modl.data[15] * proj.data[13];
    clip.data[14] = modl.data[12] * proj.data[2] + modl.data[13] * proj.data[6] + modl.data[14] * proj.data[10]
                    + modl.data[15] * proj.data[14];
    clip.data[15] = modl.data[12] * proj.data[3] + modl.data[13] * proj.data[7] + modl.data[14] * proj.data[11]
                    + modl.data[15] * proj.data[15];

    frustum[0][0] = clip.data[3] - clip.data[0];
    frustum[0][1] = clip.data[7] - clip.data[4];
    frustum[0][2] = clip.data[11] - clip.data[8];
    frustum[0][3] = clip.data[15] - clip.data[12];
    normalizePlane(frustum[0]);

    frustum[1][0] = clip.data[3] + clip.data[0];
    frustum[1][1] = clip.data[7] + clip.data[4];
    frustum[1][2] = clip.data[11] + clip.data[8];
    frustum[1][3] = clip.data[15] + clip.data[12];
    normalizePlane(frustum[1]);

    frustum[2][0] = clip.data[3] + clip.data[1];
    frustum[2][1] = clip.data[7] + clip.data[5];
    frustum[2][2] = clip.data[11] + clip.data[9];
    frustum[2][3] = clip.data[15] + clip.data[13];
    normalizePlane(frustum[2]);

    frustum[3][0] = clip.data[3] - clip.data[1];
    frustum[3][1] = clip.data[7] - clip.data[5];
    frustum[3][2] = clip.data[11] - clip.data[9];
    frustum[3][3] = clip.data[15] - clip.data[13];
    normalizePlane(frustum[3]);

    frustum[4][0] = clip.data[3] - clip.data[2];
    frustum[4][1] = clip.data[7] - clip.data[6];
    frustum[4][2] = clip.data[11] - clip.data[10];
    frustum[4][3] = clip.data[15] - clip.data[14];
    normalizePlane(frustum[4]);

    frustum[5][0] = clip.data[3] + clip.data[2];
    frustum[5][1] = clip.data[7] + clip.data[6];
    frustum[5][2] = clip.data[11] + clip.data[10];
    frustum[5][3] = clip.data[15] + clip.data[14];
    normalizePlane(frustum[5]);
}

FrustumData* Frustum::getFrustum()
{
    theFrustum->calculateFrustum();
    return theFrustum;
}
