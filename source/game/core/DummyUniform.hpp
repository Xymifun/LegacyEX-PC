#ifndef GAME_CORE_DUMMYUNIFORM_HPP
#define GAME_CORE_DUMMYUNIFORM_HPP

#include <string>

#include "game/core/Matrix4f.hpp"
#include "game/core/Uniform.hpp"
#include "game/core/Vector3f.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DummyUniform : public Uniform
{
public:
    DummyUniform() : Uniform(L"dummy", 4, 1, NULL) {}

    virtual ~DummyUniform() {}

    virtual void set(float x);
    virtual void set(float x, float y);
    virtual void set(float x, float y, float z);
    virtual void set(const Vector3f& value);
    virtual void set(float x, float y, float z, float w);
    virtual void set(int x);
    virtual void set(int x, int y);
    virtual void set(int x, int y, int z);
    virtual void set(int x, int y, int z, int w);
    virtual void set(arrayWithLength<float> components);
    virtual void set(Matrix4f* matrix);
    virtual void setSafe(float x, float y, float z, float w);
    virtual void setSafe(int x, int y, int z, int w);
    virtual void setMat2x2(float m00, float m01, float m10, float m11);
    virtual void setMat2x3(float m00, float m01, float m02, float m10, float m11, float m12);
    virtual void setMat2x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13);
    virtual void setMat3x2(float m00, float m01, float m10, float m11, float m20, float m21);
    virtual void setMat3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
                           float m22);
    virtual void setMat3x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
                           float m20, float m21, float m22, float m23);
    virtual void setMat4x2(float m00, float m01, float m10, float m11, float m20, float m21, float m30, float m31);
    virtual void setMat4x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
                           float m22, float m30, float m31, float m32);
    virtual void setMat4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
                           float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
};

#endif
