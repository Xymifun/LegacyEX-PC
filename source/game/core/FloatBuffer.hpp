#ifndef GAME_CORE_FLOATBUFFER_HPP
#define GAME_CORE_FLOATBUFFER_HPP

#include "game/core/Buffer.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class FloatBuffer : public Buffer
{
    friend class Camera;
    friend class GlStateManager;

public:
    FloatBuffer(unsigned int capacity);
    FloatBuffer(unsigned int capacity, float* data);
    virtual ~FloatBuffer();
    virtual FloatBuffer* flip();
    FloatBuffer* put(float value);
    void put(int index, float value);
    FloatBuffer* put(arrayWithLength<float> values);
    void get(arrayWithLength<float>* values);
    float get(int index);

protected:
    float* mBuffer;
};

#endif
