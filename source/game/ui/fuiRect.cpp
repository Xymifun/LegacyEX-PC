#include "game/ui/fuiRect.hpp"

void Swap4Bytes(float& value);

void fuiRect::EndianSwap()
{
    Swap4Bytes(xMin);
    Swap4Bytes(xMax);
    Swap4Bytes(yMin);
    Swap4Bytes(yMax);
}
