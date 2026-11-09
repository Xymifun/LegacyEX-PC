#ifndef GAME_CORE_NBTIO_HPP
#define GAME_CORE_NBTIO_HPP

#include "game/world/biome/arrayWithLength.hpp"

class CompoundTag;
class DataInput;
class DataOutput;
class InputStream;
class OutputStream;
class Tag;

class NbtIo
{
public:
    static Tag* readUnnamedTag(DataInput* input, int depth);
    static CompoundTag* read(DataInput* input);
    static CompoundTag* readCompressed(InputStream* stream);
    static void writeUnnamedTag(Tag* tag, DataOutput* output);
    static void write(CompoundTag* tag, DataOutput* output);
    static void writeCompressed(CompoundTag* tag, OutputStream* stream);
    static CompoundTag* decompress(arrayWithLength<unsigned char> data);
    static arrayWithLength<unsigned char> compress(CompoundTag* tag);
};

#endif
