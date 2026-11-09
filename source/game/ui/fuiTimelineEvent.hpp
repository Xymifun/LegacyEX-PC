#ifndef GAME_UI_FUITIMELINEEVENT_HPP
#define GAME_UI_FUITIMELINEEVENT_HPP

#include "game/ui/fuiColorTransform.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiTimelineEvent_version0.hpp"

struct fuiTimelineEvent
{
    unsigned char flags;
    unsigned char renderFlags;
    unsigned char objectType;
    unsigned char blendMode;
    unsigned short depth;
    unsigned short objectIndex;
    unsigned short clipDepth;
    short nameIndex;
    fuiMatrix matrix;
    fuiColorTransform colorTransform;
    fuiRGBA color;

    void EndianSwap();
    static void EndianSwap(void* pData);
    void operator=(const fuiTimelineEvent_version0& other);
};

#endif
