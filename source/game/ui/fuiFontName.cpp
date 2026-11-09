#include "game/ui/fuiFontName.hpp"

void Swap4Bytes(int& value);

void fuiFontName::EndianSwap()
{
    Swap4Bytes(fontId);
}

void fuiFontName::EndianSwap(void* pData)
{
    static_cast<fuiFontName*>(pData)->EndianSwap();
}
