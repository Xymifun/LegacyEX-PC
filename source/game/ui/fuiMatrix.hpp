#ifndef GAME_UI_FUIMATRIX_HPP
#define GAME_UI_FUIMATRIX_HPP

struct fuiMatrix
{
    float scaleX;
    float rotateSkew0;
    float rotateSkew1;
    float scaleY;
    float translateX;
    float translateY;

    static void mul(const fuiMatrix& m1, const fuiMatrix& m2, fuiMatrix& result);
    void EndianSwap();
};

#endif
