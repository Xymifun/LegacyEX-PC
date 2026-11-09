#ifndef GAME_CORE_FILE_HPP
#define GAME_CORE_FILE_HPP
#include <string>
#include <vector>
class File;
class FileFilter
{
public:
    virtual ~FileFilter() {}
    virtual bool accept(const File& file) = 0;
};
class File
{
public:
    File();
    File(const std::wstring& path);
    File(const std::wstring& parent, const std::wstring& child);
    File(const File& parent, const std::wstring& child);
    ~File() {}
    std::wstring getPath() const;
    bool exists() const;
    bool isDirectory() const;
    bool mkdir() const;
    bool isFile() const;
    std::vector<File*>* listFiles() const;
    std::vector<File*>* listFiles(FileFilter* filter) const;
    long long length();
    std::wstring getName() const;
    std::wstring ext() const;
    static int hash_fnct(const File& file);
    static bool eq_test(const File& first, const File& second);
    bool _delete();
    bool mkdirs() const;
    bool renameTo(File destination);
    long long lastModified();
    std::wstring getBaseName() const;
    File parentDir();

private:
    std::wstring mPath;
};
#endif
