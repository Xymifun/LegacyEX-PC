#include <nn/ffl/FFLResult.h>

#include <nn/ffl/FFLiDatabaseFile.h>
#include <nn/ffl/FFLiDatabaseFileAccessor.h>
#include <nn/ffl/FFLiFsClient.h>
#include <nn/ffl/FFLiFsFile.h>
#include <nn/ffl/FFLiPath.h>
#include <nn/ffl/FFLiUtil.h>

#include <nn/ffl/detail/FFLiAllocator.h>
#include <nn/ffl/detail/FFLiFileWriteBuffer.h>

#include <cstring>

enum FFLiFsFileResult
{
    FFLI_FS_FILE_RESULT_OK = 0,
    FFLI_FS_FILE_RESULT_READ_BUFFER_EMPTY = 1,
    FFLI_FS_FILE_RESULT_WRITE_BUFFER_EMPTY = 2,
    FFLI_FS_FILE_RESULT_OUT_OF_MEMORY = 3,
    FFLI_FS_FILE_RESULT_NOT_FOUND = 4
};

struct FFLiFsResult
{
    FFLiFsResult(FFLiFsFileResult fileResult, FSStatus fsStatus) : fileResult(fileResult), fsStatus(fsStatus) {}

    FFLiFsFileResult fileResult;
    FSStatus fsStatus;
};
NN_STATIC_ASSERT(sizeof(FFLiFsResult) == 8);

static inline bool CheckFFLiFsResult(const FFLiFsResult& result)
{
    if (result.fileResult == FFLI_FS_FILE_RESULT_OK && result.fsStatus == FS_STATUS_OK)
        return true;

    return false;
}

static inline FFLResult ConvertFFLiFsResultToFFLResult(const FFLiFsResult& fsResult, FFLResult result)
{
    if (fsResult.fileResult == FFLI_FS_FILE_RESULT_NOT_FOUND)
        return FFL_RESULT_FS_NOT_FOUND;

    if (fsResult.fileResult == FFLI_FS_FILE_RESULT_OUT_OF_MEMORY)
        return FFL_RESULT_OUT_OF_MEMORY;

    return result;
}

static inline bool CheckFileNotFound(FSStatus status)
{
    if (status == FS_STATUS_NOT_FOUND)
        return true;

    if (status == FS_STATUS_PERMISSION_ERROR)
        return true;

    return false;
}

namespace
{

const char* ODB_FILE_NAME = "db/FFL_ODB.dat";

const char* BACKUP_FILE_NAME = "db/FFL_ODB_OLD.dat";

const char* HDB_FILE_NAME = "db/FFL_HDB.dat";

}

FFLiDatabaseFileAccessor::FFLiDatabaseFileAccessor(FFLiFsClient* pClient, FFLiDatabaseFile* pFile,
                                                   FFLiFileWriteBuffer* pWriteBuffer, FFLiAllocator* pAllocator)
    : m_pFsClient(pClient),
      m_FsCommand(pClient),
      m_pDatabaseFile(pFile),
      m_pFileWriteBuffer(pWriteBuffer),
      m_pAllocator(pAllocator),
      _a94(0),
      m_IsPathSet(false),
      m_IsBackupOfficialNeed(false),
      m_IsHiddenSaved(false),
      _d9c(0),
      m_IsSaveHiddenNeeded(false)
{
    std::memset(m_PathOfficial, 0, FFL_PATH_MAX_LEN);
    std::memset(m_PathBackup, 0, FFL_PATH_MAX_LEN);
    std::memset(m_PathHidden, 0, FFL_PATH_MAX_LEN);
}

FFLiDatabaseFileAccessor::~FFLiDatabaseFileAccessor()
{
}

void FFLiDatabaseFileAccessor::Init()
{
    m_pDatabaseFile->official.Init();
    m_pDatabaseFile->hidden.Init();
}

namespace
{

FFLResult GetDatabasePath(char* pDst, u32 size, u64 titleID, const char* filename)
{
    FSStatus status = FFLiGetDatabasePath(pDst, size, titleID, filename);
    return FFLiConvertFSStatusToFFLResult(status);
}

}

FFLResult FFLiDatabaseFileAccessor::AfterConstruct(u64 titleID)
{
    FFLResult result = GetDatabasePath(m_PathOfficial, FFL_PATH_MAX_LEN, titleID, ODB_FILE_NAME);
    if (result != FFL_RESULT_OK)
        return result;

    result = GetDatabasePath(m_PathBackup, FFL_PATH_MAX_LEN, titleID, BACKUP_FILE_NAME);
    if (result != FFL_RESULT_OK)
        return result;

    result = GetDatabasePath(m_PathHidden, FFL_PATH_MAX_LEN, titleID, HDB_FILE_NAME);
    if (result != FFL_RESULT_OK)
        return result;

    m_IsPathSet = true;
    return FFL_RESULT_OK;
}

namespace
{

FFLiFsResult ReadFile(void* pDst, u32 size, FFLiFsCommand* pCommand, const char* pPath)
{
    FFLiFsFile file(pCommand);

    FSStatus status = file.Open(pPath, "r", FS_RET_NOT_FOUND);
    if (status != FS_STATUS_OK)
    {
        FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
        return result;
    }

    status = file.Read(pDst, size, 1);
    if (status == 0)
    {
        file.Close();
        FFLiFsResult result(FFLI_FS_FILE_RESULT_READ_BUFFER_EMPTY, FS_STATUS_OK);
        return result;
    }
    else if (status < 0)
    {
        file.Close();
        FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
        return result;
    }

    status = file.Close();
    FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
    if (!CheckFFLiFsResult(result))
        return result;

    return FFLiFsResult(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);
}

FFLiFsResult LoadDatabaseHidden(FFLiDatabaseFileHidden* pHidden, FFLiFsCommand* pCommand, const char* pPath)
{
    FFLiFsResult result = ReadFile(pHidden, sizeof(FFLiDatabaseFileHidden), pCommand, pPath);
    if (!CheckFFLiFsResult(result))
        return result;

    return FFLiFsResult(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);
}

}

const char* FFLiDatabaseFileAccessor::GetPathHidden() const
{
    return m_PathHidden;
}

namespace
{

FSStatus WriteFileImpl(FFLiFsFile& file, const void* pSrc, u32 size, FFLiFileWriteBuffer* pWriteBuffer)
{
    const u32 count = (size + (FFLI_FILE_WRITE_BUFFER_SIZE - 1)) / FFLI_FILE_WRITE_BUFFER_SIZE;

    FSStatus ret = FS_STATUS_OK;

    for (u32 i = 0; i < count; i++)
    {
        const u32 offset = i * FFLI_FILE_WRITE_BUFFER_SIZE;
        u32 writeSize = FFLiMin(size - offset, FFLI_FILE_WRITE_BUFFER_SIZE);
        std::memcpy(pWriteBuffer, ((const u8*)pSrc) + offset, writeSize);
        ret = file.Write(pWriteBuffer, writeSize, 1);
        if (ret != 1)
            break;
    }

    return ret;
}

FFLiFsResult WriteFile(const void* pSrc, u32 size, FFLiFsCommand* pCommand, FFLiFileWriteBuffer* pWriteBuffer,
                       const char* pPath)
{
    FFLiFsFile file(pCommand);

    FSStatus status = file.Open(pPath, "w", FS_RET_NOT_FOUND | FS_RET_PERMISSION_ERROR);
    if (status != FS_STATUS_OK)
    {
        if (CheckFileNotFound(status))
            return FFLiFsResult(FFLI_FS_FILE_RESULT_NOT_FOUND, FS_STATUS_OK);

        FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
        return result;
    }

    status = WriteFileImpl(file, pSrc, size, pWriteBuffer);
    if (status == 0)
    {
        file.Close();
        FFLiFsResult result(FFLI_FS_FILE_RESULT_WRITE_BUFFER_EMPTY, FS_STATUS_OK);
        return result;
    }
    else if (status < 0)
    {
        file.Close();
        FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
        return result;
    }

    status = file.Close();
    FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
    if (!CheckFFLiFsResult(result))
        return result;

    return FFLiFsResult(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);
}

FFLiFsResult SaveDatabaseHidden(const FFLiDatabaseFileHidden& hidden, FFLiFsCommand* pCommand,
                                FFLiFileWriteBuffer* pWriteBuffer, const char* pPath)
{
    FFLiFsResult result = WriteFile(&hidden, sizeof(FFLiDatabaseFileHidden), pCommand, pWriteBuffer, pPath);
    return result;
}

FFLiFsResult LoadDatabaseOfficial(FFLiDatabaseFileOfficial* pOfficial, FFLiFsCommand* pCommand, const char* pPath)
{
    FFLiFsResult result = ReadFile(pOfficial, sizeof(FFLiDatabaseFileOfficial), pCommand, pPath);
    if (!CheckFFLiFsResult(result))
        return result;

    return FFLiFsResult(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);
}

}

const char* FFLiDatabaseFileAccessor::GetPathOfficial() const
{
    return m_PathOfficial;
}

bool FFLiDatabaseFileAccessor::IsExistFile(const char* pPath)
{
    FFLiFsFile file(&m_FsCommand);

    FSStatus status = file.Open(pPath, "r", FS_RET_NOT_FOUND);
    if (status != FS_STATUS_OK)
        return false;

    status = file.Close();
    FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, status);
    if (!CheckFFLiFsResult(result))
        return false;

    return true;
}

const char* FFLiDatabaseFileAccessor::GetPathBackup() const
{
    return m_PathBackup;
}

namespace
{

FFLiFsResult SaveDatabaseOfficial(const FFLiDatabaseFileOfficial& official, FFLiFsCommand* pCommand,
                                  FFLiFileWriteBuffer* pWriteBuffer, const char* pPath)
{
    FFLiFsResult result = WriteFile(&official, sizeof(FFLiDatabaseFileOfficial), pCommand, pWriteBuffer, pPath);
    return result;
}

}

FFLResult FFLiDatabaseFileAccessor::BootLoadImpl(FFLiFsCommand* pCommand)
{
    FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);

    bool needInitHidden = true;

    result = LoadDatabaseHidden(&m_pDatabaseFile->hidden, pCommand, GetPathHidden());
    if (CheckFFLiFsResult(result))
    {
        if (m_pDatabaseFile->hidden.IsValid())
            needInitHidden = false;
    }
    else if (result.fileResult == FFLI_FS_FILE_RESULT_NOT_FOUND)
    {
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);
    }

    if (needInitHidden)
    {
        m_pDatabaseFile->hidden.Init();
        m_IsFlushQuotaNeeded = true;

        result = SaveDatabaseHidden(m_pDatabaseFile->hidden, pCommand, m_pFileWriteBuffer, GetPathHidden());
        if (!CheckFFLiFsResult(result))
            return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);
    }

    result = LoadDatabaseOfficial(&m_pDatabaseFile->official, pCommand, GetPathOfficial());
    if (CheckFFLiFsResult(result))
    {
        if (m_pDatabaseFile->official.IsValid())
        {
            if (!IsExistFile(GetPathBackup()))
            {
                m_IsFlushQuotaNeeded = true;

                result = SaveDatabaseOfficial(m_pDatabaseFile->official, pCommand, m_pFileWriteBuffer, GetPathBackup());
                if (!CheckFFLiFsResult(result))
                    return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);
            }

            return FFL_RESULT_OK;
        }
    }
    else if (result.fileResult == FFLI_FS_FILE_RESULT_NOT_FOUND)
    {
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);
    }

    result = LoadDatabaseOfficial(&m_pDatabaseFile->official, pCommand, GetPathBackup());
    if (CheckFFLiFsResult(result))
    {
        if (m_pDatabaseFile->official.IsValid())
        {
            m_IsFlushQuotaNeeded = true;

            result = SaveDatabaseOfficial(m_pDatabaseFile->official, pCommand, m_pFileWriteBuffer, GetPathOfficial());
            if (!CheckFFLiFsResult(result))
                return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);

            return FFL_RESULT_OK;
        }
    }
    else if (result.fileResult == FFLI_FS_FILE_RESULT_NOT_FOUND)
    {
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);
    }

    Init();

    m_IsFlushQuotaNeeded = true;

    result = SaveDatabaseOfficial(m_pDatabaseFile->official, pCommand, m_pFileWriteBuffer, GetPathBackup());
    if (!CheckFFLiFsResult(result))
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);

    result = SaveDatabaseOfficial(m_pDatabaseFile->official, pCommand, m_pFileWriteBuffer, GetPathOfficial());
    if (!CheckFFLiFsResult(result))
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_LOAD_ERROR);

    return FFL_RESULT_ODB_EMPTY;
}

FFLResult FFLiDatabaseFileAccessor::AdjustRegularListOfficial()
{
    FFLiDatabaseFileOfficial::AdjustRegularBuffer* pBuffer =
        static_cast<FFLiDatabaseFileOfficial::AdjustRegularBuffer*>(
            m_pAllocator->Allocate(sizeof(FFLiDatabaseFileOfficial::AdjustRegularBuffer)));
    if (pBuffer == NULL)
        return FFL_RESULT_OUT_OF_MEMORY;

    m_pDatabaseFile->official.AdjustRegularList(pBuffer);

    m_pAllocator->Free(pBuffer);
    return FFL_RESULT_OK;
}

FFLResult FFLiDatabaseFileAccessor::BootLoad()
{
    FFLResult result = BootLoadImpl(&m_FsCommand);
    if (result != FFL_RESULT_OK)
        return result;

    return AdjustRegularListOfficial();
}

FFLResult FFLiDatabaseFileAccessor::BeforeDestruct()
{
    if (!m_IsPathSet)
        return FFL_RESULT_FILE_LOAD_ERROR;

    return FFL_RESULT_OK;
}

FFLResult FFLiDatabaseFileAccessor::SaveHidden()
{
    if (!m_pFsClient->IsValid())
        return FFL_RESULT_FS_ERROR;

    m_pDatabaseFile->hidden.UpdateCrc();

    FFLiFsResult result = SaveDatabaseHidden(m_pDatabaseFile->hidden, &m_FsCommand, m_pFileWriteBuffer,
                                             GetPathHidden());
    if (!CheckFFLiFsResult(result))
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_SAVE_ERROR);

    m_IsSaveHiddenNeeded = false;
    m_IsHiddenSaved = true;

    return FFL_RESULT_OK;
}

namespace
{

FFLiFsResult CopyDatabaseOfficial(const char* pPathTo, const char* pPathFrom, FFLiFsCommand* pCommand,
                                  FFLiFileWriteBuffer* pWriteBuffer, FFLiAllocator* pAllocator)
{
    FFLiDatabaseFileOfficial* pOfficial = static_cast<FFLiDatabaseFileOfficial*>(
        pAllocator->Allocate(sizeof(FFLiDatabaseFileOfficial), FS_IO_BUFFER_ALIGN));
    if (pOfficial == NULL)
    {
        FFLiFsResult result(FFLI_FS_FILE_RESULT_OUT_OF_MEMORY, FS_STATUS_OK);
        return result;
    }

    FFLiFsResult result(FFLI_FS_FILE_RESULT_OK, FS_STATUS_OK);

    result = LoadDatabaseOfficial(pOfficial, pCommand, pPathFrom);
    if (CheckFFLiFsResult(result))
        result = SaveDatabaseOfficial(*pOfficial, pCommand, pWriteBuffer, pPathTo);

    pAllocator->Free(pOfficial);
    return result;
}

}

FFLResult FFLiDatabaseFileAccessor::BackupOfficial()
{
    if (!m_pFsClient->IsValid())
        return FFL_RESULT_FS_ERROR;

    const char* pPathFrom = GetPathOfficial();
    const char* pPathTo = GetPathBackup();

    FFLiFsResult result = CopyDatabaseOfficial(pPathTo, pPathFrom, &m_FsCommand, m_pFileWriteBuffer, m_pAllocator);
    if (!CheckFFLiFsResult(result))
        return ConvertFFLiFsResultToFFLResult(result, FFL_RESULT_FILE_SAVE_ERROR);

    return FFL_RESULT_OK;
}

FFLResult FFLiDatabaseFileAccessor::BeforeFlushQuota()
{
    if (m_IsSaveHiddenNeeded)
    {
        FFLResult result = SaveHidden();
        if (result != FFL_RESULT_OK)
            return result;
    }

    if (m_IsBackupOfficialNeed)
    {
        FFLResult result = BackupOfficial();
        if (result != FFL_RESULT_OK)
            return result;
    }

    return FFL_RESULT_OK;
}

FFLResult FFLiDatabaseFileAccessor::FlushQuota(bool force)
{
    u8 flush = m_IsFlushQuotaNeeded || m_IsBackupOfficialNeed || m_IsHiddenSaved;

    if (force | flush)
    {
        if (FFLiFsFile::FlushQuota(&m_FsCommand, m_PathOfficial) != FS_STATUS_OK)
            return FFL_RESULT_FS_ERROR;

        m_IsBackupOfficialNeed = false;
        m_IsHiddenSaved = false;
        m_IsFlushQuotaNeeded = false;
    }

    return FFL_RESULT_OK;
}
