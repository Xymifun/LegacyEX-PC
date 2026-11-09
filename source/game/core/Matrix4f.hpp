#ifndef GAME_CORE_MATRIX4F_HPP
#define GAME_CORE_MATRIX4F_HPP

#include "game/core/Matrix4.hpp"

class Matrix4f : public Matrix4<float>
{
public:
    Matrix4f();

    void setZYXRot(float z, float y, float x);
};

#endif
