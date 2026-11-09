#ifndef GAME_CORE_DATAOUTPUTSTREAM_HPP
#define GAME_CORE_DATAOUTPUTSTREAM_HPP

#include <string>

#include "game/core/DataOutput.hpp"
#include "game/core/OutputStream.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerUID.hpp"

class BlockPos;
class CompoundTag;
class ItemInstance;

class DataOutputStream : public OutputStream, public DataOutput
{
public:
    DataOutputStream(OutputStream* out);
    virtual ~DataOutputStream() {}

    virtual void write(unsigned int value);
    virtual void write(arrayWithLength<unsigned char> data);
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual void flush();
    virtual void writeBytes(arrayWithLength<unsigned char> data);
    virtual void writeByte(unsigned char value);
    virtual void writeDouble(double value);
    virtual void writeFloat(float value);
    virtual void writeInt(int value);
    virtual void writeLong(long long value);
    virtual void writeShort(short value);
    virtual void writeUnsignedShort(unsigned short value);
    virtual void writeChar(wchar_t value);
    virtual void writeChars(const std::wstring& text);
    virtual void writeBoolean(bool value);
    virtual void writeUTF(const std::wstring& text);
    virtual void writePlayerUID(PlayerUID uid);

    static int GetCompressedNbtSize(CompoundTag* tag);

    void deleteChildStream();
    void writeBlockPos(const BlockPos& pos);
    void writeVarInt(int value);
    void writeVarLong(long long value);
    void writeNbt(CompoundTag* tag);
    void writeCompressedNbt(CompoundTag* tag);
    void writeItem(not_null_ptr<ItemInstance> item);
    void writeVarIntArray(arrayWithLength<int> values);

private:
    friend class Connection;

    OutputStream* mOut;
    int mWritten;
};

#endif
