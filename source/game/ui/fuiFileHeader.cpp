#include "game/ui/fuiFileHeader.hpp"

void Swap4Bytes(unsigned int& value);

void fuiFileHeader::EndianSwap()
{
    Swap4Bytes(signature);
    Swap4Bytes(fileLength);
    Swap4Bytes(dataLength);
    Swap4Bytes(timelineCount);
    Swap4Bytes(timelineEventNameCount);
    Swap4Bytes(timelineActionCount);
    Swap4Bytes(shapeCount);
    Swap4Bytes(shapeComponentCount);
    Swap4Bytes(vertCount);
    Swap4Bytes(timelineFrameCount);
    Swap4Bytes(timelineEventCount);
    Swap4Bytes(referenceCount);
    Swap4Bytes(edittextCount);
    Swap4Bytes(symbolCount);
    Swap4Bytes(bitmapCount);
    Swap4Bytes(fontCount);
    Swap4Bytes(fontNameCount);
    Swap4Bytes(importAssetCount);
    stageRect.EndianSwap();
}
