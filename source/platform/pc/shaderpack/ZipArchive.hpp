// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ZipArchive.hpp
#ifndef PLATFORM_PC_SHADERPACK_ZIPARCHIVE_HPP
#define PLATFORM_PC_SHADERPACK_ZIPARCHIVE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class ZipArchive
{
public:
    bool Open(const std::string& path, std::string& error);
    const std::vector<std::string>& Names() const { return mNames; }
    bool Contains(const std::string& name) const { return mEntries.count(name) != 0; }
    bool Read(const std::string& name, std::string& out, std::string& error) const;

private:
    struct Entry
    {
        std::uint16_t flags;
        std::uint16_t method;
        std::uint32_t crc;
        std::uint32_t compressedSize;
        std::uint32_t size;
        std::uint32_t localOffset;
    };

    std::vector<unsigned char> mData;
    std::unordered_map<std::string, Entry> mEntries;
    std::vector<std::string> mNames;
};

#endif
