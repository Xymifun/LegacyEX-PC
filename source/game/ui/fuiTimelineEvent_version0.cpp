#include "game/ui/fuiTimelineEvent_version0.hpp"

void Swap2Bytes(unsigned short& value);
void Swap2Bytes(short& value);

void fuiTimelineEvent_version0::EndianSwap()
{
    Swap2Bytes(objectType);
    Swap2Bytes(depth);
    Swap2Bytes(objectIndex);
    Swap2Bytes(clipDepth);
    Swap2Bytes(nameIndex);
    matrix.EndianSwap();
    colorTransform.EndianSwap();
    color.EndianSwap();
}

void fuiTimelineEvent_version0::EndianSwap(void* pData)
{
    static_cast<fuiTimelineEvent_version0*>(pData)->EndianSwap();
}
