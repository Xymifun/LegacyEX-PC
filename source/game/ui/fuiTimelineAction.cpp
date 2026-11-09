#include "game/ui/fuiTimelineAction.hpp"

void Swap2Bytes(short& value);

void fuiTimelineAction::EndianSwap()
{
    Swap2Bytes(frameIndex);
}

void fuiTimelineAction::EndianSwap(void* pData)
{
    static_cast<fuiTimelineAction*>(pData)->EndianSwap();
}
