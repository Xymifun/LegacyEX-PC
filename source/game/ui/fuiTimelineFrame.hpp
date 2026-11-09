#ifndef GAME_UI_FUITIMELINEFRAME_HPP
#define GAME_UI_FUITIMELINEFRAME_HPP

struct fuiTimelineFrame
{
    char name[64];
    unsigned int timelineEventIndex;
    unsigned int timelineEventCount;

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
