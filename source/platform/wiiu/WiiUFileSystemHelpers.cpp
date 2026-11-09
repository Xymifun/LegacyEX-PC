#include "platform/wiiu/WiiUFileSystemHelpers.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern void setAltMainOKToRun();
extern FSCmdBlock* GetCommandBlock();
extern void waitForAltMainThreadToComplete();

int OpenDir(const char* path, int* directory)
{
    setAltMainOKToRun();
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSOpenDir(gFileSystemClient, commandBlock, path, reinterpret_cast<FSDirectoryHandle*>(directory),
                                FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}
