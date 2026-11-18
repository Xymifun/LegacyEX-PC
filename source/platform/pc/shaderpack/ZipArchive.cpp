// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ZipArchive.cpp
#include "platform/pc/shaderpack/ZipArchive.hpp"

#include <fstream>
#include <iterator>

#include "zlib.h"

namespace
{
const std::uint32_t kEndOfCentralDirectory = 0x06054b50;
const std::uint32_t kCentralDirectoryEntry = 0x02014b50;
const std::uint32_t kLocalFileHeader = 0x04034b50;

std::uint16_t Read16(const unsigned char* data)
{
    return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
}

std::uint32_t Read32(const unsigned char* data)
{
    return static_cast<std::uint32_t>(data[0]) | (static_cast<std::uint32_t>(data[1]) << 8) |
           (static_cast<std::uint32_t>(data[2]) << 16) | (static_cast<std::uint32_t>(data[3]) << 24);
}
}

bool ZipArchive::Open(const std::string& path, std::string& error)
{
    mData.clear();
    mEntries.clear();
    mNames.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        error = "the file could not be opened";
        return false;
    }
    mData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    if (mData.size() < 22)
    {
        error = "the file is not a zip archive";
        return false;
    }

    size_t end = mData.size() - 22;
    size_t searchLimit = end > 0xFFFF ? end - 0xFFFF : 0;
    size_t eocd = std::string::npos;
    for (size_t offset = end + 1; offset-- > searchLimit;)
    {
        if (Read32(&mData[offset]) == kEndOfCentralDirectory)
        {
            eocd = offset;
            break;
        }
    }
    if (eocd == std::string::npos)
    {
        error = "the file is not a zip archive";
        return false;
    }

    std::uint16_t entryCount = Read16(&mData[eocd + 10]);
    std::uint32_t directorySize = Read32(&mData[eocd + 12]);
    std::uint32_t directoryOffset = Read32(&mData[eocd + 16]);
    if (entryCount == 0xFFFF || directoryOffset == 0xFFFFFFFFu)
    {
        error = "ZIP64 archives are not supported";
        return false;
    }
    if (static_cast<size_t>(directoryOffset) + directorySize > mData.size())
    {
        error = "the zip central directory is damaged";
        return false;
    }

    size_t offset = directoryOffset;
    for (std::uint16_t i = 0; i < entryCount; ++i)
    {
        if (offset + 46 > mData.size() || Read32(&mData[offset]) != kCentralDirectoryEntry)
        {
            error = "the zip central directory is damaged";
            return false;
        }
        const unsigned char* header = &mData[offset];
        Entry entry;
        entry.flags = Read16(header + 8);
        entry.method = Read16(header + 10);
        entry.crc = Read32(header + 16);
        entry.compressedSize = Read32(header + 20);
        entry.size = Read32(header + 24);
        std::uint16_t nameLength = Read16(header + 28);
        std::uint16_t extraLength = Read16(header + 30);
        std::uint16_t commentLength = Read16(header + 32);
        entry.localOffset = Read32(header + 42);
        if (offset + 46 + nameLength > mData.size())
        {
            error = "the zip central directory is damaged";
            return false;
        }

        std::string name(reinterpret_cast<const char*>(header + 46), nameLength);
        for (char& character : name)
        {
            if (character == '\\')
                character = '/';
        }
        if (!name.empty() && name.back() != '/')
        {
            if (mEntries.emplace(name, entry).second)
                mNames.push_back(name);
        }
        offset += 46 + nameLength + extraLength + commentLength;
    }
    return true;
}

bool ZipArchive::Read(const std::string& name, std::string& out, std::string& error) const
{
    auto found = mEntries.find(name);
    if (found == mEntries.end())
    {
        error = "missing from the archive";
        return false;
    }
    const Entry& entry = found->second;
    if ((entry.flags & 1) != 0)
    {
        error = "encrypted";
        return false;
    }
    if (static_cast<size_t>(entry.localOffset) + 30 > mData.size() ||
        Read32(&mData[entry.localOffset]) != kLocalFileHeader)
    {
        error = "the local header is damaged";
        return false;
    }
    const unsigned char* local = &mData[entry.localOffset];
    size_t dataOffset = static_cast<size_t>(entry.localOffset) + 30 + Read16(local + 26) + Read16(local + 28);
    if (dataOffset + entry.compressedSize > mData.size())
    {
        error = "the entry data is truncated";
        return false;
    }

    out.resize(entry.size);
    if (entry.method == 0)
    {
        if (entry.compressedSize != entry.size)
        {
            error = "the stored entry size is inconsistent";
            return false;
        }
        if (entry.size != 0)
            out.assign(reinterpret_cast<const char*>(&mData[dataOffset]), entry.size);
    }
    else if (entry.method == 8)
    {
        z_stream stream = {};
        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK)
        {
            error = "the inflater could not start";
            return false;
        }
        stream.next_in = const_cast<Bytef*>(&mData[dataOffset]);
        stream.avail_in = entry.compressedSize;
        stream.next_out = reinterpret_cast<Bytef*>(out.empty() ? nullptr : &out[0]);
        stream.avail_out = entry.size;
        int result = inflate(&stream, Z_FINISH);
        bool complete = result == Z_STREAM_END && stream.total_out == entry.size;
        inflateEnd(&stream);
        if (!complete)
        {
            error = "the compressed data is damaged";
            return false;
        }
    }
    else
    {
        error = "compression method " + std::to_string(entry.method) + " is not supported";
        return false;
    }

    uLong crc = crc32(0L, Z_NULL, 0);
    if (!out.empty())
        crc = crc32(crc, reinterpret_cast<const Bytef*>(out.data()), static_cast<uInt>(out.size()));
    if (crc != entry.crc)
    {
        error = "the checksum does not match";
        return false;
    }
    return true;
}
