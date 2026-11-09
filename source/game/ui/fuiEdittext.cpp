#include "game/ui/fuiEdittext.hpp"

void Swap4Bytes(int& value);
void Swap4Bytes(float& value);

void fuiEdittext::EndianSwap()
{
    bounds.EndianSwap();
    Swap4Bytes(fontId);
    Swap4Bytes(fontScale);
    textColor.EndianSwap();
    Swap4Bytes(leftMargin);
    Swap4Bytes(rightMargin);
    Swap4Bytes(indent);
    Swap4Bytes(leading);
}

void fuiEdittext::EndianSwap(void* pData)
{
    static_cast<fuiEdittext*>(pData)->EndianSwap();
}
