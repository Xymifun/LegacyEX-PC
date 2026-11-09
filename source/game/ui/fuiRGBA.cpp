#include "game/ui/fuiRGBA.hpp"

void Swap4Bytes(unsigned int& value);

void fuiRGBA::EndianSwap()
{
    Swap4Bytes(rgba);
}
