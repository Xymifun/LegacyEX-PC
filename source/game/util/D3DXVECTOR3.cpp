#include "game/util/D3DXVECTOR3.hpp"

D3DXVECTOR3::D3DXVECTOR3()
{
}

D3DXVECTOR3::D3DXVECTOR3(float x, float y, float z) : x(x), y(y), z(z)
{
}

D3DXVECTOR3& D3DXVECTOR3::operator+=(const D3DXVECTOR3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}
