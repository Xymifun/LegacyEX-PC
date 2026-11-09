#include "game/core/Uniform.hpp"

Uniform::Uniform(const std::wstring& name, int type, int count, Effect* effect) : mName(name)
{
    mCount = count;
    mType = type;
    mEffect = effect;
    if (type <= 3)
    {
        mIntBuffer = new IntBuffer(count);
        mFloatBuffer = NULL;
    }
    else
    {
        mIntBuffer = NULL;
        mFloatBuffer = new FloatBuffer(count);
    }
    mLocation = -1;
    markDirty();
}

void Uniform::markDirty()
{
    mDirty = true;
    if (mEffect)
    {
        mEffect->markDirty();
    }
}

void Uniform::set(float a)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    markDirty();
}

void Uniform::set(float a, float b)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    markDirty();
}

void Uniform::set(float a, float b, float c)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    markDirty();
}

void Uniform::set(const Vector3f& v)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, v.x);
    mFloatBuffer->put(1, v.y);
    mFloatBuffer->put(2, v.z);
    markDirty();
}

void Uniform::set(float a, float b, float c, float d)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(a);
    mFloatBuffer->put(b);
    mFloatBuffer->put(c);
    mFloatBuffer->put(d);
    mFloatBuffer->flip();
    markDirty();
}

void Uniform::setSafe(float a, float b, float c, float d)
{
    mFloatBuffer->position(0);
    if (mType >= 4)
    {
        mFloatBuffer->put(0, a);
    }
    if (mType >= 5)
    {
        mFloatBuffer->put(1, b);
    }
    if (mType >= 6)
    {
        mFloatBuffer->put(2, c);
    }
    if (mType >= 7)
    {
        mFloatBuffer->put(3, d);
    }
    markDirty();
}

void Uniform::setSafe(int a, int b, int c, int d)
{
    mIntBuffer->position(0);
    if (mType >= 0)
    {
        mIntBuffer->put(0, a);
    }
    if (mType > 0)
    {
        mIntBuffer->put(1, b);
    }
    if (mType >= 2)
    {
        mIntBuffer->put(2, c);
    }
    if (mType >= 3)
    {
        mIntBuffer->put(3, d);
    }
    markDirty();
}

void Uniform::set(int a)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, a);
    markDirty();
}

void Uniform::set(int a, int b)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, a);
    mIntBuffer->put(1, b);
    markDirty();
}

void Uniform::set(int a, int b, int c)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, a);
    mIntBuffer->put(1, b);
    mIntBuffer->put(2, c);
    markDirty();
}

void Uniform::set(int a, int b, int c, int d)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, a);
    mIntBuffer->put(1, b);
    mIntBuffer->put(2, c);
    mIntBuffer->put(3, d);
    markDirty();
}

void Uniform::set(arrayWithLength<float> values)
{
    if (values.length >= mCount)
    {
        mFloatBuffer->position(0);
        mFloatBuffer->put(values);
        mFloatBuffer->position(0);
        markDirty();
    }
}

void Uniform::setMat2x2(float a, float b, float c, float d)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    markDirty();
}

void Uniform::setMat2x3(float a, float b, float c, float d, float e, float f)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    markDirty();
}

void Uniform::setMat2x4(float a, float b, float c, float d, float e, float f, float g, float h)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    markDirty();
}

void Uniform::setMat3x2(float a, float b, float c, float d, float e, float f)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    markDirty();
}

void Uniform::setMat3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    mFloatBuffer->put(8, i);
    markDirty();
}

void Uniform::setMat3x4(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j,
                        float k, float l)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    mFloatBuffer->put(8, i);
    mFloatBuffer->put(9, j);
    mFloatBuffer->put(10, k);
    mFloatBuffer->put(11, l);
    markDirty();
}

void Uniform::setMat4x2(float a, float b, float c, float d, float e, float f, float g, float h)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    markDirty();
}

void Uniform::setMat4x3(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j,
                        float k, float l)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    mFloatBuffer->put(8, i);
    mFloatBuffer->put(9, j);
    mFloatBuffer->put(10, k);
    mFloatBuffer->put(11, l);
    markDirty();
}

void Uniform::setMat4x4(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j,
                        float k, float l, float m, float n, float o, float p)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, a);
    mFloatBuffer->put(1, b);
    mFloatBuffer->put(2, c);
    mFloatBuffer->put(3, d);
    mFloatBuffer->put(4, e);
    mFloatBuffer->put(5, f);
    mFloatBuffer->put(6, g);
    mFloatBuffer->put(7, h);
    mFloatBuffer->put(8, i);
    mFloatBuffer->put(9, j);
    mFloatBuffer->put(10, k);
    mFloatBuffer->put(11, l);
    mFloatBuffer->put(12, m);
    mFloatBuffer->put(13, n);
    mFloatBuffer->put(14, o);
    mFloatBuffer->put(15, p);
    markDirty();
}

void Uniform::set(Matrix4f* matrix)
{
    setMat4x4(matrix->m[0], matrix->m[1], matrix->m[2], matrix->m[3], matrix->m[4], matrix->m[5], matrix->m[6],
              matrix->m[7], matrix->m[8], matrix->m[9], matrix->m[10], matrix->m[11], matrix->m[12], matrix->m[13],
              matrix->m[14], matrix->m[15]);
}
