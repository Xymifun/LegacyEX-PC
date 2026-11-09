#ifndef PLATFORM_PC_PCSAVEFILESPLIT_HPP
#define PLATFORM_PC_PCSAVEFILESPLIT_HPP

#include "game/data/ConsoleSaveFileOriginal.hpp"

#include <stdio.h>

#include <map>
#include <string>
#include <vector>

class PcSaveFileSplit : public ConsoleSaveFileOriginal
{
public:
    PcSaveFileSplit(const std::wstring& filename, void* data, unsigned int dataSize, ESavePlatform platform);
    virtual ~PcSaveFileSplit();

    virtual FileEntry* createFile(const ConsoleSavePath& path);
    virtual void deleteFile(FileEntry* entry);
    virtual void setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
                                unsigned int moveMethod);
    virtual bool writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten);
    virtual bool zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten);
    virtual bool readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead);
    virtual bool doesFileExist(ConsoleSavePath path);
    virtual void Flush(bool bForce, bool bBlocking);
    virtual unsigned int getSizeOnDisk();
    virtual std::vector<FileEntry*>* getFilesWithPrefix(const std::wstring& prefix);
    virtual unsigned char* getWritePointer(FileEntry* entry);
    virtual bool fjHasRegionFile(const File& file);

private:
    struct Region
    {
        FileEntry entry;
        std::string diskName;
        bool committed;
        bool working;
        bool deleted;
        FILE* handle;
        bool handleWritable;
        unsigned long long lastUse;
    };

    static bool IsRegionName(const std::wstring& name);
    static std::string DiskName(const std::wstring& name);
    static std::wstring SaveName(const std::string& diskName);

    std::string CurrentSaveName();
    void UseDirectoryFor(const std::string& saveName);
    void ScanDirectory();
    void MigrateContainerRegions();
    Region* Find(const std::wstring& name);
    Region* FindOrCreate(const std::wstring& name);
    Region* RegionFor(FileEntry* entry);
    bool Exists(const Region* region) const;
    std::string CommittedPath(const Region* region) const;
    std::string WorkPath(const Region* region) const;
    FILE* Open(Region* region, bool writable);
    void CloseHandle(Region* region);
    void LimitOpenHandles();
    bool EnsureWorking(Region* region);
    bool WriteAt(Region* region, const void* buffer, unsigned int size, bool zero);

    std::map<std::wstring, Region*> mRegions;
    std::map<FileEntry*, Region*> mRegionsByEntry;
    std::string mSaveName;
    std::string mDirectory;
    unsigned long long mUseCounter;
    unsigned long long mBytesRead;
    unsigned long long mBytesWritten;
};

#endif
