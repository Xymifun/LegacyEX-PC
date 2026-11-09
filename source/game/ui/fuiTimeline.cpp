#include "game/ui/fuiTimeline.hpp"

void Swap2Bytes(unsigned short& value);

void fuiTimeline::EndianSwap()
{
    Swap2Bytes(timelineFrameIndex);
    Swap2Bytes(timelineFrameCount);
    Swap2Bytes(timelineActionIndex);
    Swap2Bytes(timelineActionCount);
    bounds.EndianSwap();
    fuiObject::EndianSwap();
}

void fuiTimeline::EndianSwap(void* pData)
{
    static_cast<fuiTimeline*>(pData)->EndianSwap();
}
