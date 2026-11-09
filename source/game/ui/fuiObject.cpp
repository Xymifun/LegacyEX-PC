#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiSymbol.hpp"

#include <cstring>

void Swap4Bytes(unsigned int& value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    unsigned int swapped = value;
    swapped = ((swapped << 24) | ((swapped & 0x0000ff00) << 8) | (swapped >> 24) | ((swapped >> 8) & 0x0000ff00));
    value = swapped;
#else
    (void)value;
#endif
}

void Swap4Bytes(eFuiObjectType& value)
{
    Swap4Bytes(reinterpret_cast<unsigned int&>(value));
}

void Swap2Bytes(unsigned short& value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    value = static_cast<unsigned short>((value << 8) | (value >> 8));
#else
    (void)value;
#endif
}

void Swap2Bytes(short& value)
{
    Swap2Bytes(reinterpret_cast<unsigned short&>(value));
}

void Swap4Bytes(int& value)
{
    Swap4Bytes(reinterpret_cast<unsigned int&>(value));
}

void Swap4Bytes(float& value)
{
    Swap4Bytes(reinterpret_cast<unsigned int&>(value));
}

void fuiObject::EndianSwap()
{
    Swap4Bytes(symbolIndex);
}
