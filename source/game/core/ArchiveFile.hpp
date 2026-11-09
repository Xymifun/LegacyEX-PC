#ifndef GAME_CORE_ARCHIVEFILE_HPP
#define GAME_CORE_ARCHIVEFILE_HPP
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include "game/core/File.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class DataInputStream;
class ArchiveFile : public File
{
public:
    struct _MetaData
    {
        std::wstring name;
        unsigned int offset;
        unsigned int size;
        bool compressed;
    };
    ArchiveFile(File file);
    ArchiveFile(unsigned char* data, unsigned int size);
    ~ArchiveFile();
    void _readHeader(DataInputStream* stream);
    bool hasFile(const std::wstring& name);
    arrayWithLength<unsigned char> getFile(const std::wstring& name);
    unsigned int getFileSize(const std::wstring& name);
    std::vector<std::wstring> getFileList();
    void dereferenceCachedData();

private:
    unsigned char* m_data;
    bool m_cachedData;
    boost::unordered_map<std::wstring, _MetaData*> m_files;
#ifdef MINECRAFT_PC
    struct _ModFile
    {
        std::wstring name;
        std::string path;
    };
    void _readModFiles();
    const _ModFile* _findModFile(const std::wstring& name);
    boost::unordered_map<std::wstring, _ModFile> m_modFiles;
#endif
};
#endif
