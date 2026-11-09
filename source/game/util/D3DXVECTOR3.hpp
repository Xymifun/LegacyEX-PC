#ifndef GAME_UTIL_D3DXVECTOR3_HPP
#define GAME_UTIL_D3DXVECTOR3_HPP

class D3DXVECTOR3
{
public:
    float x;
    float y;
    float z;
    float w;

    D3DXVECTOR3();
    D3DXVECTOR3(float x, float y, float z);

    D3DXVECTOR3& operator+=(const D3DXVECTOR3& other);
};

#endif
