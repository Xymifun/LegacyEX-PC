#ifndef GAME_CORE_MATRIX4_HPP
#define GAME_CORE_MATRIX4_HPP

template <class T>
class Matrix4
{
public:
    T m[16];

    Matrix4();

    void setIdentity();
};

template <class T>
Matrix4<T>::Matrix4()
{
    m[0] = m[1] = m[2] = m[3] = 0;
    m[4] = m[5] = m[6] = m[7] = 0;
    m[8] = m[9] = m[10] = m[11] = 0;
    m[12] = m[13] = m[14] = m[15] = 0;
}

template <class T>
void Matrix4<T>::setIdentity()
{
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = 1;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = 1;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

#endif
