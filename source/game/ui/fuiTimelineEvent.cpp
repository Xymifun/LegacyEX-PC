#include "game/ui/fuiTimelineEvent.hpp"

void Swap2Bytes(unsigned short& value);
void Swap2Bytes(short& value);

void fuiTimelineEvent::EndianSwap()
{
    Swap2Bytes(depth);
    Swap2Bytes(objectIndex);
    Swap2Bytes(clipDepth);
    Swap2Bytes(nameIndex);
    matrix.EndianSwap();
    colorTransform.EndianSwap();
    color.EndianSwap();
}

void fuiTimelineEvent::EndianSwap(void* pData)
{
    static_cast<fuiTimelineEvent*>(pData)->EndianSwap();
}

void fuiTimelineEvent::operator=(const fuiTimelineEvent_version0& other)
{
    flags = other.flags;
    renderFlags = other.renderFlags;
    objectType = (unsigned char)other.objectType;
    blendMode = 1;
    depth = other.depth;
    objectIndex = other.objectIndex;
    clipDepth = other.clipDepth;
    nameIndex = other.nameIndex;
    matrix = other.matrix;
    colorTransform = other.colorTransform;
    color = other.color;
}
