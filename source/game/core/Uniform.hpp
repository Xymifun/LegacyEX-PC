#ifndef GAME_CORE_UNIFORM_HPP
#define GAME_CORE_UNIFORM_HPP
#include <stdint.h>
#include <string>
#include "game/core/FloatBuffer.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/Matrix4f.hpp"
#include "game/core/Vector3f.hpp"
#include "game/item/Effect.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class Uniform
{
public:
    Uniform(const std::wstring&, int, int, Effect*);
    virtual ~Uniform() {}
    virtual void set(float);
    virtual void set(float, float);
    virtual void set(float, float, float);
    virtual void set(const Vector3f&);
    virtual void set(float, float, float, float);
    virtual void set(int);
    virtual void set(int, int);
    virtual void set(int, int, int);
    virtual void set(int, int, int, int);
    virtual void set(arrayWithLength<float>);
    virtual void set(Matrix4f*);
    virtual void setSafe(float, float, float, float);
    virtual void setSafe(int, int, int, int);
    virtual void setMat2x2(float, float, float, float);
    virtual void setMat2x3(float, float, float, float, float, float);
    virtual void setMat2x4(float, float, float, float, float, float, float, float);
    virtual void setMat3x2(float, float, float, float, float, float);
    virtual void setMat3x3(float, float, float, float, float, float, float, float, float);
    virtual void setMat3x4(float, float, float, float, float, float, float, float, float, float, float, float);
    virtual void setMat4x2(float, float, float, float, float, float, float, float);
    virtual void setMat4x3(float, float, float, float, float, float, float, float, float, float, float, float);
    virtual void setMat4x4(float, float, float, float, float, float, float, float, float, float, float, float, float,
                           float, float, float);
    static int getTypeFromString(const std::wstring&);
    void setLocation(int);
    std::wstring getName();
    void set(int, float);
    void uploadAsInteger();
    void uploadAsFloat();
    void uploadAsMatrix();
    void upload();
    int getLocation();
    int getCount();
    int getType();
    IntBuffer* getIntBuffer();
    FloatBuffer* getFloatBuffer();
    void markDirty();

private:
    int mLocation;
    int mCount;
    int mType;
    IntBuffer* mIntBuffer;
    FloatBuffer* mFloatBuffer;
    std::wstring mName;
    bool mDirty;
    Effect* mEffect;
};
#endif
