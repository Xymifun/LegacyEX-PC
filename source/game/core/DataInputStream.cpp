#include "game/core/DataInputStream.hpp"

#include <cstring>

#include "game/core/Compression.hpp"
#include "game/core/NbtIo.hpp"
#include "game/item/Item.hpp"
namespace DataInputStreamInteractionHand
{
enum EInteractionHand
{
    MAIN_HAND,
    OFF_HAND
};
}
#define InteractionHand DataInputStreamInteractionHand
#include "game/item/ItemInstance.hpp"
#undef InteractionHand
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/PlayerUID.hpp"

DataInputStream::DataInputStream(InputStream* stream)
{
    mChildStream = stream;
}

int DataInputStream::read()
{
    return mChildStream->read();
}

int DataInputStream::read(arrayWithLength<unsigned char> buffer)
{
    return read(buffer, 0, buffer.length);
}

int DataInputStream::read(arrayWithLength<unsigned char> buffer, unsigned int offset, unsigned int length)
{
    return mChildStream->read(buffer, offset, length);
}

void DataInputStream::close()
{
    mChildStream->close();
}

long long DataInputStream::skip(long long count)
{
    return mChildStream->skip(count);
}

bool DataInputStream::readBoolean()
{
    return mChildStream->read() != 0;
}

unsigned char DataInputStream::readByte()
{
    return (unsigned char)mChildStream->read();
}

unsigned char DataInputStream::readUnsignedByte()
{
    return (unsigned char)mChildStream->read();
}

unsigned short DataInputStream::readUnsignedShort()
{
    int high = mChildStream->read();
    int low = mChildStream->read();

    return (unsigned short)((high << 8) | (low & 0xFF));
}

wchar_t DataInputStream::readChar()
{
    int high = mChildStream->read();
    int low = mChildStream->read();

    return (wchar_t)(unsigned short)((high << 8) | (low & 0xFF));
}

bool DataInputStream::readFully(arrayWithLength<unsigned char> buffer)
{
    for (unsigned int index = 0; index < buffer.length; index++)
    {
        int value = mChildStream->read();

        if (value == -1)
        {
            return false;
        }

        buffer.data[index] = (unsigned char)value;
    }

    return true;
}

bool DataInputStream::readFully(arrayWithLength<char> buffer)
{
    for (unsigned int index = 0; index < buffer.length; index++)
    {
        int value = mChildStream->read();

        if (value == -1)
        {
            return false;
        }

        buffer.data[index] = (char)value;
    }

    return true;
}

bool DataInputStream::readBytes(arrayWithLength<unsigned char> buffer)
{
    return readFully(buffer);
}

double DataInputStream::readDouble()
{
    union
    {
        long long bits;
        double value;
    } converter;

    converter.bits = readLong();
    return converter.value;
}

float DataInputStream::readFloat()
{
    union
    {
        int bits;
        float value;
    } converter;

    converter.bits = readInt();
    return converter.value;
}

int DataInputStream::readInt()
{
    int byte1 = mChildStream->read();
    int byte2 = mChildStream->read();
    int byte3 = mChildStream->read();
    int byte4 = mChildStream->read();

    return (byte1 << 24) | ((byte2 & 0xFF) << 16) | ((byte3 & 0xFF) << 8) | (byte4 & 0xFF);
}

long long DataInputStream::readLong()
{
    int byte1 = mChildStream->read();
    int byte2 = mChildStream->read();
    int byte3 = mChildStream->read();
    int byte4 = mChildStream->read();
    int byte5 = mChildStream->read();
    int byte6 = mChildStream->read();
    int byte7 = mChildStream->read();
    int byte8 = mChildStream->read();

    int high = (byte1 << 24) | ((byte2 & 0xFF) << 16) | ((byte3 & 0xFF) << 8) | (byte4 & 0xFF);

    return ((long long)high << 32) | ((unsigned long long)(unsigned int)(byte5 & 0xFF) << 24)
           | ((unsigned long long)(unsigned int)(byte6 & 0xFF) << 16)
           | ((unsigned long long)(unsigned int)(byte7 & 0xFF) << 8) | (unsigned long long)(unsigned int)(byte8 & 0xFF);
}

short DataInputStream::readShort()
{
    int high = mChildStream->read();
    int low = mChildStream->read();

    return (short)((high << 8) | (low & 0xFF));
}

std::wstring DataInputStream::readUTF()
{
    std::wstring text;

    int lengthHigh = mChildStream->read();
    int lengthLow = mChildStream->read();

    unsigned short length = (unsigned short)(((lengthHigh & 0xFF) << 8) | (lengthLow & 0xFF));
    unsigned short count = 0;

    while (count < length)
    {
        int byte1 = mChildStream->read();
        count++;

        if (byte1 == -1 || (byte1 & 0xC0) == 0x80 || (byte1 & 0xF0) == 0xF0)
        {
            break;
        }

        if ((byte1 & 0x80) == 0)
        {
            text.push_back((wchar_t)byte1);
        }
        else if ((byte1 & 0xE0) == 0xC0)
        {
            if (count >= length)
            {
                break;
            }

            int byte2 = mChildStream->read();
            count++;

            if (byte2 == -1 || (byte2 & 0xC0) != 0x80)
            {
                break;
            }

            text.push_back((wchar_t)(((byte1 & 0x1F) << 6) | (byte2 & 0x3F)));
        }
        else if ((byte1 & 0xF0) == 0xE0)
        {
            if (count >= length)
            {
                break;
            }

            int byte2 = mChildStream->read();
            count++;

            if (byte2 == -1 || count >= length)
            {
                break;
            }

            int byte3 = mChildStream->read();
            count++;

            if (byte3 == -1 || (byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80)
            {
                break;
            }

            text.push_back((wchar_t)(((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F)));
        }
    }

    return text;
}

int DataInputStream::readUTFChar()
{
    int result = -1;
    int byte1 = mChildStream->read();

    if (byte1 != -1 && (byte1 & 0xC0) != 0x80 && (byte1 & 0xF0) != 0xF0)
    {
        result = byte1;

        if ((byte1 & 0x80) != 0)
        {
            if ((byte1 & 0xE0) == 0xC0)
            {
                int byte2 = mChildStream->read();

                result = -1;

                if (byte2 != -1 && (byte2 & 0xC0) == 0x80)
                {
                    result = ((byte1 & 0x1F) << 6) | (byte2 & 0x3F);
                }
            }
            else
            {
                int byte2;
                int byte3;

                result = -1;

                if ((byte1 & 0xF0) == 0xE0 && (byte2 = mChildStream->read()) != -1
                    && (byte3 = mChildStream->read()) != -1 && (byte2 & 0xC0) == 0x80 && (byte3 & 0xC0) == 0x80)
                {
                    result = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
                }
            }
        }
    }

    return result;
}

PlayerUID DataInputStream::readPlayerUID()
{
    PlayerUID playerUID(0);

    unsigned char* bytes = (unsigned char*)&playerUID;

    for (unsigned int index = 0; index < sizeof(PlayerUID); index++)
    {
        bytes[index] = readByte();
    }

    return playerUID;
}

long long DataInputStream::skipBytes(long long count)
{
    return skip(count);
}

void DataInputStream::deleteChildStream()
{
    if (mChildStream != NULL)
    {
        delete mChildStream;
    }
}

BlockPos DataInputStream::readBlockPos()
{
    return BlockPos::decompressBlockPos(readLong());
}

BlockPos* DataInputStream::readBlockPosPointer()
{
    return BlockPos::decompressBlockPosAsPointer(readLong());
}

int DataInputStream::readVarInt()
{
    int value = 0;
    int shift = 0;
    int byteCount = 0;
    unsigned char byteValue;

    do
    {
        byteValue = readByte();
        byteCount++;

        value |= (byteValue & 0x7F) << shift;
        shift += 7;

        if (byteCount > 5)
        {
            return -1;
        }
    } while ((byteValue & 0x80) != 0);

    return value;
}

long long DataInputStream::readVarLong()
{
    int value = 0;
    int shift = 0;
    int byteCount = 0;
    unsigned char byteValue;

    do
    {
        byteValue = readByte();
        byteCount++;

        value |= (byteValue & 0x7F) << shift;
        shift += 7;

        if (byteCount > 10)
        {
            return -1;
        }
    } while ((byteValue & 0x80) != 0);

    return value;
}

arrayWithLength<int> DataInputStream::readVarIntArray()
{
    int count = readVarInt();

    arrayWithLength<int> values;

    if (count > 0)
    {
        values = arrayWithLength<int>(count, true);
    }

    unsigned int length = values.length;

    for (unsigned int index = 0; index < length; index++)
    {
        values.data[index] = readVarInt();
    }

    return values;
}

CompoundTag* DataInputStream::readNbt()
{
    int length = readShort();

#ifdef MINECRAFT_PC
    if (length < -1)
        length = static_cast<unsigned short>(length);
#endif
    if (length < 0)
    {
        return NULL;
    }

    arrayWithLength<unsigned char> data(length, true);

    readFully(data);

    CompoundTag* tag = NbtIo::decompress(data);

    delete[] data.data;

    return tag;
}

CompoundTag* DataInputStream::readCompressedNbt()
{
    int length = readShort();

    if (length < 0)
    {
        return NULL;
    }

    int compressedLength = readShort();

    if (compressedLength < 0)
    {
        return NULL;
    }

    arrayWithLength<unsigned char> compressedData(compressedLength, true);

    readFully(compressedData);

    arrayWithLength<unsigned char> data(length, true);

    unsigned int decompressedLength = length;

    Compression::getCompression()->Decompress(data.data, &decompressedLength, compressedData.data, compressedLength);

    CompoundTag* tag = NbtIo::decompress(data);

    delete[] data.data;
    delete[] compressedData.data;

    return tag;
}

not_null_ptr<ItemInstance> DataInputStream::readItem()
{
    not_null_ptr<ItemInstance> itemInstance = ItemInstance::EMPTY;

    int id = readShort();

    if (id >= 0)
    {
        unsigned char count = readByte();
        int auxValue = readShort();

        itemInstance = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(id), count, auxValue));

        itemInstance->setTag(readNbt());
    }

    return itemInstance;
}
