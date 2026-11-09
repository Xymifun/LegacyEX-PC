#include "game/ui/fuiBitmap.hpp"

void Swap4Bytes(unsigned int& value);

void Swap4Bytes(eFuiBitmapType& value)
{
    Swap4Bytes(reinterpret_cast<unsigned int&>(value));
}

void Swap4Bytes(int& value);

void fuiBitmap::EndianSwap()
{
    Swap4Bytes(type);
    Swap4Bytes(width);
    Swap4Bytes(height);
    Swap4Bytes(imageDataOffset);
    Swap4Bytes(imageDataSize);
    Swap4Bytes(jpegDataSize);
    Swap4Bytes(textureId);
    fuiObject::EndianSwap();
}

void fuiBitmap::EndianSwap(void* pData)
{
    static_cast<fuiBitmap*>(pData)->EndianSwap();
}
