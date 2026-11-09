#ifndef GAME_CORE_REGIONFILECACHE_HPP
#define GAME_CORE_REGIONFILECACHE_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <boost/unordered_map.hpp>

#include "game/core/File.hpp"
#include "game/core/FileHeader.hpp"

class ConsoleSaveFile;
class DataInputStream;
class DataOutputStream;
class RegionFile;

class _FileKeyHash
{
public:
    std::size_t operator()(const File& file) const { return File::hash_fnct(file); }
};

class FileKeyEq
{
public:
    bool operator()(const File& first, const File& second) const { return File::eq_test(first, second); }
};

class RegionFileCache
{
public:
    typedef boost::unordered_map<File, RegionFile*, _FileKeyHash, FileKeyEq,
                                 std::allocator<std::pair<const File, RegionFile*> > >
        RegionFileMap;

    void _clear();

    ~RegionFileCache();

    bool useSplitSaves(ESavePlatform platform);

    File getFilepath(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z);

    RegionFile* _getRegionFile(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z);
    int _getSizeDelta(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z);
    DataInputStream* _getChunkDataInputStream(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z,
                                              bool* pbNewSaveFormat);
    DataOutputStream* _getChunkDataOutputStream(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z,
                                                bool bNewSaveFormat);

    static File fjGetFilepath(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z);

    static DataOutputStream* getChunkDataOutputStream(ConsoleSaveFile* saveFile, const std::wstring& folder, int x,
                                                      int z);

private:
    RegionFileMap mRegionFiles;
};

extern RegionFileCache gRegionFileCache;

inline DataOutputStream* RegionFileCache::getChunkDataOutputStream(ConsoleSaveFile* saveFile,
                                                                   const std::wstring& folder, int x, int z)
{
    return gRegionFileCache._getChunkDataOutputStream(saveFile, folder, x, z, true);
}

#endif
