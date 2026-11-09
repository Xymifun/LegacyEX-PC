#ifndef GAME_DATA_CONSOLESAVEFILEORIGINAL_HPP
#define GAME_DATA_CONSOLESAVEFILEORIGINAL_HPP

#include "game/data/ConsoleSaveFileCommon.hpp"

class ConsoleSaveFileOriginal : public ConsoleSaveFileCommon
{
public:
    ConsoleSaveFileOriginal(const std::wstring& filename, void* data, unsigned int dataSize, bool resetData,
                            ESavePlatform platform);
    virtual ~ConsoleSaveFileOriginal();

    static int SaveSaveDataCallback(void* context, bool success);

    virtual FileEntry* createFile(const ConsoleSavePath& path);
    virtual void deleteFile(FileEntry* entry);
    virtual void setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
                                unsigned int moveMethod);
    virtual bool writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten);
    virtual bool zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten);
    virtual bool readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead);
    virtual void Flush(bool bForce, bool bBlocking);
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(unsigned int dimension);
    virtual unsigned char* getWritePointer(FileEntry* entry);
    virtual void refreshFromLevelgen();
    virtual void AllocInitialQuota(unsigned int size);
    virtual void ReAlloc(unsigned int size);

private:
    static const unsigned int SAVE_DATA_PAGE_SIZE = 0x10000;
    static const unsigned int SAVE_DATA_MIN_QUOTA = 0x200000;
};

#endif
