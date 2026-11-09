#ifndef GAME_UI_FUIFILEHEADER_HPP
#define GAME_UI_FUIFILEHEADER_HPP

#include "game/ui/fuiRect.hpp"

struct fuiFileHeader
{
    unsigned int signature;
    unsigned int fileLength;
    unsigned int dataLength;
    char name[64];
    unsigned int timelineCount;
    unsigned int timelineEventNameCount;
    unsigned int timelineActionCount;
    unsigned int shapeCount;
    unsigned int shapeComponentCount;
    unsigned int vertCount;
    unsigned int timelineFrameCount;
    unsigned int timelineEventCount;
    unsigned int referenceCount;
    unsigned int edittextCount;
    unsigned int symbolCount;
    unsigned int bitmapCount;
    unsigned int fontCount;
    unsigned int fontNameCount;
    unsigned int importAssetCount;
    fuiRect stageRect;

    void EndianSwap();
};

#endif
