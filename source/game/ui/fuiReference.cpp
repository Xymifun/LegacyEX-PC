#include "game/ui/fuiReference.hpp"

void Swap4Bytes(unsigned int& value);

void fuiReference::EndianSwap()
{
    Swap4Bytes(index);
    fuiObject::EndianSwap();
}

void fuiReference::EndianSwap(void* pData)
{
    static_cast<fuiReference*>(pData)->EndianSwap();
}
