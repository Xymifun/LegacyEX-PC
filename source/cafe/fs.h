#ifndef CAFE_FS_H
#define CAFE_FS_H

#include <coreinit/filesystem.h>

#include <cafe/os.h>

#include <nn/types.h>

typedef FSErrorFlag FSRetFlag;
typedef u32 FSFlag;

#define FS_RET_NO_ERROR FS_ERROR_FLAG_NONE
#define FS_RET_MAX FS_ERROR_FLAG_MAX
#define FS_RET_ALREADY_OPEN FS_ERROR_FLAG_ALREADY_OPEN
#define FS_RET_EXISTS FS_ERROR_FLAG_EXISTS
#define FS_RET_NOT_FOUND FS_ERROR_FLAG_NOT_FOUND
#define FS_RET_NOT_FILE FS_ERROR_FLAG_NOT_FILE
#define FS_RET_NOT_DIR FS_ERROR_FLAG_NOT_DIR
#define FS_RET_ACCESS_ERROR FS_ERROR_FLAG_ACCESS_ERROR
#define FS_RET_PERMISSION_ERROR FS_ERROR_FLAG_PERMISSION_ERROR
#define FS_RET_FILE_TOO_BIG FS_ERROR_FLAG_FILE_TOO_BIG
#define FS_RET_STORAGE_FULL FS_ERROR_FLAG_STORAGE_FULL
#define FS_RET_UNSUPPORTED_CMD FS_ERROR_FLAG_UNSUPPORTED_CMD
#define FS_RET_JOURNAL_FULL FS_ERROR_FLAG_JOURNAL_FULL
#define FS_RET_ALL_ERROR FS_ERROR_FLAG_ALL

#define FS_INVALID_HANDLE_VALUE (-1)
#define FS_IO_BUFFER_ALIGN 64

#ifdef __cplusplus
inline FSRetFlag operator|(FSRetFlag a, FSRetFlag b)
{
    return static_cast<FSRetFlag>(static_cast<u32>(a) | static_cast<u32>(b));
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

FSStatus FSFlushQuota(FSClient* client, FSCmdBlock* block, const char* path, FSRetFlag errHandling);

#ifdef __cplusplus
}
#endif

#endif
