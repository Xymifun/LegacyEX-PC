#ifndef GAME_CORE_FRUSTUM_HPP
#define GAME_CORE_FRUSTUM_HPP

#include "game/core/FrustumData.hpp"

class FloatBuffer;

class Frustum : public FrustumData
{
public:
    Frustum();
    ~Frustum();

    static FrustumData* getFrustum();

private:
    void normalizePlane(float* plane);
    void calculateFrustum();

    FloatBuffer* _proj;
    FloatBuffer* _modl;
    FloatBuffer* _clip;
};

#endif
