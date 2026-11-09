#ifndef GAME_CORE_LIGHTING_HPP
#define GAME_CORE_LIGHTING_HPP

#include "game/core/FloatBuffer.hpp"

class Lighting
{
public:
    static void turnOn();
    static void turnOff();
    static void turnOnGui();

private:
    static FloatBuffer* getBuffer(float x, float y, float z, float w);
    static FloatBuffer* getBuffer(double x, double y, double z, double w);
};

#endif
