#include "game/ui/fuiShape.hpp"

void Swap4Bytes(unsigned int& value);

void fuiShape::EndianSwap()
{
    Swap4Bytes(shapeComponentIndex);
    Swap4Bytes(shapeComponentCount);
    bounds.EndianSwap();
    fuiObject::EndianSwap();
}

void fuiShape::EndianSwap(void* pData)
{
    static_cast<fuiShape*>(pData)->EndianSwap();
}
