#include "game/ui/fuiVert.hpp"

void Swap4Bytes(float& value);

void fuiVert::EndianSwap()
{
    Swap4Bytes(x);
    Swap4Bytes(y);
}

void fuiVert::EndianSwap(void* pData)
{
    static_cast<fuiVert*>(pData)->EndianSwap();
}
