#ifndef GAME_DATA_CONSOLESAVEFILE_HPP
#define GAME_DATA_CONSOLESAVEFILE_HPP

#include <string>
#include <vector>

#include "game/core/FileEntry.hpp"
#include "game/core/FileHeader.hpp"
#include "game/data/ConsoleSavePath.hpp"

class File;

class ConsoleSaveFile
{
public:
    virtual ~ConsoleSaveFile() {}

    virtual FileEntry* createFile(const ConsoleSavePath& path) = 0;
    virtual void deleteFile(FileEntry* entry) = 0;
    virtual void setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
                                unsigned int moveMethod) = 0;
    virtual bool writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
                           unsigned int* bytesWritten) = 0;
    virtual bool zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten) = 0;
    virtual bool readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead) = 0;
    virtual bool closeHandle(FileEntry* entry) = 0;
    virtual void finalizeWrite() = 0;
    virtual void tick();
    virtual bool doesFileExist(ConsoleSavePath path) = 0;
    virtual void Flush(bool bForce, bool bBlocking) = 0;
    virtual unsigned int getSizeOnDisk() = 0;
    virtual std::wstring getFilename() = 0;
    virtual std::vector<FileEntry*>* getFilesWithPrefix(const std::wstring& prefix) = 0;
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(unsigned int dimension) = 0;
    virtual short getSaveVersion() = 0;
    virtual short getOriginalSaveVersion() = 0;
    virtual void LockSaveAccess() = 0;
    virtual void ReleaseSaveAccess() = 0;
    virtual ESavePlatform getSavePlatform() = 0;
    virtual bool isSaveEndianDifferent() = 0;
    virtual void setLocalPlatform() = 0;
    virtual void setPlatform(ESavePlatform platform) = 0;
    virtual ByteOrder getSaveEndian() = 0;
    virtual ByteOrder getLocalEndian() = 0;
    virtual void setEndian(ByteOrder endian) = 0;
    virtual void ConvertRegionFile(File regionFile) = 0;
    virtual void ConvertToLocalPlatform() = 0;
    virtual unsigned char* getWritePointer(FileEntry* entry);
    virtual void refreshFromLevelgen();
};

#endif
