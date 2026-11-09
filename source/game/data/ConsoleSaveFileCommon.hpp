#ifndef GAME_DATA_CONSOLESAVEFILECOMMON_HPP
#define GAME_DATA_CONSOLESAVEFILECOMMON_HPP

#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/FileHeader.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSavePath.hpp"

class File;

class ConsoleSaveFileCommon : public ConsoleSaveFile
{
public:
    ConsoleSaveFileCommon();
    virtual ~ConsoleSaveFileCommon();
    virtual FileEntry* createFile(const ConsoleSavePath& path) = 0;
    virtual void deleteFile(FileEntry* entry) = 0;
    virtual void setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
                                unsigned int moveMethod) = 0;
    virtual bool writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
                           unsigned int* bytesWritten) = 0;
    virtual bool zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten) = 0;
    virtual bool readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead) = 0;
    virtual bool closeHandle(FileEntry* entry);
    virtual void finalizeWrite();
    virtual bool doesFileExist(ConsoleSavePath path);
    virtual void Flush(bool bForce, bool bBlocking) = 0;
    virtual unsigned int getSizeOnDisk();
    virtual std::wstring getFilename();
    virtual std::vector<FileEntry*>* getFilesWithPrefix(const std::wstring& prefix);
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(unsigned int dimension) = 0;
    virtual short getSaveVersion();
    virtual short getOriginalSaveVersion();
    virtual void LockSaveAccess();
    virtual void ReleaseSaveAccess();
    virtual ESavePlatform getSavePlatform();
    virtual bool isSaveEndianDifferent();
    virtual void setLocalPlatform();
    virtual void setPlatform(ESavePlatform platform);
    virtual ByteOrder getSaveEndian();
    virtual ByteOrder getLocalEndian();
    virtual void setEndian(ByteOrder endian);
    virtual void ConvertRegionFile(File regionFile);
    virtual void ConvertToLocalPlatform();
    virtual void AllocInitialQuota(unsigned int size) = 0;
    virtual void ReAlloc(unsigned int size) = 0;
    virtual bool isLocalEndianDifferent(ESavePlatform platform);
#ifdef MINECRAFT_PC
    static ESavePlatform PcBaseSavePlatform(const void* data, unsigned int size);
#endif
    void _initSaveFromData(const std::wstring& filename, unsigned int dataSize, void* data, ESavePlatform platform);
    void _deleteFile(FileEntry* entry);
    void MoveDataBeyond(FileEntry* entry, unsigned int size);
    void PrepareForWrite(FileEntry* entry, unsigned int size);
    bool _writeFile(FileEntry* entry, const void* buffer, unsigned int size, unsigned int* written);
    bool _zeroFile(FileEntry* entry, unsigned int size, unsigned int* written);
    bool _readFile(FileEntry* entry, void* buffer, unsigned int size, unsigned int* read);
    unsigned int fjGetSaveThumbnailData(unsigned char* data, unsigned int size);
    int fjCountSavedChunks(int dimension);
    virtual bool fjHasRegionFile(const File& file);

protected:
    FileHeader mFileHeader;
    std::wstring mFilename;
    OSFastMutex mSaveAccessMutex;
    void* mData;
};

#endif
