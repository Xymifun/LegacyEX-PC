#ifndef GAME_UI_FUIREFERENCE_HPP
#define GAME_UI_FUIREFERENCE_HPP

#include "game/ui/fuiObject.hpp"

struct fuiReference : public fuiObject
{
    char name[64];
    unsigned int index;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
