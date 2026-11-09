#include "game/core/StbiFile.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

#include <stddef.h>

static const unsigned int FILE_CURRENT = 1;

int StbiFile::read(void* user, char* data, int size)
{
    StbiFile* stbiFile = (StbiFile*)user;

    unsigned int bytesRead = 0;

    if (ReadFile(stbiFile->mFile, data, size, &bytesRead, NULL) < 0)
    {
        return -1;
    }

    stbiFile->mFilePosition += bytesRead;

    return bytesRead;
}

void StbiFile::skip(void* user, int count)
{
    StbiFile* stbiFile = (StbiFile*)user;

    SetFilePointer(stbiFile->mFile, count, NULL, FILE_CURRENT);

    stbiFile->mFilePosition += count;
}

int StbiFile::eof(void* user)
{
    StbiFile* stbiFile = (StbiFile*)user;

    return (stbiFile->mFileSize - stbiFile->mFilePosition) <= 0;
}
