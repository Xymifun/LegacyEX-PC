#include "game/core/DataOutputStream.hpp"
#include <stdio.h>
#include "game/core/System.hpp"

#include <string.h>

#include "game/core/Compression.hpp"
#include "game/core/Double.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/NbtIo.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/BlockPos.hpp"

DataOutputStream::DataOutputStream(OutputStream* out)
{
    mOut = out;
    mWritten = 0;
}

void DataOutputStream::write(unsigned int value)
{
    mOut->write(value);

    mWritten++;
}

void DataOutputStream::write(arrayWithLength<unsigned char> data)
{
    write(data, 0, data.length);
}

void DataOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    mOut->write(data, offset, length);

    mWritten += length;
}

void DataOutputStream::close()
{
    mOut->close();
}

void DataOutputStream::flush()
{
    mOut->flush();
}

void DataOutputStream::writeBytes(arrayWithLength<unsigned char> data)
{
    write(data, 0, data.length);
}

void DataOutputStream::writeByte(unsigned char value)
{
    mOut->write(value);
}

void DataOutputStream::writeDouble(double value)
{
    long long bits = Double::doubleToLongBits(value);

    writeLong(bits);

    mWritten += 8;
}

void DataOutputStream::writeFloat(float value)
{
    union
    {
        float asFloat;
        int asBits;
    } converted;

    converted.asFloat = value;

    writeInt(converted.asBits);

    mWritten += 4;
}

void DataOutputStream::writeInt(int value)
{
    mOut->write((value >> 24) & 0xFF);
    mOut->write((value >> 16) & 0xFF);
    mOut->write((value >> 8) & 0xFF);
    mOut->write(value & 0xFF);

    mWritten += 4;
}

void DataOutputStream::writeLong(long long value)
{
    mOut->write((unsigned int)((value >> 56) & 0xFF));
    mOut->write((unsigned int)((value >> 48) & 0xFF));
    mOut->write((unsigned int)((value >> 40) & 0xFF));
    mOut->write((unsigned int)((value >> 32) & 0xFF));
    mOut->write((unsigned int)((value >> 24) & 0xFF));
    mOut->write((unsigned int)((value >> 16) & 0xFF));
    mOut->write((unsigned int)((value >> 8) & 0xFF));
    mOut->write((unsigned int)(value & 0xFF));

    mWritten += 4;
}

void DataOutputStream::writeShort(short value)
{
    mOut->write((value >> 8) & 0xFF);
    mOut->write(value & 0xFF);

    mWritten += 2;
}

void DataOutputStream::writeUnsignedShort(unsigned short value)
{
    mOut->write((value >> 8) & 0xFF);
    mOut->write(value & 0xFF);

    mWritten += 2;
}

void DataOutputStream::writeChar(wchar_t value)
{
    mOut->write((value >> 8) & 0xFF);
    mOut->write(value & 0xFF);

    mWritten += 2;
}

void DataOutputStream::writeChars(const std::wstring& text)
{
    for (unsigned int i = 0; i < text.length(); i++)
    {
        writeChar(text.at(i));
    }
}

void DataOutputStream::writeBoolean(bool value)
{
    mOut->write(value);

    mWritten++;
}

void DataOutputStream::writeUTF(const std::wstring& text)
{
    int textLength = text.length();
    int utfLength = 0;
    int i;
    int c;

    for (i = 0; i < textLength; i++)
    {
        c = text.at(i);

        if (c >= 0x0001 && c <= 0x007F)
        {
            utfLength++;
        }
        else if (c > 0x07FF)
        {
            utfLength += 3;
        }
        else
        {
            utfLength += 2;
        }
    }

    arrayWithLength<unsigned char> bytes(utfLength + 2, true);

    bytes[0] = (unsigned char)((utfLength >> 8) & 0xFF);
    bytes[1] = (unsigned char)(utfLength & 0xFF);

    int count = 2;

    for (i = 0; i < textLength; i++)
    {
        c = text.at(i);

        if (c < 0x0001 || c > 0x007F)
        {
            break;
        }

        bytes[count++] = (unsigned char)c;
    }

    for (; i < textLength; i++)
    {
        c = text.at(i);

        if (c >= 0x0001 && c <= 0x007F)
        {
            bytes[count++] = (unsigned char)c;
        }
        else if (c > 0x07FF)
        {
            bytes[count++] = (unsigned char)(0xE0 | ((c >> 12) & 0x0F));
            bytes[count++] = (unsigned char)(0x80 | ((c >> 6) & 0x3F));
            bytes[count++] = (unsigned char)(0x80 | (c & 0x3F));
        }
        else
        {
            bytes[count++] = (unsigned char)(0xC0 | ((c >> 6) & 0x1F));
            bytes[count++] = (unsigned char)(0x80 | (c & 0x3F));
        }
    }

    write(bytes, 0, utfLength + 2);

    delete[] bytes.data;
}

void DataOutputStream::writePlayerUID(PlayerUID uid)
{
    const unsigned char* bytes = (const unsigned char*)&uid;

    for (unsigned int i = 0; i < sizeof(PlayerUID); i++)
    {
        writeByte(bytes[i]);
    }
}

int DataOutputStream::GetCompressedNbtSize(CompoundTag* tag)
{
    if (tag == NULL)
    {
        return 2;
    }

    arrayWithLength<unsigned char> nbtData = NbtIo::compress(tag);

    unsigned int compressedSize = 0;
    Compression::getCompression()->Compress(NULL, &compressedSize, nbtData.data, nbtData.length);

    delete[] nbtData.data;

    return compressedSize + 4;
}

void DataOutputStream::deleteChildStream()
{
    if (mOut != NULL)
    {
        delete mOut;
    }
}

void DataOutputStream::writeBlockPos(const BlockPos& pos)
{
    writeLong(pos.compressBlockPos());
}

void DataOutputStream::writeVarInt(int value)
{
    unsigned int bits;
    memcpy(&bits, &value, sizeof(bits));

    for (;;)
    {
        if ((bits & 0xFFFFFF80) == 0)
        {
            writeByte((unsigned char)bits);

            return;
        }

        writeByte((unsigned char)((bits & 0x7F) | 0x80));

        bits >>= 7;
    }
}

void DataOutputStream::writeVarLong(long long value)
{
    unsigned long long bits;
    memcpy(&bits, &value, sizeof(bits));

    for (;;)
    {
        if ((bits & ~(unsigned long long)0x7F) == 0)
        {
            writeByte((unsigned char)bits);

            return;
        }

        writeByte((unsigned char)((bits & 0x7F) | 0x80));

        bits >>= 7;
    }
}

void DataOutputStream::writeNbt(CompoundTag* tag)
{
    if (tag == NULL)
    {
        writeShort(-1);

        return;
    }

    arrayWithLength<unsigned char> nbtData = NbtIo::compress(tag);

#ifdef MINECRAFT_PC
    if (nbtData.length > 32767)
    {
        static unsigned long long lastReport = 0;
        const unsigned long long now = System::processTimeInMilliSecs();
        if (lastReport == 0 || now - lastReport > 10000)
        {
            lastReport = now;
            printf("[pc] net: an item's data is %u bytes, over the 32,767 a packet can carry; sent without it\n",
                   nbtData.length);
            fflush(stdout);
        }
        writeShort(-1);
        delete[] nbtData.data;
        return;
    }
#endif
    writeShort((short)nbtData.length);
    write(nbtData);

    delete[] nbtData.data;
}

void DataOutputStream::writeCompressedNbt(CompoundTag* tag)
{
    if (tag == NULL)
    {
        writeShort(-1);

        return;
    }

    arrayWithLength<unsigned char> nbtData = NbtIo::compress(tag);

    unsigned int compressedSize = 0;
    Compression::getCompression()->Compress(NULL, &compressedSize, nbtData.data, nbtData.length);

    arrayWithLength<unsigned char> compressedData(compressedSize, true);

    Compression::getCompression()->Compress(compressedData.data, &compressedSize, nbtData.data, nbtData.length);
    compressedData.length = compressedSize;

    writeShort((short)nbtData.length);
    writeShort((short)compressedData.length);
    write(compressedData);

    delete[] nbtData.data;
    delete[] compressedData.data;
}

void DataOutputStream::writeItem(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        writeShort(-1);

        return;
    }

    writeShort(Item::VALIDgetId(item->getItem()));
    writeByte(item->getCount());
    writeShort(item->getAuxValue());
    CompoundTag* tag = item->getTag();

    writeNbt(tag);
}

void DataOutputStream::writeVarIntArray(arrayWithLength<int> values)
{
    unsigned int count = values.length;

    writeVarInt(count);

    for (unsigned int i = 0; i < count; i++)
    {
        writeVarInt(values.data[i]);
    }
}
