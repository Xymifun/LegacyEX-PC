#ifndef GAME_CORE_DATAINPUTSTREAM_HPP
#define GAME_CORE_DATAINPUTSTREAM_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/DataInput.hpp"
#include "game/core/InputStream.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPos;
class CompoundTag;
class ItemInstance;
struct PlayerUID;

class DataInputStream : public InputStream, public DataInput
{
public:
    DataInputStream(InputStream* stream);
    virtual ~DataInputStream() {}

    virtual int read();
    virtual int read(arrayWithLength<unsigned char> buffer);
    virtual int read(arrayWithLength<unsigned char> buffer, unsigned int offset, unsigned int length);
    virtual void close();
    virtual long long skip(long long count);

    virtual bool readBoolean();
    virtual unsigned char readByte();
    virtual unsigned char readUnsignedByte();
    virtual unsigned short readUnsignedShort();
    virtual wchar_t readChar();
    virtual bool readFully(arrayWithLength<unsigned char> buffer);
    virtual bool readFully(arrayWithLength<char> buffer);
    virtual bool readBytes(arrayWithLength<unsigned char> buffer);
    virtual double readDouble();
    virtual float readFloat();
    virtual int readInt();
    virtual long long readLong();
    virtual short readShort();
    virtual std::wstring readUTF();
    virtual int readUTFChar();
    virtual PlayerUID readPlayerUID();
    virtual long long skipBytes(long long count);

    void deleteChildStream();

    BlockPos readBlockPos();
    BlockPos* readBlockPosPointer();

    int readVarInt();
    long long readVarLong();
    arrayWithLength<int> readVarIntArray();

    CompoundTag* readNbt();
    CompoundTag* readCompressedNbt();

    not_null_ptr<ItemInstance> readItem();

private:
    InputStream* mChildStream;
};

#endif
