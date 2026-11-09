#ifndef GAME_DATA_CONSOLESAVEFILEREADONLY_HPP
#define GAME_DATA_CONSOLESAVEFILEREADONLY_HPP

#include "game/data/ConsoleSaveFileCommon.hpp"

class ConsoleSaveFileReadOnly : public ConsoleSaveFileCommon
{
public:
    ConsoleSaveFileReadOnly(const std::wstring& filename, void* data, unsigned int dataSize, bool resetData,
                            ESavePlatform platform);
    virtual ~ConsoleSaveFileReadOnly();

    virtual FileEntry* createFile(const ConsoleSavePath& path);
    virtual void deleteFile(FileEntry* entry);
    virtual void setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
                                unsigned int moveMethod);
    virtual bool writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten);
    virtual bool zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten);
    virtual bool readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead);
    virtual void Flush(bool bForce, bool bBlocking);
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(unsigned int dimension);
    virtual void refreshFromLevelgen();
    virtual void AllocInitialQuota(unsigned int size);
    virtual void ReAlloc(unsigned int size);

private:
    static const unsigned int SAVE_DATA_PAGE_SIZE = 0x10000;
    static const unsigned int SAVE_DATA_MIN_QUOTA = 0x200000;
};

#endif
