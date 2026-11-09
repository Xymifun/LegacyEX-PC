#ifndef GAME_UI_FUITIMELINEEVENT_VERSION0_HPP
#define GAME_UI_FUITIMELINEEVENT_VERSION0_HPP

#include "game/ui/fuiColorTransform.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiRGBA.hpp"

struct fuiTimelineEvent_version0
{
    unsigned char flags;
    unsigned char renderFlags;
    unsigned short objectType;
    unsigned short depth;
    unsigned short objectIndex;
    unsigned short clipDepth;
    short nameIndex;
    fuiMatrix matrix;
    fuiColorTransform colorTransform;
    fuiRGBA color;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
