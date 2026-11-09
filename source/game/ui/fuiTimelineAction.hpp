#ifndef GAME_UI_FUITIMELINEACTION_HPP
#define GAME_UI_FUITIMELINEACTION_HPP

struct fuiTimelineAction
{
    unsigned char actionType;
    short frameIndex;
    char targetName[64];
    char actionValue[64];

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
