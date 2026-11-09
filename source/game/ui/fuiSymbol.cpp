#include "game/ui/fuiSymbol.hpp"

void Swap4Bytes(eFuiObjectType& value);
void Swap4Bytes(unsigned int& value);

void fuiSymbol::EndianSwap()
{
    Swap4Bytes(objectType);
    Swap4Bytes(objectIndex);
}

void fuiSymbol::EndianSwap(void* pData)
{
    static_cast<fuiSymbol*>(pData)->EndianSwap();
}
