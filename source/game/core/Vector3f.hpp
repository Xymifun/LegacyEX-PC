#ifndef GAME_CORE_VECTOR3F_HPP
#define GAME_CORE_VECTOR3F_HPP

class Vector3f
{
public:
    float x;
    float y;
    float z;

    Vector3f() { x = y = z = 0.0f; }

    Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
};

#endif
