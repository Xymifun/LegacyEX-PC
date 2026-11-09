#include "game/ui/fuiColorTransform.hpp"

void Swap4Bytes(float& value);

void fuiColorTransform::EndianSwap()
{
    Swap4Bytes(redMultiply);
    Swap4Bytes(greenMultiply);
    Swap4Bytes(blueMultiply);
    Swap4Bytes(alphaMultiply);
    Swap4Bytes(redAdd);
    Swap4Bytes(greenAdd);
    Swap4Bytes(blueAdd);
    Swap4Bytes(alphaAdd);
}
