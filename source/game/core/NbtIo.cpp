#include "game/core/NbtIo.hpp"

#include "game/core/BufferedOutputStream.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInput.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutput.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/InputStream.hpp"
#include "game/core/OutputStream.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/EndTag.hpp"
#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/MemSect.hpp"

#include <string>

Tag* NbtIo::readUnnamedTag(DataInput* input, int depth)
{
    unsigned char type = input->readByte();

    if (type == 0)
    {
        return new EndTag();
    }

    if (type == 255)
    {
        return new EndTag();
    }

    input->readUTF();

    Tag* tag = Tag::newTag(type);

    tag->load(input, depth);

    return tag;
}

CompoundTag* NbtIo::read(DataInput* input)
{
    Tag* tag = readUnnamedTag(input, 0);

    if (tag->getId() == Tag::TAG_COMPOUND)
    {
        return (CompoundTag*)tag;
    }

    delete tag;

    return NULL;
}

CompoundTag* NbtIo::readCompressed(InputStream* stream)
{
    MemSect(26);

    DataInputStream input(stream);

    CompoundTag* tag = read(&input);

    input.close();

    MemSect(0);

    return tag;
}

void NbtIo::writeUnnamedTag(Tag* tag, DataOutput* output)
{
    output->writeByte(tag->getId());

    if (tag->getId() == Tag::TAG_END)
        return;

    output->writeUTF(L"");

    tag->write(output);
}

void NbtIo::write(CompoundTag* tag, DataOutput* output)
{
    writeUnnamedTag(tag, output);
}

void NbtIo::writeCompressed(CompoundTag* tag, OutputStream* stream)
{
    BufferedOutputStream bufferedStream(stream, 1024);
    DataOutputStream output(&bufferedStream);

    write(tag, &output);

    output.close();
}

CompoundTag* NbtIo::decompress(arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream input((InputStream*)&byteStream);

    CompoundTag* tag = read(&input);

    byteStream.reset();

    input.close();

    return tag;
}

arrayWithLength<unsigned char> NbtIo::compress(CompoundTag* tag)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream output(&byteStream);

    write(tag, &output);

    arrayWithLength<unsigned char> bytes(byteStream.mCount, true);

    System::arraycopy(byteStream.mBuffer, 0, &bytes, 0, byteStream.mCount);

    output.close();

    return bytes;
}
