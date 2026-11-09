#ifndef GAME_UI_FUISHAPECOMPONENT_HPP
#define GAME_UI_FUISHAPECOMPONENT_HPP

#include "game/ui/fuiFillStyle.hpp"

struct fuiShapeComponent : public fuiFillStyle
{
    unsigned int vertexOffset;
    unsigned int vertexCount;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
