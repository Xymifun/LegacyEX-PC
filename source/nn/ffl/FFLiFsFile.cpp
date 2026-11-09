#include <nn/ffl/FFLiFsClient.h>
#include <nn/ffl/FFLiFsCommand.h>
#include <nn/ffl/FFLiFsFile.h>

FFLiFsFile::FFLiFsFile(FFLiFsCommand* pCommand) : m_pFsCommand(pCommand), m_Handle(FS_INVALID_HANDLE_VALUE)
{
}

FFLiFsFile::~FFLiFsFile()
{
}

FSStatus FFLiFsFile::Open(const char* path, const char* mode, u32 errHandling)
{
    return FSOpenFile(&m_pFsCommand->GetClient()->GetFSClient(), &m_pFsCommand->GetFSCmdBlock(), path, mode, &m_Handle,
                      static_cast<FSErrorFlag>(errHandling));
}

FSStatus FFLiFsFile::Close(u32 errHandling)
{
    FSStatus status = FSCloseFile(&m_pFsCommand->GetClient()->GetFSClient(), &m_pFsCommand->GetFSCmdBlock(), m_Handle,
                                  static_cast<FSErrorFlag>(errHandling));
    if (status == FS_STATUS_OK)
        m_Handle = FS_INVALID_HANDLE_VALUE;

    return status;
}

FSStatus FFLiFsFile::Read(void* dst, u32 size, u32 count, u32 errHandling, FSFlag flag)
{
    return FSReadFile(&m_pFsCommand->GetClient()->GetFSClient(), &m_pFsCommand->GetFSCmdBlock(), static_cast<u8*>(dst),
                      size, count, m_Handle, flag, static_cast<FSErrorFlag>(errHandling));
}

FSStatus FFLiFsFile::ReadWithPos(void* dst, u32 size, u32 count, u32 pos, u32 errHandling, FSFlag flag)
{
    return FSReadFileWithPos(&m_pFsCommand->GetClient()->GetFSClient(), &m_pFsCommand->GetFSCmdBlock(),
                             static_cast<u8*>(dst), size, count, pos, m_Handle, flag,
                             static_cast<FSErrorFlag>(errHandling));
}

FSStatus FFLiFsFile::Write(const void* src, u32 size, u32 count, u32 errHandling, FSFlag flag)
{
    return FSWriteFile(&m_pFsCommand->GetClient()->GetFSClient(), &m_pFsCommand->GetFSCmdBlock(),
                       static_cast<u8*>(const_cast<void*>(src)), size, count, m_Handle, flag,
                       static_cast<FSErrorFlag>(errHandling));
}

bool FFLiFsFile::IsOpened() const
{
    return m_Handle != FS_INVALID_HANDLE_VALUE;
}

FSStatus FFLiFsFile::FlushQuota(FFLiFsCommand* pCommand, const char* path, u32 errHandling)
{
    return FSFlushQuota(&pCommand->GetClient()->GetFSClient(), &pCommand->GetFSCmdBlock(), path,
                        static_cast<FSErrorFlag>(errHandling));
}
