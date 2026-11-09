#ifndef GAME_UI_FUIBITMAP_HPP
#define GAME_UI_FUIBITMAP_HPP

#include "game/ui/fuiObject.hpp"

enum eFuiBitmapType
{
    eFuiBitmapType_None = 0,
    eFuiBitmapType_LosslessIndexed = 1,
    eFuiBitmapType_LosslessRGB15 = 2,
    eFuiBitmapType_LosslessRGB24 = 3,
    eFuiBitmapType_Lossless2Indexed = 4,
    eFuiBitmapType_Lossless2ARGB32 = 5,
    eFuiBitmapType_Jpeg = 6,
    eFuiBitmapType_Jpeg2 = 7,
    eFuiBitmapType_Jpeg3 = 8
};

struct fuiBitmap : public fuiObject
{
    eFuiBitmapType type;
    unsigned int width;
    unsigned int height;
    unsigned int imageDataOffset;
    unsigned int imageDataSize;
    unsigned int jpegDataSize;
    int textureId;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
