#ifndef GAME_UI_FUISHAPE_HPP
#define GAME_UI_FUISHAPE_HPP

#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRect.hpp"

struct fuiShape : public fuiObject
{
    unsigned int shapeComponentIndex;
    unsigned int shapeComponentCount;
    fuiRect bounds;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
