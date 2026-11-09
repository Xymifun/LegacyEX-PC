#include "game/ui/fuiTimelineFrame.hpp"

void Swap4Bytes(unsigned int& value);

void fuiTimelineFrame::EndianSwap()
{
    Swap4Bytes(timelineEventIndex);
    Swap4Bytes(timelineEventCount);
}

void fuiTimelineFrame::EndianSwap(void* pData)
{
    static_cast<fuiTimelineFrame*>(pData)->EndianSwap();
}
