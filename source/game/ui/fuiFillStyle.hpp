#ifndef GAME_UI_FUIFILLSTYLE_HPP
#define GAME_UI_FUIFILLSTYLE_HPP

#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiRGBA.hpp"

enum eFuiFillType
{
    eFuiFillTypeSolid = 0x00,
    eFuiFillTypeLinearGradient = 0x10,
    eFuiFillTypeRadialGradient = 0x12,
    eFuiFillTypeFocalRadialGradient = 0x13,
    eFuiFillTypeRepeatingBitmap = 0x40,
    eFuiFillTypeClippedBitmap = 0x41,
    eFuiFillTypeNonSmoothedRepeatingBitmap = 0x42,
    eFuiFillTypeNonSmoothedClippedBitmap = 0x43
};

struct fuiFillStyle
{
    eFuiFillType fillType;
    fuiRGBA color;
    unsigned int bitmapId;
    fuiMatrix bitmapMatrix;

    void EndianSwap();
};

#endif
