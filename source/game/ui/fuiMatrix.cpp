#include "game/ui/fuiMatrix.hpp"

void Swap4Bytes(float& value);

void fuiMatrix::mul(const fuiMatrix& m1, const fuiMatrix& m2, fuiMatrix& result)
{
    result.scaleX = m1.scaleX * m2.scaleX + m1.rotateSkew0 * m2.rotateSkew1 + 0.0f * m2.translateX;
    result.rotateSkew0 = m1.scaleX * m2.rotateSkew0 + m1.rotateSkew0 * m2.scaleY + 0.0f * m2.translateY;
    result.rotateSkew1 = m1.rotateSkew1 * m2.scaleX + m1.scaleY * m2.rotateSkew1 + 0.0f * m2.translateX;
    result.scaleY = m1.rotateSkew1 * m2.rotateSkew0 + m1.scaleY * m2.scaleY + 0.0f * m2.translateY;
    result.translateX = m1.translateX * m2.scaleX + m1.translateY * m2.rotateSkew1 + 1.0f * m2.translateX;
    result.translateY = m1.translateX * m2.rotateSkew0 + m1.translateY * m2.scaleY + 1.0f * m2.translateY;
}

void fuiMatrix::EndianSwap()
{
    Swap4Bytes(scaleX);
    Swap4Bytes(rotateSkew0);
    Swap4Bytes(rotateSkew1);
    Swap4Bytes(scaleY);
    Swap4Bytes(translateX);
    Swap4Bytes(translateY);
}
