#ifndef GAME_UI_FUITIMELINE_HPP
#define GAME_UI_FUITIMELINE_HPP

#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRect.hpp"

struct fuiTimeline : public fuiObject
{
    unsigned short timelineFrameIndex;
    unsigned short timelineFrameCount;
    unsigned short timelineActionIndex;
    unsigned short timelineActionCount;
    fuiRect bounds;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
