#ifndef GAME_UI_FUIEDITTEXT_HPP
#define GAME_UI_FUIEDITTEXT_HPP

#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"

struct fuiEdittext : public fuiObject
{
    fuiRect bounds;
    int fontId;
    float fontScale;
    fuiRGBA textColor;
    unsigned char align;
    float leftMargin;
    float rightMargin;
    float indent;
    float leading;
    unsigned char wordWrap;
    unsigned char multiline;
    unsigned char password;
    unsigned char readOnly;
    char text[256];

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
