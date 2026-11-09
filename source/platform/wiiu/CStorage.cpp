#include "platform/wiiu/CStorage.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcSaveRegions.hpp"
#endif
#include <coreinit/memdefaultheap.h>
#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <nn/act/client_cpp.h>

extern CProfile* gProfileInstance;

extern "C"
{
void OSFastMutex_Init(OSFastMutex*, const char*);
void OSFastMutex_Lock(OSFastMutex*);
void OSFastMutex_Unlock(OSFastMutex*);
int strcasecmp(const char*, const char*);
void* memalign(unsigned int, unsigned int);

extern "C" int SAVEGetStatEvidence(FSClient*, FSCmdBlock*, unsigned char, const char*, FSStat*,
                                   FSErrorFlag) asm("SAVEGetStat");
}

void stateChangeCallback(FSClient*, FSVolumeState, void*);
extern "C"
{
int MCP_Open();
int MCP_GetSysProdSettings(int, void*);
}
extern "C" void MCP_Close(int);
extern "C" void OSReport(const char*, ...);
extern void altMain();
extern void altMainStop();

static CStorage* s_storage;
static const unsigned int STORAGE_THREAD_STACK_SIZE = 64 * 1024;
static const int STORAGE_THREAD_PRIORITY = 16;
C4JStorage* gStorage = new CStorage();
static char s_wideStringBuffer[1024];
static FSClient s_sciClient;
static FSCmdBlock s_sciCommandBlock;
static unsigned char s_sciBuffer[0x800];
static char s_sciPath[1024];

static inline unsigned int ReadPngChunkWord(const unsigned char* data)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return *(const unsigned int*)data;
#else
    return ((unsigned int)data[0] << 24) | ((unsigned int)data[1] << 16) | ((unsigned int)data[2] << 8) | data[3];
#endif
}

static inline void WritePngChunkWord(unsigned char* data, unsigned int value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *(unsigned int*)data = value;
#else
    data[0] = (unsigned char)(value >> 24);
    data[1] = (unsigned char)(value >> 16);
    data[2] = (unsigned char)(value >> 8);
    data[3] = (unsigned char)value;
#endif
}

void stateChangeCallback(FSClient* client, FSVolumeState state, void*)
{
    if (state != static_cast<FSVolumeState>(1))
    {
        gRenderer->InitialiseContext();
        g_pInput->RequestErrorViewer(1, static_cast<unsigned int>(FSGetLastErrorCodeForViewer(client)), NULL, NULL);
        altMain();
        return;
    }
    g_pInput->DestroyErrorViewer();
    altMainStop();
}

int SCIGetContentPath(char* path, const char* content)
{
    FSDirectoryHandle directory;
    std::sprintf(path, "%s/%s", "vol/storage_mlc01", content);
    int result = FSOpenDir(&s_sciClient, &s_sciCommandBlock, path, &directory, FS_ERROR_FLAG_ALL);
    if (result != 0)
    {
        std::sprintf(path, "%s/%s", "vol/storage_hfiomlc01", content);
        int fallback = FSOpenDir(&s_sciClient, &s_sciCommandBlock, path, &directory, FS_ERROR_FLAG_ALL);
        if (fallback != 0)
        {
            OSReport("SCI ERROR %s(%d): FSOpenDir(%s) failed: %d\n", "SCIGetContentPath", 0x137, path, result);
            return -3;
        }
    }
    FSCloseDir(&s_sciClient, &s_sciCommandBlock, directory, FS_ERROR_FLAG_NONE);
    return 1;
}

int SCIGetISOResource(char* destination, unsigned int destinationSize, const char* resource)
{
    unsigned int length = 0;
    bool inComment = false;
    bool inValue = false;
    char* cursor = destination + 2;
    do
    {
        ++cursor;
    } while (*cursor != '\0');
    if (cursor != destination + 3)
        return 1;

    FSInitCmdBlock(&s_sciCommandBlock);
    FSAddClient(&s_sciClient, FS_ERROR_FLAG_NONE);
    FSStateChangeParams notification = {stateChangeCallback, NULL, NULL};
    FSSetStateChangeNotification(&s_sciClient, &notification);

    int result = SCIGetContentPath(s_sciPath, "sys_title_0005001b_1005c000_cont");
    if (result != 1)
    {
        FSDelClient(&s_sciClient, FS_ERROR_FLAG_ALL);
        return result;
    }
    std::sprintf(s_sciPath, "%s/%s", s_sciPath, resource);

    FSFileHandle file;
    result = FSOpenFile(&s_sciClient, &s_sciCommandBlock, s_sciPath, "r", &file, FS_ERROR_FLAG_ALL);
    if (result < 0)
    {
        OSReport("SCI ERROR %s(%d): FSReadFile(%s) failed\n", "SCIGetISOResource", 0x5a7, s_sciPath);
        FSDelClient(&s_sciClient, FS_ERROR_FLAG_ALL);
        return -3;
    }
    int bytesRead = FSReadFile(&s_sciClient, &s_sciCommandBlock, s_sciBuffer, 1, 0x800, file, 0, FS_ERROR_FLAG_NONE);
    if (bytesRead < 0)
    {
        OSReport("SCI ERROR %s(%d): FSReadFile(%s) failed\n", "SCIGetISOResource", 0x5b4, s_sciPath);
        FSCloseFile(&s_sciClient, &s_sciCommandBlock, file, FS_ERROR_FLAG_NONE);
        FSDelClient(&s_sciClient, FS_ERROR_FLAG_ALL);
        return -1;
    }
    if (bytesRead == 0)
    {
        FSCloseFile(&s_sciClient, &s_sciCommandBlock, file, FS_ERROR_FLAG_NONE);
        FSDelClient(&s_sciClient, FS_ERROR_FLAG_ALL);
        return 1;
    }

    unsigned int offset = 0;
    while (offset < bytesRead)
    {
        char value = s_sciBuffer[offset];
        if (value == '\r' || value == '\n')
        {
            inComment = false;
            ++offset;
            continue;
        }
        if (!inComment)
        {
            if (value == '/' && offset + 1 < bytesRead && s_sciBuffer[offset + 1] == '/')
            {
                inComment = true;
                offset += 2;
                continue;
            }
            if (value == '"' && offset + 3 < bytesRead && s_sciBuffer[offset + 3] == '"')
            {
                unsigned int index = length * 3;
                int remaining = 2;
                do
                {
                    char copied = s_sciBuffer[++offset];
                    destination[index++] = copied;
                    if (copied == '\0')
                        break;
                    --remaining;
                } while (remaining != 0);
                while (remaining-- > 0)
                    destination[index++] = '\0';
                destination[length * 3 + 2] = '\0';
                inValue = true;
                offset += 2;
                continue;
            }
            if (value == ',')
            {
                if (!inValue)
                    destination[length * 3] = '\0';
                ++length;
                inValue = false;
                if (destinationSize / 3 <= length)
                    break;
            }
        }
        ++offset;
    }
    FSCloseFile(&s_sciClient, &s_sciCommandBlock, file, FS_ERROR_FLAG_NONE);
    FSDelClient(&s_sciClient, FS_ERROR_FLAG_ALL);
    return 1;
}

int SCIGetSystemSettings(void* settings)
{
    unsigned char buffer[0x80] = {};
    int handle = MCP_Open();
    if (handle < 0)
    {
        OSReport("SCI ERROR %s(%d): Couldn't get mcp handle %d\n", "SCIGetSystemSettings", 0x83, handle);
        MCP_Close(handle);
        return 0;
    }
    if (MCP_GetSysProdSettings(handle, buffer) != 0)
    {
        OSReport("SCI ERROR %s(%d): MCP Error loading system settings\n", "SCIGetSystemSettings", 0x8c);
        MCP_Close(handle);
        return -1;
    }
    MCP_Close(handle);
    std::memcpy(settings, buffer, 0x46);
    return 1;
}

void SCIGetSystemProdArea(unsigned int* area)
{
    unsigned char settings[0x46] = {};
    SCIGetSystemSettings(settings);
    *area = settings[3];
}

CStorage::DriveMapping::DriveMapping()
{
}

CStorage::DriveMapping::DriveMapping(const std::string& driveName, const std::string& mountedPath)
    : drive(driveName),
      path(mountedPath)
{
}

CStorage::CStorage()
{
    s_storage = this;
    m_crcReady = false;
    m_savingMessageCallback = NULL;
    m_savingMessageContext = NULL;
    DLCOnCreate();
    OSFastMutex_Init(&m_storageMutex, "");
}
void CStorage::DisplaySavingMessages()
{
    C4JStorage::ESaveGameState saveState = GetSaveState();
    int optionsWriting = IsOptionsWriteInProgress();
    if (optionsWriting == 0 && saveState != C4JStorage::ESaveGame_Save)
    {
        if (m_savingMessageCallback != NULL)
            m_savingMessageCallback(m_savingMessageContext, C4JStorage::ESavingMessage_0, 0);
    }
    else if (m_savingMessageCallback != NULL)
        m_savingMessageCallback(m_savingMessageContext, C4JStorage::ESavingMessage_2, 0);
}
void CStorage::Tick()
{
    DisplaySavingMessages();
    TickSaveGame();
    TickDLC();
    TickOptions();
    GenericDataTick();
}
void CStorage::Init(SavingMessageCallback callback, void* context, const char*)
{
    m_savingMessageCallback = callback;
    m_savingMessageContext = context;
}
void CStorage::Init(unsigned int device, const wchar_t* title, const char* prefix, int maxSaves,
                    SavingMessageCallback callback, void* context, const char* extra, bool* exists)
{
    InitSaveGame(device, title, prefix, maxSaves, exists);
    Init(callback, context, extra);
}
void CStorage::SetDefaultImages(unsigned char* icon, unsigned int iconSize, unsigned char* image,
                                unsigned int imageSize, unsigned char* thumbnail, unsigned int thumbnailSize)
{
    SetOptionsIcon(icon, iconSize);
    SetDefaultImages(image, imageSize, thumbnail, thumbnailSize);
}
int CStorage::DoesSaveExist(bool* exists)
{
    *exists = true;
    return 0;
}
int CStorage::EnoughSpaceForAMinSaveGame()
{
    return 1;
}
void CStorage::MakeCRCTable()
{
    unsigned int value;
    int n;
    int k;
    for (n = 0; n < 256; n++)
    {
        value = (unsigned int)n;
        for (k = 0; k < 8; k++)
        {
            if (value & 1)
                value = 0xedb88320U ^ (value >> 1);
            else
                value = value >> 1;
        }
        m_crcTable[n] = value;
    }
    m_crcReady = true;
}
unsigned int CStorage::UpdateCRC(unsigned int crc, unsigned char* data, int length)
{
    unsigned int value = crc;
    int offset;
    if (!m_crcReady)
        MakeCRCTable();
    for (offset = 0; offset < length; offset++)
        value = m_crcTable[(value ^ data[offset]) & 0xff] ^ (value >> 8);
    return value;
}
unsigned int CStorage::CRC(unsigned char* data, int length)
{
    return ~UpdateCRC(FS_ERROR_FLAG_ALL, data, length);
}
void CStorage::DebugPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    va_end(args);
}
void CStorage::SetSaveDeviceSelected(unsigned int, bool)
{
}
int CStorage::GetSaveDeviceSelected(unsigned int)
{
    return 1;
}
void CStorage::ClearDLCOffers()
{
}
int CStorage::ReadTMSFile(int, C4JStorage::eGlobalStorage, C4JStorage::eTMS_FileType, wchar_t*, unsigned char**,
                          unsigned int*, int (*)(void*, wchar_t*, int, bool, C4JStorage::eTMS_FileType), void*, int)
{
    return 0;
}
int CStorage::WriteTMSFile(int, C4JStorage::eGlobalStorage, wchar_t*, unsigned char*, unsigned int)
{
    return 1;
}
int CStorage::DeleteTMSFile(int, C4JStorage::eGlobalStorage, wchar_t*)
{
    return 1;
}
int CStorage::GetDLCOffers(int, DLCOffersCallback, void*, unsigned int)
{
    return 1;
}
void CStorage::InitialiseProfileData(unsigned short version, unsigned int profileCount, unsigned int optionCount,
                                     unsigned int* defaults, int gameDataSize, unsigned int* gameDataSizes)
{
    m_profileVersion = version;
    m_optionDefaults = new unsigned int[optionCount];
    m_profileCount = profileCount;
    m_optionCount = optionCount;
    m_optionsDataCallback = NULL;
    m_optionsDataContext = NULL;
    m_oldProfileVersionCallback = NULL;
    std::memcpy(m_optionDefaults, defaults, optionCount * sizeof(unsigned int));

    for (int profile = 0; profile < 4; ++profile)
    {
        m_profileWriteState[profile].state = 0;
        m_profileWriteState[profile].timer = 0;
        m_profileReadState[profile].state = 0;
        m_profileReadState[profile].reserved = 0;
    }

    m_gameProfileData = (unsigned char*)std::calloc(gameDataSize, 1);
    m_gameProfileStride = gameDataSize / 4;
    *gameDataSizes = 0;
    m_gameProfileSizes = gameDataSizes;

    FSInitCmdBlock(&m_profileCommandBlock);
    FSAddClient(&m_profileClient, FS_ERROR_FLAG_ALL);
    StateChangeNotification notification = {stateChangeCallback, NULL, NULL};
    FSSetStateChangeNotification(&m_profileClient, &notification);

    m_profileInitialised = false;
    m_optionsIcon = NULL;
    m_optionsIconSize = 0;
    m_profileResult = 0;
    m_profileReserved = 0;
}
int CStorage::SetDefaultOptionsCallback(DefaultOptionsCallback callback, void* context)
{
    m_defaultOptionsCallback = callback;
    m_defaultOptionsContext = context;
    return 0;
}
void CStorage::SetOptionsDataCallback(OptionsDataCallback callback, void* context)
{
    m_optionsDataCallback = callback;
    m_optionsDataContext = context;
}
int CStorage::SetOldProfileVersionCallback(OldProfileVersionCallback callback, void* context)
{
    m_oldProfileVersionCallback = callback;
    m_oldProfileVersionContext = context;
    return 0;
}
C4JStorage::PROFILESETTINGS* CStorage::GetDashboardProfileSettings(int profile)
{
    return &m_dashboardProfileSettings[profile];
}
unsigned char* CStorage::GetGameDefinedProfileData(int profile)
{
    return m_gameProfileData + m_gameProfileStride * profile;
}
void CStorage::ReadFromProfile(int profile, int value)
{
    if (profile == -1 || m_profileReadState[profile].state != 0)
        return;
    m_profileReadState[profile].value = value;
    m_profileReadState[profile].state = 5;
}
void CStorage::ClearProfileWriteTimer(int profile)
{
    if (profile != -1)
        m_profileWriteState[profile].timer = 0;
}
void CStorage::WriteToProfile(int profile, bool write, bool force)
{
    if (profile == -1 || !write)
        return;
    if (m_profileWriteState[profile].state == 0)
        m_profileWriteState[profile].state = 1;
    else if (m_profileWriteState[profile].state != 1)
        return;
    if (force)
        ClearProfileWriteTimer(profile);
}
void CStorage::DeleteOptionsData(int)
{
}
void CStorage::ForceQueuedProfileWrites(int profile)
{
    int i;
    if (profile == 255)
    {
        for (i = 0; i < 4; ++i)
            ClearProfileWriteTimer(i);
    }
    else
        ClearProfileWriteTimer(profile);
}
int CStorage::IsOptionsWriteInProgressOrPending(int profile)
{
    if (m_profileWriteState[profile].state == 2 || m_profileWriteState[profile].state == 1)
        return 1;
    return 0;
}
void CStorage::SetOptionsIcon(unsigned char* data, unsigned int size)
{
    m_optionsIconSize = size;
    m_optionsIcon = (unsigned char*)std::malloc(size);
    std::memcpy(m_optionsIcon, data, size);
}
void CStorage::SetOptionDefaults(int profile)
{
    m_dashboardProfileSettings[profile].values[0] = 0;
    m_dashboardProfileSettings[profile].values[1] = 0;
    m_dashboardProfileSettings[profile].values[2] = 0;
    m_dashboardProfileSettings[profile].flags = 0;
    if (m_gameProfileStride > 0)
        std::memset(GetGameDefinedProfileData(profile), 0, m_gameProfileStride);
    if (m_defaultOptionsCallback != NULL)
        m_defaultOptionsCallback(m_defaultOptionsContext, &m_dashboardProfileSettings[profile], profile);
}
std::string CStorage::GetOptionsFilename(int profile)
{
    PlayerUID player(0);
    char filename[128];
    gProfileInstance->GetXUID(profile, &player, false);
    std::sprintf(filename, "options_%ls", player.toString().c_str());
    return std::string(filename);
}
unsigned int CStorage::SaveOptionsThreadProc(int profile)
{
    OSFastMutex_Lock(&m_storageMutex);
    if (!gProfileInstance->IsSignedIn(profile) || gProfileInstance->IsGuest(profile))
    {
        m_profileWriteState[profile].state = 4;
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    gProfileInstance->EnterInitSaveDirCS();
    FSFileHandle handle;
    unsigned char slot = nn::act::GetSlotNo();
    int result = SAVEOpenFile(&m_profileClient, &m_profileCommandBlock, slot, GetOptionsFilename(profile).c_str(), "w",
                              &handle, FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        unsigned int size = m_gameProfileStride + 2;
        result = FSWriteFile(&m_profileClient, &m_profileCommandBlock, m_profileBuffers[profile], 1, size, handle, 0,
                             FS_ERROR_FLAG_ALL);
        FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
        if (result == (int)size
            && SAVEFlushQuota(&m_profileClient, &m_profileCommandBlock, slot, FS_ERROR_FLAG_ALL) == 0)
            m_profileWriteState[profile].state = 3;
        else
        {
            SAVERollbackQuota(&m_profileClient, &m_profileCommandBlock, slot, FS_ERROR_FLAG_ALL);
            m_profileWriteState[profile].state = 4;
        }
    }
    else
    {
        SAVERollbackQuota(&m_profileClient, &m_profileCommandBlock, slot, FS_ERROR_FLAG_ALL);
        m_profileWriteState[profile].state = 4;
    }
    gProfileInstance->LeaveInitSaveDirCS();
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_SaveOptionsThreadProc(int slot, void*)
{
    s_storage->SaveOptionsThreadProc(slot);
    return 0;
}
unsigned int CStorage::LoadOptionsThreadProc(int profile)
{
    OSFastMutex_Lock(&m_storageMutex);
    if (!gProfileInstance->IsSignedIn(profile) || gProfileInstance->IsGuest(profile))
    {
        m_profileReadState[profile].state = 8;
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    gProfileInstance->EnterInitSaveDirCS();
    FSFileHandle handle;
    unsigned char slot = nn::act::GetSlotNo();
    int result = SAVEOpenFile(&m_profileClient, &m_profileCommandBlock, slot, GetOptionsFilename(profile).c_str(), "r",
                              &handle, FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        FSStat stat;
        if (FSGetStatFile(&m_profileClient, &m_profileCommandBlock, handle, &stat, FS_ERROR_FLAG_ALL) == 0)
        {
            unsigned char* buffer = (unsigned char*)MEMAllocFromDefaultHeapEx((m_gameProfileStride + 0x3fU) & ~0x3fU,
                                                                              64);
            m_profileReadBuffers[profile] = buffer;
            FSReadFile(&m_profileClient, &m_profileCommandBlock, buffer, 1, 2, handle, 0, FS_ERROR_FLAG_ALL);
            unsigned short storedVersion = *(unsigned short*)buffer;
            if (storedVersion == m_profileVersion)
            {
                if (FSReadFile(&m_profileClient, &m_profileCommandBlock, buffer, 1, m_gameProfileStride, handle, 0,
                               FS_ERROR_FLAG_ALL)
                    == m_gameProfileStride)
                {
                    m_profileReadState[profile].state = 7;
                    FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
                    gProfileInstance->LeaveInitSaveDirCS();
                    OSFastMutex_Unlock(&m_storageMutex);
                    return 0;
                }
                m_profileUpdateRequired = true;
                FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
            }
            else if (m_oldProfileVersionCallback != NULL)
            {
                std::memset(buffer, 0, m_gameProfileStride);
                int bytesRead = FSReadFile(&m_profileClient, &m_profileCommandBlock, buffer, 1, m_gameProfileStride,
                                           handle, 0, FS_ERROR_FLAG_ALL);
                FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
                if (bytesRead <= m_gameProfileStride)
                {
                    m_oldProfileVersionCallback(m_oldProfileVersionContext, buffer, storedVersion, profile);
                    m_profileReadState[profile].state = 7;
                    gProfileInstance->LeaveInitSaveDirCS();
                    OSFastMutex_Unlock(&m_storageMutex);
                    return 0;
                }
            }
            else
            {
                FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
                MEMFreeToDefaultHeap(buffer);
                m_profileReadState[profile].state = 8;
                gProfileInstance->LeaveInitSaveDirCS();
                OSFastMutex_Unlock(&m_storageMutex);
                return 0;
            }
        }
        else
            FSCloseFile(&m_profileClient, &m_profileCommandBlock, handle, FS_ERROR_FLAG_ALL);
    }
    else if (result != -6)
        m_profileUpdateRequired = true;
    m_profileReadState[profile].state = 8;
    gProfileInstance->LeaveInitSaveDirCS();
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_LoadOptionsThreadProc(int slot, void*)
{
    s_storage->LoadOptionsThreadProc(slot);
    return 0;
}
void CStorage::CheckForProfileUpdatesRequired()
{
    unsigned int now = GetTickCount();
    int profile;
    for (profile = 0; profile < 4; ++profile)
    {
        if (m_profileWriteState[profile].timer != 0 && GetSaveDisabled())
            m_profileWriteState[profile].timer = GetTickCount() + 300000;
        if (m_profileWriteState[profile].state == 1 && now >= m_profileWriteState[profile].timer)
        {
            unsigned int bytes = m_gameProfileStride + 2;
            unsigned char* buffer = (unsigned char*)MEMAllocFromDefaultHeapEx((bytes + 0x3fU) & ~0x3fU, 64);
            m_profileBuffers[profile] = buffer;
            std::memcpy(buffer, &m_profileVersion, 2);
            if (m_gameProfileStride > 0)
                std::memcpy(buffer + 2, m_gameProfileData + m_gameProfileStride * profile, m_gameProfileStride);
            m_profileWriteState[profile].state = 2;
            m_profileWriteState[profile].timer = GetTickCount() + 300000;
            unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                                      + STORAGE_THREAD_STACK_SIZE;
            bool created = C4JThreadImpl::OSCreateThread4J(&m_profileSaveThreads[profile], _SaveOptionsThreadProc,
                                                           profile, this, stackTop, STORAGE_THREAD_STACK_SIZE,
                                                           STORAGE_THREAD_PRIORITY, OS_THREAD_ATTRIB_AFFINITY_ANY);
            if (created)
                OSResumeThread(&m_profileSaveThreads[profile]);
            else
                m_profileWriteState[profile].state = 4;
        }
    }
    for (profile = 0; profile < 4; ++profile)
    {
        if (m_profileReadState[profile].state == 5)
        {
            m_profileReadState[profile].state = 6;
            unsigned char* stack = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE));
            if (C4JThreadImpl::OSCreateThread4J(&m_profileLoadThreads[profile], _LoadOptionsThreadProc, profile, this,
                                                stack + STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_STACK_SIZE,
                                                STORAGE_THREAD_PRIORITY, OS_THREAD_ATTRIB_AFFINITY_ANY))
                OSResumeThread(&m_profileLoadThreads[profile]);
            else
                m_profileWriteState[profile].state = 8;
        }
    }
}
void CStorage::ProcessOptionsUpdate()
{
    int profile;
    for (profile = 0; profile < 4; ++profile)
    {
        if (m_profileReadState[profile].state == 7)
        {
            OSJoinThread(&m_profileLoadThreads[profile], NULL);
            m_profileReadState[profile].state = 0;
            std::memcpy(GetGameDefinedProfileData(profile), m_profileReadBuffers[profile], m_gameProfileStride);
            MEMFreeToDefaultHeap(m_profileReadBuffers[profile]);
            m_profileReadBuffers[profile] = NULL;
            *m_gameProfileSizes |= 1U << profile;
            if (m_optionsDataCallback != NULL)
                m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Read);
        }
        else if (m_profileReadState[profile].state == 8)
        {
            OSJoinThread(&m_profileLoadThreads[profile], NULL);
            m_profileReadState[profile].state = 0;
            MEMFreeToDefaultHeap(m_profileReadBuffers[profile]);
            m_profileReadBuffers[profile] = NULL;
            if (m_optionsDataCallback != NULL)
            {
                if (m_profileUpdateRequired)
                    m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Read_Corrupt);
                else
                    m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Read_Fail);
            }
        }
        else if (m_profileWriteState[profile].state == 3)
        {
            OSJoinThread(&m_profileSaveThreads[profile], NULL);
            m_profileWriteState[profile].state = 0;
            MEMFreeToDefaultHeap(m_profileBuffers[profile]);
            m_profileBuffers[profile] = NULL;
            if (m_optionsDataCallback != NULL)
                m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Write);
        }
        else if (m_profileWriteState[profile].state == 4)
        {
            OSJoinThread(&m_profileSaveThreads[profile], NULL);
            m_profileWriteState[profile].state = 0;
            MEMFreeToDefaultHeap(m_profileBuffers[profile]);
            m_profileBuffers[profile] = NULL;
            if (m_optionsDataCallback != NULL)
            {
                if (m_profileResult == 4)
                {
                    m_profileResult = 0;
                    m_optionsDataCallback(m_optionsDataContext, profile, 0,
                                          C4JStorage::eOptions_Callback_Write_Fail_NoSpace);
                }
                else
                    m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Write_Fail);
            }
        }
    }
}
void CStorage::TickOptions()
{
    CheckForProfileUpdatesRequired();
    ProcessOptionsUpdate();
}
int CStorage::IsOptionsWriteInProgress()
{
    int i;
    for (i = 0; i < 4; ++i)
        if (m_profileWriteState[i].state == 2)
            return 1;
    return 0;
}
unsigned int CStorage::DeleteOptionsThreadProc(int profile)
{
    OSFastMutex_Lock(&m_storageMutex);
    if (gProfileInstance->IsSignedIn(profile) && !gProfileInstance->IsGuest(profile))
    {
        unsigned char slot = nn::act::GetSlotNo();
        int result = SAVERemove(&m_profileClient, &m_profileCommandBlock, slot, GetOptionsFilename(profile).c_str(),
                                FS_ERROR_FLAG_ALL);
        if (result == 0 && m_optionsDataCallback != NULL)
            m_optionsDataCallback(m_optionsDataContext, profile, 0, C4JStorage::eOptions_Callback_Read_CorruptDeleted);
    }
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
void CStorage::SetGameSaveFolderTitle(const wchar_t* title)
{
    std::wcscpy(m_gameSaveFolderTitle, title);
}
void CStorage::SetSaveCacheFolderTitle(const wchar_t* title)
{
    std::wcscpy(m_saveCacheFolderTitle, title);
}
void CStorage::SetOptionsFolderTitle(const wchar_t* title)
{
    std::wcscpy(m_optionsFolderTitle, title);
}
void CStorage::SetCorruptSaveName(const wchar_t* title)
{
    std::wcscpy(m_corruptSaveName, title);
}
void CStorage::SetGameSaveFolderPrefix(const char* prefix)
{
    std::strcpy(m_gameSaveFolderPrefix, prefix);
}
void CStorage::SetMaxSaves(int maxSaves)
{
    m_maxSaves = maxSaves;
}
void CStorage::SetIncompleteSaveCallback(IncompleteSaveCallback callback, void* context)
{
    m_incompleteSaveCallback = callback;
    m_incompleteSaveContext = context;
}
void CStorage::SetSaveDisabled(bool disabled)
{
    m_saveDisabled = disabled;
}
unsigned char CStorage::GetSaveDisabled()
{
    return m_saveDisabled ? 1 : 0;
}
void CStorage::ResetSaveData()
{
    MEMFreeToDefaultHeap(m_saveData);
    m_saveData = NULL;
    m_saveSize = 0;
}
C4JStorage::ESaveGameState CStorage::GetSaveState()
{
    return m_saveState;
}
int CStorage::GetFileNANDSize(unsigned int size)
{
    if (size < 0xa000U)
        return (int)((size + 0x1fffU) & 0xffffe000U);
    if (size < 0x50000U)
        return (int)((size + 0xffffU) & 0xffff0000U);
    return (int)((size + 0x7ffffU) & 0xfff80000U);
}
int CStorage::saveInfoSortfn(const void* left, const void* right)
{
    const SAVE_INFO* first = (const SAVE_INFO*)left;
    const SAVE_INFO* second = (const SAVE_INFO*)right;
    if (first->modifiedTime > second->modifiedTime)
        return -1;
    return first->modifiedTime != second->modifiedTime;
}
unsigned int CStorage::GetSavesInfoThreadProc()
{
    s_storage->DebugPrintf("GetSavesInfoThreadProc - start\n");
    while (m_saveState != C4JStorage::ESaveGame_Idle && m_saveState != C4JStorage::ESaveGame_SaveIncompleteWaiting)
        Sleep(10);

    unsigned char slot = nn::act::GetSlotNo();
    FSDirectoryHandle directory;
    int result = SAVEOpenDir(&m_saveClient, &m_saveDirectoryCommandBlock, slot, "", &directory, FS_ERROR_FLAG_ALL);

    if (m_savesInfo != NULL)
    {
        for (int index = 0; index < m_savesInfo->count; ++index)
            delete[] m_savesInfo->saves[index].thumbnail;
        delete[] m_savesInfo->saves;
        m_savesInfo->count = 0;
        m_savesInfo->saves = NULL;
    }

    m_savesInfo = new SAVE_DETAILS;
    std::memset(m_savesInfo, 0, sizeof(SAVE_DETAILS));
    m_savesInfo->saves = new SAVE_INFO[100];

    if (result == 0)
    {
        FSDirEntry entry;
        while (FSReadDir(&m_saveClient, &m_saveDirectoryCommandBlock, directory, &entry, FS_ERROR_FLAG_ALL) == 0)
        {
            int length = std::strlen(entry.name);
            if (length > 4 && m_savesInfo->count < 100 && (entry.info.flags & 0x80000000U) == 0
                && std::strcmp(entry.name + length - 4, ".ext") == 0)
            {
                FSFileHandle file;
                if (SAVEOpenFile(&m_saveClient, &m_saveDirectoryCommandBlock, slot, entry.name, "r", &file,
                                 FS_ERROR_FLAG_ALL)
                    == 0)
                {
                    unsigned int bytes = entry.info.size;
                    unsigned char* contents = (unsigned char*)MEMAllocFromDefaultHeapEx((bytes + 0x3fU) & ~0x3fU, 64);
                    int read = FSReadFile(&m_saveClient, &m_saveDirectoryCommandBlock, contents, 1, bytes, file, 0,
                                          FS_ERROR_FLAG_ALL);
                    FSCloseFile(&m_saveClient, &m_saveDirectoryCommandBlock, file, FS_ERROR_FLAG_ALL);
                    if (read == (int)bytes)
                    {
                        SAVE_INFO& info = m_savesInfo->saves[m_savesInfo->count++];
                        std::strncpy(info.filename, entry.name, sizeof(info.filename));
                        info.filename[length - 4] = 0;
                        std::memcpy(info.title, contents, 0x100);
                        info.modifiedTime = entry.info.modified;
                        info.thumbnailSize = bytes - 0x100;
                        info.thumbnail = new unsigned char[info.thumbnailSize];
                        std::memcpy(info.thumbnail, (unsigned char*)contents + 0x100, info.thumbnailSize);
                        MEMFreeToDefaultHeap(contents);

                        info.nandSize = GetFileNANDSize(bytes);
                        FSFileHandle saveFile;
                        if (SAVEOpenFile(&m_saveClient, &m_saveDirectoryCommandBlock, slot, info.filename, "r",
                                         &saveFile, FS_ERROR_FLAG_ALL)
                            == 0)
                        {
                            FSStat saveStat;
                            if (FSGetStatFile(&m_saveClient, &m_saveDirectoryCommandBlock, saveFile, &saveStat,
                                              FS_ERROR_FLAG_ALL)
                                == 0)
                                info.nandSize += GetFileNANDSize(saveStat.size);
                            FSCloseFile(&m_saveClient, &m_saveDirectoryCommandBlock, saveFile, FS_ERROR_FLAG_ALL);
                        }
                        m_savesInfo->nandSize += info.nandSize;
                    }
                }
            }
        }
        std::qsort(m_savesInfo->saves, m_savesInfo->count, sizeof(SAVE_INFO), saveInfoSortfn);
        m_savesInfoState = 2;
        s_storage->DebugPrintf("GetSavesInfoThreadProc - complete\n");
        FSCloseDir(&m_saveClient, &m_saveDirectoryCommandBlock, directory, FS_ERROR_FLAG_ALL);
    }
    else
    {
        m_savesInfoState = 0;
        s_storage->DebugPrintf("GetSavesInfoThreadProc - ret != FS_STATUS_OK (%d)\n", result);
        m_savesInfoState = 2;
    }
    return 0;
}
int CStorage::_GetSavesInfoThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->GetSavesInfoThreadProc();
}
int CStorage::GetSavesInfo(int, SavesInfoCallback callback, void* context, const char*)
{
    m_savesInfoCallback = callback;
    m_savesInfoAvailable = false;
    m_savesInfoContext = context;
    if (m_savesInfoState == 0)
    {
        m_savesInfoState = 1;
        unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                                  + STORAGE_THREAD_STACK_SIZE;
        bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[2], _GetSavesInfoThreadProc, 0, this, stackTop,
                                                       STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                       OS_THREAD_ATTRIB_AFFINITY_ANY);
        if (created)
            OSResumeThread(&m_saveThreads[2]);
        else
            m_savesInfoState = 0;
    }
    return 0;
}
SAVE_DETAILS* CStorage::ReturnSavesInfo()
{
    return m_savesInfoAvailable ? m_savesInfo : NULL;
}
void CStorage::ClearSavesInfo()
{
    m_savesInfoAvailable = false;
    if (m_savesInfo != NULL)
    {
        if (m_savesInfo->saves != NULL)
        {
            delete[] m_savesInfo->saves;
            m_savesInfo->saves = NULL;
            m_savesInfo->count = 0;
        }
        delete m_savesInfo;
        m_savesInfo = NULL;
    }
}
C4JStorage::ESaveGameState CStorage::LoadSaveDataThumbnail(SAVE_INFO* info,
                                                           int (*callback)(void*, unsigned char*, unsigned int),
                                                           void* context)
{
    callback(context, info->thumbnail, info->thumbnailSize);
    return C4JStorage::ESaveGame_GetThumbnail;
}
unsigned int CStorage::LoadGameThreadProc()
{
    gProfileInstance->EnterInitSaveDirCS();
    unsigned char slot = nn::act::GetSlotNo();
    FSFileHandle file;
    FSStat stat;
    if (SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, "r", &file, FS_ERROR_FLAG_ALL) == 0
        && FSGetStatFile(&m_saveClient, &m_saveCommandBlock, file, &stat, FS_ERROR_FLAG_ALL) == 0)
    {
        MEMFreeToDefaultHeap(m_saveData);
        m_saveData = (unsigned char*)MEMAllocFromDefaultHeapEx((stat.size + 0x3fU) & ~0x3fU, 64);
        m_saveSize = FSReadFile(&m_saveClient, &m_saveCommandBlock, m_saveData, 1, stat.size, file, 0,
                                FS_ERROR_FLAG_ALL);
        if (m_saveSize == stat.size)
        {
            FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
            m_saveState = C4JStorage::ESaveGame_LoadComplete;
            gProfileInstance->LeaveInitSaveDirCS();
            return 0;
        }
    }
    gProfileInstance->LeaveInitSaveDirCS();
    m_saveState = C4JStorage::ESaveGame_LoadFailed;
    return 0;
}
int CStorage::_LoadGameThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->LoadGameThreadProc();
}
C4JStorage::ESaveGameState CStorage::LoadSaveData(SAVE_INFO* info, LoadCallback callback, void* context)
{
    m_loadCallback = callback;
    m_loadCallbackContext = context;
    if (m_saveState != C4JStorage::ESaveGame_Idle)
        return C4JStorage::ESaveGame_Idle;
    SetSaveUniqueFilename(info->filename);
    std::memcpy(m_saveTitle, info->title, 0x100);
    m_saveState = C4JStorage::ESaveGame_Load;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[1], _LoadGameThreadProc, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_saveThreads[1]);
        return C4JStorage::ESaveGame_Load;
    }
    m_saveState = C4JStorage::ESaveGame_Idle;
    return C4JStorage::ESaveGame_Idle;
}
int CStorage::GetSaveSize()
{
    return (int)m_saveSize;
}
void CStorage::GetSaveData(void* destination, unsigned int* size)
{
    if (destination != NULL)
    {
        std::memcpy(destination, m_saveData, m_saveSize);
        *size = m_saveSize;
    }
    else
        *size = 0;
}
int CStorage::GetSaveUniqueNumber(int*)
{
    return 1;
}
int CStorage::GetSaveUniqueFilename(char*)
{
    return 0;
}
void CStorage::SetSaveUniqueFilename(char* filename)
{
    std::strcpy(m_saveUniqueFilename, filename);
}
int CStorage::GetSaveUniqueFileDir(char* filename)
{
    std::strcpy(filename, m_saveUniqueFilename);
    return 1;
}
void CStorage::CreateSaveUniqueName()
{
    OSCalendarTime calendar;
    char suffix[20];
    OSTicksToCalendarTime(OSGetTime(), &calendar);
    std::sprintf(suffix, "%2d%02d%02d%02d%02d%02d", calendar.tm_year % 100, calendar.tm_mon, calendar.tm_mday,
                 calendar.tm_hour, calendar.tm_min, calendar.tm_sec);
    std::strcpy(m_saveUniqueFilename, m_gameSaveFolderPrefix);
    std::strcat(m_saveUniqueFilename, suffix);
}
void CStorage::SetSaveTitle(const wchar_t* title)
{
    CreateSaveUniqueName();
#if defined(RECON_NATIVE)
    std::memset(m_saveTitle, 0, sizeof(m_saveTitle));
    const size_t titleLength = std::min(std::wcslen(title), sizeof(m_saveTitle) / sizeof(wchar_t) - 1);
    std::memcpy(m_saveTitle, title, titleLength * sizeof(wchar_t));
#else
    std::memcpy(m_saveTitle, title, 0x100);
#endif
}
#ifdef MINECRAFT_PC
const wchar_t* CStorage::GetSaveTitle()
{
    return m_saveTitle;
}
#endif
void* CStorage::AllocateSaveData(unsigned int size)
{
    unsigned int allocSize = (size + 0x3fU) & ~0x3fU;
    MEMFreeToDefaultHeap(m_saveData);
    m_saveData = (unsigned char*)MEMAllocFromDefaultHeapEx(allocSize, 64);
    if (m_saveData != NULL)
        m_saveSize = size;
    return m_saveData;
}
void CStorage::SetSaveDataSize(unsigned int size)
{
    m_saveSize = size;
}
void CStorage::GetDefaultSaveImage(unsigned char** data, unsigned int* size)
{
    *data = m_defaultImage;
    *size = m_defaultImageSize;
}
void CStorage::GetDefaultSaveThumbnail(unsigned char** data, unsigned int* size)
{
    *data = m_defaultThumbnail;
    *size = m_defaultThumbnailSize;
}
void CStorage::AddTextFieldToPNG(unsigned char* data, unsigned int imageSize, unsigned char* text,
                                 unsigned int textSize, unsigned int)
{
    if (imageSize == 0 || data[0] != 0x89 || data[1] != 'P' || data[2] != 'N' || data[3] != 'G' || data[4] != 13
        || data[5] != 10 || data[6] != 26 || data[7] != 10)
        return;

    unsigned int offset = 8;
    while (offset < imageSize)
    {
        unsigned int chunkStart = offset;
        unsigned int length = ReadPngChunkWord(data + offset);
        offset += 4;
        unsigned int type = ReadPngChunkWord(data + offset);
        offset += 4;
        if (type == PNG_CHUNK_TYPE_IEND)
        {
            offset = chunkStart;
            WritePngChunkWord(data + offset, textSize);
            offset += 4;
            unsigned char* textChunk = data + offset;
            WritePngChunkWord(data + offset, PNG_CHUNK_TYPE_TEXT);
            offset += 4;
            std::memcpy(data + offset, text, textSize);
            offset += textSize;
            WritePngChunkWord(data + offset, s_storage->CRC(textChunk, textSize + 4));
            offset += 4;
            WritePngChunkWord(data + offset, 0);
            offset += 4;
            unsigned char* endChunk = data + offset;
            WritePngChunkWord(data + offset, PNG_CHUNK_TYPE_IEND);
            offset += 4;
            WritePngChunkWord(data + offset, s_storage->CRC(endChunk, 4));
            offset += 4;
        }
        else
        {
            offset += length;
            offset += 4;
        }
    }
}
void CStorage::SetSaveImages(unsigned char* image, unsigned int imageSize, unsigned char* thumbnail,
                             unsigned int thumbnailSize, unsigned char* text, unsigned int textSize)
{
    unsigned int size = imageSize;
    if (m_saveImage != NULL)
        std::free(m_saveImage);
    m_saveThumbnail = thumbnail;
    m_saveThumbnailSize = thumbnailSize;
    if (textSize != 0)
        size += textSize + 12;
    m_saveImage = (unsigned char*)std::malloc(size);
    m_saveImageSize = size;
    std::memset(m_saveImage, 0, size);
    std::memcpy(m_saveImage, image, imageSize);
    if (textSize != 0)
        AddTextFieldToPNG(m_saveImage, imageSize, text, textSize, size);
}
unsigned int CStorage::SaveGameThreadProc()
{
    if (m_savesInfo == NULL)
    {
        m_savesInfo = new SAVE_DETAILS;
        std::memset(m_savesInfo, 0, sizeof(SAVE_DETAILS));
    }
    char metadataName[256];
    std::snprintf(metadataName, sizeof(metadataName), "%s.ext", m_saveUniqueFilename);
    unsigned char slot = nn::act::GetSlotNo();
    FSStat stat;
    unsigned int replacedSize = 0;
    bool hadSave = false;
    bool hadMetadata = false;
    if (SAVEGetStatEvidence(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, &stat, FS_ERROR_FLAG_ALL)
        == 0)
    {
        hadSave = true;
        replacedSize = GetFileNANDSize(stat.size);
    }
    if (SAVEGetStatEvidence(&m_saveClient, &m_saveCommandBlock, slot, metadataName, &stat, FS_ERROR_FLAG_ALL) == 0)
    {
        hadMetadata = true;
        replacedSize += GetFileNANDSize(stat.size);
    }
    unsigned int requiredSize = GetFileNANDSize(m_saveSize) + GetFileNANDSize(m_saveImageSize + 0x100);
    if (m_savesInfo->nandSize + requiredSize - replacedSize > SAVE_DATA_QUOTA)
    {
        m_saveState = C4JStorage::ESaveGame_SaveIncomplete;
        m_incompleteReason = 1;
        return 0;
    }
    OSFastMutex_Lock(&m_storageMutex);
    gProfileInstance->EnterInitSaveDirCS();
    if (hadSave)
        SAVERemove(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, FS_ERROR_FLAG_ALL);
    if (hadMetadata)
        SAVERemove(&m_saveClient, &m_saveCommandBlock, slot, metadataName, FS_ERROR_FLAG_ALL);
    FSFileHandle file;
    int result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, "w", &file,
                              FS_ERROR_FLAG_ALL);
    if (result == FS_STATUS_STORAGE_FULL)
    {
        m_incompleteReason = 1;
        m_saveState = C4JStorage::ESaveGame_SaveIncomplete;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    if (result != 0)
    {
        m_saveState = C4JStorage::ESaveGame_SaveFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    int written = FSWriteFile(&m_saveClient, &m_saveCommandBlock, m_saveData, 1, m_saveSize, file, 0,
                              FS_ERROR_FLAG_ALL);
    if (written == FS_STATUS_STORAGE_FULL)
    {
        m_saveState = C4JStorage::ESaveGame_SaveIncomplete;
        m_incompleteReason = 1;
        FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    if (written != (int)m_saveSize)
    {
        m_saveState = C4JStorage::ESaveGame_SaveFailed;
        FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
    unsigned int metadataSize = m_saveImageSize + 0x100;
    unsigned char* metadata = (unsigned char*)MEMAllocFromDefaultHeapEx((metadataSize + 0x3fU) & ~0x3fU, 64);
    std::memcpy(metadata, m_saveTitle, 0x100);
    std::memcpy(metadata + 0x100, m_saveImage, m_saveImageSize);
    result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, metadataName, "w", &file, FS_ERROR_FLAG_ALL);
    if (result == FS_STATUS_STORAGE_FULL)
    {
        m_incompleteReason = 1;
        m_saveState = C4JStorage::ESaveGame_SaveIncomplete;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    if (result != 0)
    {
        m_saveState = C4JStorage::ESaveGame_SaveFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    written = FSWriteFile(&m_saveClient, &m_saveCommandBlock, metadata, 1, metadataSize, file, 0, FS_ERROR_FLAG_ALL);
    std::free(metadata);
    FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
    if (written == FS_STATUS_STORAGE_FULL)
    {
        m_incompleteReason = 1;
        m_saveState = C4JStorage::ESaveGame_SaveIncomplete;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    if (written != (int)metadataSize
        || SAVEFlushQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL) != 0)
    {
        m_saveState = C4JStorage::ESaveGame_SaveFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        gProfileInstance->LeaveInitSaveDirCS();
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    gProfileInstance->LeaveInitSaveDirCS();
    OSFastMutex_Unlock(&m_storageMutex);
    if (m_savesInfo != NULL)
        m_savesInfo->nandSize += requiredSize - replacedSize;
    m_saveState = C4JStorage::ESaveGame_SaveComplete;
    return 0;
}
int CStorage::_SaveGameThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->SaveGameThreadProc();
}
C4JStorage::ESaveGameState CStorage::SaveSaveData(SaveCallback callback, void* context)
{
    m_saveCallback = callback;
    m_saveCallbackContext = context;
    if (m_saveState != C4JStorage::ESaveGame_Idle && m_saveState != C4JStorage::ESaveGame_SaveIncompleteWaiting)
        return C4JStorage::ESaveGame_Idle;
    m_saveState = C4JStorage::ESaveGame_Save;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[0], _SaveGameThreadProc, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_saveThreads[0]);
        return C4JStorage::ESaveGame_Save;
    }
    m_saveState = C4JStorage::ESaveGame_Idle;
    return C4JStorage::ESaveGame_Idle;
}
void CStorage::ContinueIncompleteOperation()
{
    if (m_saveState == C4JStorage::ESaveGame_SaveIncompleteWaiting)
        SaveSaveData(m_saveCallback, m_saveCallbackContext);
}
void CStorage::CancelIncompleteOperation()
{
    if (m_saveState == C4JStorage::ESaveGame_SaveIncompleteWaiting)
        m_saveState = C4JStorage::ESaveGame_SaveCancelled;
}
unsigned int CStorage::DeleteGameThreadProc()
{
    char metadataName[256];
    std::snprintf(metadataName, 128, "%s.ext", m_selectedSave->filename);
    OSFastMutex_Lock(&m_storageMutex);
    unsigned char slot = nn::act::GetSlotNo();
    int result = SAVERemove(&m_saveClient, &m_saveCommandBlock, slot, m_selectedSave->filename, FS_ERROR_FLAG_ALL);
    if (result == 0)
        result = SAVERemove(&m_saveClient, &m_saveCommandBlock, slot, metadataName, FS_ERROR_FLAG_ALL);
    if (result == 0)
        result = SAVEFlushQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        OSFastMutex_Unlock(&m_storageMutex);
#ifdef MINECRAFT_PC
        PcSaveRegionsDelete(m_selectedSave->filename);
#endif
        m_savesInfo->nandSize -= m_selectedSave->nandSize;
        m_selectedSave->nandSize = 0;
        m_saveState = C4JStorage::ESaveGame_DeleteComplete;
    }
    else
    {
        m_saveState = C4JStorage::ESaveGame_DeleteFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
    }
    return 0;
}
int CStorage::_DeleteGameThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->DeleteGameThreadProc();
}
C4JStorage::ESaveGameState CStorage::DeleteSaveData(SAVE_INFO* info, SaveCallback callback, void* context)
{
    C4JStorage::ESaveGameState oldState = m_saveState;
    m_deleteCallback = callback;
    m_deleteCallbackContext = context;
    m_selectedSave = info;
    if (oldState != C4JStorage::ESaveGame_Idle && oldState != C4JStorage::ESaveGame_SaveIncompleteWaiting)
        return C4JStorage::ESaveGame_Idle;
    m_saveState = C4JStorage::ESaveGame_Delete;
    m_previousSaveState = oldState;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[3], _DeleteGameThreadProc, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_saveThreads[3]);
        return C4JStorage::ESaveGame_Delete;
    }
    m_saveState = m_previousSaveState;
    return C4JStorage::ESaveGame_Idle;
}
unsigned int CStorage::CopyGameThreadProc()
{
    unsigned char slot = nn::act::GetSlotNo();
    OSFastMutex_Lock(&m_storageMutex);

    FSFileHandle sourceFile;
    FSStat sourceStat;
    int result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, "r", &sourceFile,
                              FS_ERROR_FLAG_ALL);
    if (result != 0
        || FSGetStatFile(&m_saveClient, &m_saveCommandBlock, sourceFile, &sourceStat, FS_ERROR_FLAG_ALL) != 0)
    {
        m_saveState = C4JStorage::ESaveGame_CopyFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }

    unsigned char* save = (unsigned char*)MEMAllocFromDefaultHeapEx((sourceStat.size + 0x3fU) & ~0x3fU, 64);
    int saveSize = FSReadFile(&m_saveClient, &m_saveCommandBlock, save, 1, sourceStat.size, sourceFile, 0,
                              FS_ERROR_FLAG_ALL);
    if (saveSize != (int)sourceStat.size)
    {
        MEMFreeToDefaultHeap(save);
        m_saveState = C4JStorage::ESaveGame_CopyFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    FSCloseFile(&m_saveClient, &m_saveCommandBlock, sourceFile, FS_ERROR_FLAG_ALL);

    char metadataName[128];
    std::snprintf(metadataName, sizeof(metadataName), "%s.ext", m_saveUniqueFilename);
    FSFileHandle metadataFile;
    FSStat metadataStat;
    result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, metadataName, "r", &metadataFile,
                          FS_ERROR_FLAG_ALL);
    if (result != 0
        || FSGetStatFile(&m_saveClient, &m_saveCommandBlock, metadataFile, &metadataStat, FS_ERROR_FLAG_ALL) != 0)
    {
        m_saveState = C4JStorage::ESaveGame_CopyFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }

    unsigned char* metadata = (unsigned char*)MEMAllocFromDefaultHeapEx((metadataStat.size + 0x3fU) & ~0x3fU, 64);
    int metadataSize = FSReadFile(&m_saveClient, &m_saveCommandBlock, metadata, 1, metadataStat.size, metadataFile, 0,
                                  FS_ERROR_FLAG_ALL);
    if (metadataSize != (int)metadataStat.size)
    {
        MEMFreeToDefaultHeap(save);
        MEMFreeToDefaultHeap(metadata);
        FSCloseFile(&m_saveClient, &m_saveCommandBlock, metadataFile, FS_ERROR_FLAG_ALL);
        m_saveState = C4JStorage::ESaveGame_CopyFailed;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }
    FSCloseFile(&m_saveClient, &m_saveCommandBlock, metadataFile, FS_ERROR_FLAG_ALL);

    if (m_savesInfo->nandSize + GetFileNANDSize(saveSize) + GetFileNANDSize(metadataSize) > SAVE_DATA_QUOTA)
    {
        MEMFreeToDefaultHeap(save);
        MEMFreeToDefaultHeap(metadata);
        m_saveState = C4JStorage::ESaveGame_CopyFailedNoSpace;
        SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
        OSFastMutex_Unlock(&m_storageMutex);
        return 0;
    }

#ifdef MINECRAFT_PC
    char sourceSaveName[sizeof(m_saveUniqueFilename)];
    std::strcpy(sourceSaveName, m_saveUniqueFilename);
#endif
    CreateSaveUniqueName();
    FSFileHandle destinationFile;
    result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, m_saveUniqueFilename, "w", &destinationFile,
                          FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        result = FSWriteFile(&m_saveClient, &m_saveCommandBlock, save, 1, saveSize, destinationFile, 0,
                             FS_ERROR_FLAG_ALL);
        if (result != saveSize)
        {
            MEMFreeToDefaultHeap(save);
            MEMFreeToDefaultHeap(metadata);
            FSCloseFile(&m_saveClient, &m_saveCommandBlock, destinationFile, FS_ERROR_FLAG_ALL);
            m_saveState = result == FS_STATUS_STORAGE_FULL ? C4JStorage::ESaveGame_CopyFailedNoSpace
                                                           : C4JStorage::ESaveGame_CopyFailed;
            SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
            OSFastMutex_Unlock(&m_storageMutex);
            return 0;
        }

        FSCloseFile(&m_saveClient, &m_saveCommandBlock, destinationFile, FS_ERROR_FLAG_ALL);
        std::snprintf(metadataName, sizeof(metadataName), "%s.ext", m_saveUniqueFilename);
        result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, metadataName, "w", &destinationFile,
                              FS_ERROR_FLAG_ALL);
        if (result == 0)
        {
            result = FSWriteFile(&m_saveClient, &m_saveCommandBlock, metadata, 1, metadataSize, destinationFile, 0,
                                 FS_ERROR_FLAG_ALL);
            MEMFreeToDefaultHeap(save);
            MEMFreeToDefaultHeap(metadata);
            FSCloseFile(&m_saveClient, &m_saveCommandBlock, destinationFile, FS_ERROR_FLAG_ALL);
            if (result == metadataSize)
            {
                if (SAVEFlushQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL) == 0)
                {
                    OSFastMutex_Unlock(&m_storageMutex);
#ifdef MINECRAFT_PC
                    PcSaveRegionsCopy(sourceSaveName, m_saveUniqueFilename);
#endif
                    m_saveState = C4JStorage::ESaveGame_CopyComplete;
                    return 0;
                }
                SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
                OSFastMutex_Unlock(&m_storageMutex);
                m_saveState = C4JStorage::ESaveGame_CopyFailed;
                return 0;
            }
        }
        else
        {
            MEMFreeToDefaultHeap(save);
            MEMFreeToDefaultHeap(metadata);
        }
    }
    else
    {
        MEMFreeToDefaultHeap(save);
        MEMFreeToDefaultHeap(metadata);
    }

    m_saveState = result == FS_STATUS_STORAGE_FULL ? C4JStorage::ESaveGame_CopyFailedNoSpace
                                                   : C4JStorage::ESaveGame_CopyFailed;
    SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_CopyGameThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->CopyGameThreadProc();
}
C4JStorage::ESaveGameState CStorage::CopySaveData(SAVE_INFO* info, CopyCallback callback,
                                                  CopyProgressCallback progressCallback, void* context)
{
    m_copyCallback = callback;
    m_copyProgressCallback = progressCallback;
    m_copyCallbackContext = context;
    if (m_saveState != C4JStorage::ESaveGame_Idle)
        return C4JStorage::ESaveGame_Idle;
    SetSaveUniqueFilename(info->filename);
    std::memcpy(m_saveTitle, info->title, 0x100);
    m_saveState = C4JStorage::ESaveGame_Copy;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[5], _CopyGameThreadProc, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_saveThreads[5]);
        return C4JStorage::ESaveGame_Copy;
    }
    m_saveState = C4JStorage::ESaveGame_Idle;
    return C4JStorage::ESaveGame_Idle;
}
unsigned int CStorage::RenameGameThreadProc()
{
    char metadataName[128];
    std::snprintf(metadataName, sizeof(metadataName), "%s.ext", m_savesInfo->saves[m_renameSaveIndex].filename);
    OSFastMutex_Lock(&m_storageMutex);
    unsigned char slot = nn::act::GetSlotNo();
    FSFileHandle file;
    int result = SAVEOpenFile(&m_saveClient, &m_saveCommandBlock, slot, metadataName, "r+", &file, FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        unsigned char* title = (unsigned char*)MEMAllocFromDefaultHeapEx(0x100, 64);
        std::memcpy(title, m_renameTitle, 0x100);
        result = FSWriteFile(&m_saveClient, &m_saveCommandBlock, title, 1, 0x100, file, 0, FS_ERROR_FLAG_ALL);
        FSCloseFile(&m_saveClient, &m_saveCommandBlock, file, FS_ERROR_FLAG_ALL);
        MEMFreeToDefaultHeap(title);
        if (result == 0x100 && SAVEFlushQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL) == 0)
        {
            OSFastMutex_Unlock(&m_storageMutex);
            std::memcpy(m_savesInfo->saves[m_renameSaveIndex].title, m_renameTitle, 0x100);
            m_saveState = C4JStorage::ESaveGame_RenameComplete;
            return 0;
        }
    }
    m_saveState = C4JStorage::ESaveGame_RenameFailed;
    SAVERollbackQuota(&m_saveClient, &m_saveCommandBlock, slot, FS_ERROR_FLAG_ALL);
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_RenameGameThreadProc(int, void* storage)
{
    return ((CStorage*)storage)->RenameGameThreadProc();
}
C4JStorage::ESaveGameState CStorage::RenameSaveData(int index, wchar_t* title, SaveCallback callback, void* context)
{
    m_renameCallbackContext = context;
    m_renameCallback = callback;
    m_renameSaveIndex = index;
    std::memcpy(m_renameTitle, title, 0x100);
    if (m_saveState != C4JStorage::ESaveGame_Idle)
        return C4JStorage::ESaveGame_Idle;
    m_saveState = C4JStorage::ESaveGame_Rename;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_saveThreads[4], _RenameGameThreadProc, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_saveThreads[4]);
        return C4JStorage::ESaveGame_Rename;
    }
    m_saveState = C4JStorage::ESaveGame_Idle;
    return C4JStorage::ESaveGame_Idle;
}
void CStorage::GetSaveImage(unsigned char**, int*)
{
}
void CStorage::GetSaveThumbnail(unsigned char**, int*)
{
}
void CStorage::InitSaveGame(unsigned int, const wchar_t*, const char*, int, bool* exists)
{
    SAVEInit();
    m_saveState = C4JStorage::ESaveGame_Idle;
    std::memset(m_saveUniqueFilename, 0, 32);
    std::memset(m_saveTitle, 0, 0x100);
    std::memset(m_gameSaveFolderTitle, 0, 0x100);
    std::memset(m_saveCacheFolderTitle, 0, 0x100);
    std::memset(m_optionsFolderTitle, 0, 0x100);
    std::memset(m_gameSaveFolderPrefix, 0, 11);

    m_loadCallback = NULL;
    m_savesInfoCallback = NULL;
    m_maxSaves = 99;
    m_saveCallbackContext = NULL;
    m_savesInfo = NULL;
    m_saveImageSize = 0;
    m_saveSize = 0;
    m_copyCallbackContext = NULL;
    m_renameCallback = NULL;
    m_saveThumbnail = NULL;
    m_incompleteReason = 0;
    m_deleteCallback = NULL;
    m_savesInfoState = 0;
    m_saveCallback = NULL;
    m_copyProgressCallback = NULL;
    m_deleteCallbackContext = NULL;
    m_saveData = NULL;
    m_saveImage = NULL;
    m_copyCallback = NULL;
    m_savesInfoContext = NULL;
    m_savesInfoAvailable = false;
    m_incompleteSaveCallback = NULL;
    m_saveThumbnailSize = 0;
    m_loadCallbackContext = NULL;
    m_saveDisabled = false;
    m_incompleteSaveContext = NULL;
    m_renameCallbackContext = NULL;

    FSInitCmdBlock(&m_saveCommandBlock);
    FSInitCmdBlock(&m_saveDirectoryCommandBlock);
    FSAddClient(&m_saveClient, FS_ERROR_FLAG_ALL);

    StateChangeNotification notification = {stateChangeCallback, NULL, NULL};
    FSSetStateChangeNotification(&m_saveClient, &notification);

    unsigned char slot = nn::act::GetSlotNo();
    int result = SAVEInitSaveDir(slot);
    if (exists != 0)
        *exists = result == -12;
}
void CStorage::SetDefaultImages(unsigned char* image, unsigned int imageSize, unsigned char* thumbnail,
                                unsigned int thumbnailSize)
{
    m_defaultImageSize = imageSize;
    m_defaultImage = (unsigned char*)std::malloc(imageSize);
    std::memcpy(m_defaultImage, image, imageSize);
    m_defaultThumbnailSize = thumbnailSize;
    m_defaultThumbnail = (unsigned char*)std::malloc(thumbnailSize);
    std::memcpy(m_defaultThumbnail, thumbnail, thumbnailSize);
}
void CStorage::TickSaveGame()
{
    C4JStorage::ESaveGameState state = m_saveState;

    if (state == C4JStorage::ESaveGame_SaveComplete)
    {
        OSJoinThread(&m_saveThreads[0], NULL);
        m_saveState = C4JStorage::ESaveGame_Idle;
        state = C4JStorage::ESaveGame_Idle;

        if (m_saveCallback != NULL)
        {
            m_saveCallback(m_saveCallbackContext, true);
            state = m_saveState;
        }
    }
    else if (state == C4JStorage::ESaveGame_SaveFailed || state == C4JStorage::ESaveGame_SaveCancelled)
    {
        if (state == C4JStorage::ESaveGame_SaveFailed)
        {
            OSJoinThread(&m_saveThreads[0], NULL);
        }

        if (m_saveCallback != NULL)
        {
            m_saveCallback(m_saveCallbackContext, false);
        }

        m_saveState = C4JStorage::ESaveGame_Idle;
        state = C4JStorage::ESaveGame_Idle;
    }

    if (state == C4JStorage::ESaveGame_LoadComplete || state == C4JStorage::ESaveGame_LoadFailed)
    {
        OSJoinThread(&m_saveThreads[1], NULL);

        if (m_loadCallback != NULL)
        {
            m_loadCallback(m_loadCallbackContext, state == C4JStorage::ESaveGame_LoadFailed, true);
        }

        m_saveState = C4JStorage::ESaveGame_Idle;
        state = C4JStorage::ESaveGame_Idle;
    }

    if (m_savesInfoState == 2)
    {
        OSJoinThread(&m_saveThreads[2], NULL);
        m_savesInfoAvailable = true;

        if (m_savesInfoCallback != NULL)
        {
            m_savesInfoCallback(m_saveCallbackContext, m_savesInfo, true);
        }

        state = m_saveState;
        m_savesInfoState = 0;
    }

    if (state == C4JStorage::ESaveGame_DeleteComplete || state == C4JStorage::ESaveGame_DeleteFailed)
    {
        OSJoinThread(&m_saveThreads[3], NULL);

        if (state == C4JStorage::ESaveGame_DeleteComplete)
        {
            int index = (int)(m_selectedSave - m_savesInfo->saves);
            for (int i = index; i < m_savesInfo->count - 1; ++i)
            {
                m_savesInfo->saves[i] = m_savesInfo->saves[i + 1];
            }
            --m_savesInfo->count;
        }

        if (m_deleteCallback != NULL)
        {
            m_deleteCallback(m_deleteCallbackContext, true);
        }

        m_saveState = m_previousSaveState;
        state = m_saveState;
    }

    if (state == C4JStorage::ESaveGame_RenameComplete || state == C4JStorage::ESaveGame_RenameFailed)
    {
        OSJoinThread(&m_saveThreads[4], NULL);

        if (m_renameCallback != NULL)
        {
            m_renameCallback(m_renameCallbackContext, true);
            m_saveState = C4JStorage::ESaveGame_Idle;
            return;
        }
    }
    else if (state == C4JStorage::ESaveGame_CopyComplete)
    {
        OSJoinThread(&m_saveThreads[5], NULL);

        if (m_copyCallback != NULL)
        {
            m_copyCallback(m_copyCallbackContext, true, m_saveState);
            m_saveState = C4JStorage::ESaveGame_Idle;
            return;
        }
    }
    else if (state >= C4JStorage::ESaveGame_CopyFailed && state <= C4JStorage::ESaveGame_CopyFailedOther)
    {
        OSJoinThread(&m_saveThreads[5], NULL);

        if (m_copyCallback != NULL)
        {
            m_copyCallback(m_copyCallbackContext, false, m_saveState);
            m_saveState = C4JStorage::ESaveGame_Idle;
            return;
        }
    }
    else
    {
        if (state != C4JStorage::ESaveGame_SaveIncomplete)
        {
            return;
        }

        OSJoinThread(&m_saveThreads[0], NULL);
        if (m_incompleteSaveCallback != NULL)
        {
            m_saveState = C4JStorage::ESaveGame_SaveIncompleteWaiting;
            m_incompleteSaveCallback(m_incompleteSaveContext, (C4JStorage::ESaveIncompleteType)m_incompleteReason);
            return;
        }

        if (m_saveCallback != NULL)
        {
            m_saveCallback(m_saveCallbackContext, false);
        }
    }

    m_saveState = C4JStorage::ESaveGame_Idle;
}
void CStorage::SetOpenDirFn(DirectoryOpenCallback callback)
{
    m_openDir = callback;
}
void CStorage::SetReadDirFn(DirectoryReadCallback callback)
{
    m_readDir = callback;
}
void CStorage::SetCloseDirFn(DirectoryCloseCallback callback)
{
    m_closeDir = callback;
}
void CStorage::SetDLCPackageRoot(const char* root)
{
    std::strcpy(m_defaultDLCMountName, root);
}
void CStorage::AddInstalled(CONTENT_DATA& content)
{
    m_installedContent.push_back(content);
}
unsigned int CStorage::EnumerateLocalDLCPackages()
{
    char root[32];
    std::strcpy(root, "/vol/content/WiiU/DLC");
    if (!m_openDir(root, &m_dlcDirectory))
    {
        s_storage->DebugPrintf("Error reading directory %s\n", root);
        return 0;
    }

    while (m_readDir(&m_dlcDirectory, &m_dlcDirectoryEntry))
    {
        if ((m_dlcDirectoryEntry.info.flags & 0x80000000U) != 0 && std::strcmp(m_dlcDirectoryEntry.name, ".") != 0
            && std::strcmp(m_dlcDirectoryEntry.name, "..") != 0)
        {
            CONTENT_DATA content;
            content.state = 0;
            content.updatePackage = false;
            std::strcpy(content.name, m_dlcDirectoryEntry.name);
            std::strcpy(content.path, root);
            std::strcat(content.path, "/");
            std::strcat(content.path, m_dlcDirectoryEntry.name);
            content.index = -1;
            AddInstalled(content);
        }
    }
    s_storage->DebugPrintf("Finished finding files in %s\n", root);
    m_closeDir(&m_dlcDirectory);

    std::strcpy(root, "/vol/content/WiiU/CU/DLC");
    if (!m_openDir(root, &m_dlcDirectory))
    {
        s_storage->DebugPrintf("Error reading directory %s\n", root);
        return 0;
    }

    while (m_readDir(&m_dlcDirectory, &m_dlcDirectoryEntry))
    {
        if ((m_dlcDirectoryEntry.info.flags & 0x80000000U) != 0 && std::strcmp(m_dlcDirectoryEntry.name, ".") != 0
            && std::strcmp(m_dlcDirectoryEntry.name, "..") != 0)
        {
            CONTENT_DATA content;
            content.state = 0;
            content.updatePackage = true;
            std::strcpy(content.name, m_dlcDirectoryEntry.name);
            std::strcpy(content.path, root);
            std::strcat(content.path, "/");
            std::strcat(content.path, m_dlcDirectoryEntry.name);
            content.index = -1;
            AddInstalled(content);
        }
    }
    s_storage->DebugPrintf("Finished finding files in %s\n", root);
    m_closeDir(&m_dlcDirectory);
#ifdef MINECRAFT_PC
    EnumerateLauncherDLCPackages();
#endif
    return 1;
}
#ifdef MINECRAFT_PC
void CStorage::EnumerateLauncherDLCPackages()
{
    const char* root = "/vol/launcher/Windows64Media/DLC";
    if (!m_openDir(root, &m_dlcDirectory))
        return;
    while (m_readDir(&m_dlcDirectory, &m_dlcDirectoryEntry))
    {
        if ((m_dlcDirectoryEntry.info.flags & 0x80000000U) == 0 || std::strcmp(m_dlcDirectoryEntry.name, ".") == 0
            || std::strcmp(m_dlcDirectoryEntry.name, "..") == 0)
            continue;
        bool duplicate = false;
        for (std::vector<CONTENT_DATA>::iterator it = m_installedContent.begin(); it != m_installedContent.end(); ++it)
            duplicate = duplicate || strcasecmp(it->name, m_dlcDirectoryEntry.name) == 0;
        if (duplicate)
        {
            printf("[pc] dlc: skipped Windows64Media/DLC/%s: a pack of that name is already installed\n",
                   m_dlcDirectoryEntry.name);
            continue;
        }
        CONTENT_DATA content;
        content.state = 0;
        content.updatePackage = false;
        std::snprintf(content.name, sizeof(content.name), "%s", m_dlcDirectoryEntry.name);
        std::snprintf(content.path, sizeof(content.path), "%s/%s", root, m_dlcDirectoryEntry.name);
        content.index = -1;
        AddInstalled(content);
        printf("[pc] dlc: found Windows64Media/DLC/%s\n", m_dlcDirectoryEntry.name);
    }
    fflush(stdout);
    m_closeDir(&m_dlcDirectory);
}

bool CStorage::IsLauncherDLC(const CONTENT_DATA& content)
{
    return std::strncmp(content.path, "/vol/launcher/", 14) == 0;
}
#endif
int CStorage::GetInstalledDLC(int, InstalledDLCCallback callback, void* context)
{
    if (m_dlcState != 0)
        return 5;
    m_installedCallback = callback;
    m_installedContext = context;
    if (EnumerateLocalDLCPackages() != 0)
    {
        m_dlcState = 1;
        return 5;
    }
    return 1;
}
intptr_t CStorage::GetDLC(unsigned int index)
{
    return (intptr_t)(&m_installedContent[0] + index);
}
int CStorage::GetAvailableDLCCount(int)
{
    return 0;
}
void CStorage::MountInstalledDLC_thread()
{
    CONTENT_DATA& content = m_installedContent[m_mountedDLCIndex];
    DebugPrintf("m_mountedDrives.push_back %s\n", content.path);
    m_mountedDrives.push_back(DriveMapping(m_dlcMountName, content.path));
    m_dlcMountState = 1;
}
int CStorage::MountInstalledDLC_threadEntry(int, void* storage)
{
    CStorage* self = (CStorage*)storage;
    self->MountInstalledDLC_thread();
    OSExitThread(0);
    return 0;
}
#ifdef MINECRAFT_PC
static OSFastMutex s_pcMountSection;

void CStorage::PcStartNextMount()
{
    EnterCriticalSection(&s_pcMountSection);
    if (m_pcMountBusy || m_pcMountQueue.empty())
    {
        LeaveCriticalSection(&s_pcMountSection);
        return;
    }
    PcQueuedMount next = m_pcMountQueue.front();
    m_pcMountQueue.erase(m_pcMountQueue.begin());
    LeaveCriticalSection(&s_pcMountSection);
    printf("[pc] dlc: mounting queued %s (content %u)\n", next.hasName ? next.name.c_str() : m_defaultDLCMountName,
           next.index);
    if (MountInstalledDLC(0, next.index, next.callback, next.context, next.hasName ? next.name.c_str() : NULL) != 0x3e5)
        next.callback(next.context, 0, 1, 0);
}
#endif
int CStorage::MountInstalledDLC(int, unsigned int index, DLCMountCallback callback, void* context,
                                const char* mountName)
{
#ifdef MINECRAFT_PC
    EnterCriticalSection(&s_pcMountSection);
    if (m_dlcMountState != 0 || m_pcMountBusy)
    {
        PcQueuedMount queued;
        queued.index = index;
        queued.callback = callback;
        queued.context = context;
        queued.hasName = mountName != NULL;
        queued.name = mountName == NULL ? "" : mountName;
        m_pcMountQueue.push_back(queued);
        LeaveCriticalSection(&s_pcMountSection);
        printf("[pc] dlc: mount of %s (content %u) waits for the one in progress\n",
               mountName == NULL ? m_defaultDLCMountName : mountName, index);
        return 0x3e5;
    }
    m_pcMountBusy = true;
    LeaveCriticalSection(&s_pcMountSection);
#else
    if (m_dlcMountState != 0)
        return 0x3e5;
#endif
    m_dlcMountCallback = callback;
    m_dlcMountContext = context;
    m_dlcMountName = mountName == NULL ? m_defaultDLCMountName : mountName;
    m_mountedDLCIndex = index;
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE))
                              + STORAGE_THREAD_STACK_SIZE;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_dlcThread, MountInstalledDLC_threadEntry, 0, this, stackTop,
                                                   STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_PRIORITY,
                                                   OS_THREAD_ATTRIB_AFFINITY_ANY);
    if (created)
    {
        OSResumeThread(&m_dlcThread);
        return 0x3e5;
    }
#ifdef MINECRAFT_PC
    EnterCriticalSection(&s_pcMountSection);
    m_pcMountBusy = false;
    LeaveCriticalSection(&s_pcMountSection);
#endif
    return 0;
}
int CStorage::UnmountInstalledDLC(const char* drive)
{
    const char* name = drive == NULL ? m_defaultDLCMountName : drive;
    std::vector<DriveMapping>::iterator mapping = m_mountedDrives.begin();
    while (mapping != m_mountedDrives.end())
    {
        if (mapping->drive == name)
        {
            m_mountedDrives.erase(mapping);
            return 0;
        }
        ++mapping;
    }
    return 0;
}
int CStorage::GetMountedDLCFileList(const char*, std::vector<std::string>& files)
{
    CONTENT_DATA& content = m_installedContent[m_mountedDLCIndex];
    if (!m_openDir(content.path, &m_dlcDirectory))
    {
        DebugPrintf("Error reading directory %s\n", content.path);
        return 0;
    }
    while (m_readDir(&m_dlcDirectory, &m_dlcDirectoryEntry))
        if ((m_dlcDirectoryEntry.info.flags & 0x80000000U) == 0
            && strcasecmp(m_dlcDirectoryEntry.name, "ProductID.txt") != 0)
        {
            std::string path = std::string(content.path) + "/" + m_dlcDirectoryEntry.name;
            DebugPrintf("In %s found %s\n", content.path, m_dlcDirectoryEntry.name);
            files.push_back(path);
        }
    DebugPrintf("Finished finding files in %s\n", content.path);
    m_closeDir(&m_dlcDirectory);
#ifdef MINECRAFT_PC
    if (IsLauncherDLC(content))
    {
        std::vector<std::string> packs;
        for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
            if (it->size() > 4 && strcasecmp(it->c_str() + it->size() - 4, ".pck") == 0)
                packs.push_back(*it);
        files.swap(packs);
    }
#endif
    return 0;
}
int CStorage::GetMountedDLCIndex()
{
    return (int)m_mountedDLCIndex;
}
std::string CStorage::GetMountedPath(std::string path)
{
    for (std::string::size_type position = 0; position < path.size(); ++position)
    {
        if (path[position] == '/' || path[position] == '\\')
            return std::string();
        if (path[position] == ':')
        {
            std::string drive = path.substr(0, position);
            for (std::vector<DriveMapping>::iterator mapping = m_mountedDrives.begin();
                 mapping != m_mountedDrives.end(); ++mapping)
            {
                if (mapping->drive == drive)
                    return mapping->path + path.substr(position + 1);
            }
            break;
        }
    }
    return std::string();
}
void CStorage::SetDLCProductCode(const char* code)
{
    std::strcpy(m_dlcProductCode, code);
}
void CStorage::SetProductUpgradeKey(const char* key)
{
    std::strcpy(m_productUpgradeKey, key);
}
int CStorage::CheckForTrialUpgradeKey(DLCUpgradeCallback, void*)
{
    return 1;
}
void CStorage::DLCOnCreate()
{
    m_dlcState = 0;
    m_dlcResult = 0;
    m_dlcMountState = 0;
    m_dlcMountNameData = 0;
#ifdef MINECRAFT_PC
    m_pcMountBusy = false;
#endif
    std::memset(m_dlcProductCode, 0, sizeof(m_dlcProductCode));
    std::memset(m_productUpgradeKey, 0, sizeof(m_productUpgradeKey));
}
void CStorage::TickDLC()
{
    if (m_dlcState == 1)
    {
        m_dlcState = 0;
        int count = m_installedContent.empty() ? 0 : (int)m_installedContent.size();
        m_installedCallback(m_installedContext, count, 0);
    }
    if (m_dlcMountState == 1)
    {
        m_dlcMountState = 0;
        OSJoinThread(&m_dlcThread, NULL);
#ifdef MINECRAFT_PC
        EnterCriticalSection(&s_pcMountSection);
        m_pcMountBusy = false;
        LeaveCriticalSection(&s_pcMountSection);
#endif
        m_dlcMountCallback(m_dlcMountContext, 0, 0, m_dlcMountResult);
#ifdef MINECRAFT_PC
        PcStartNextMount();
#endif
    }
}
void CStorage::InitialiseGenericData()
{
    m_genericState = 0;
    m_genericLoadedData = NULL;
    m_genericContext = NULL;
    m_genericWriteData = NULL;
    m_genericCallback = NULL;
    m_genericDataSize = 0;
    FSInitCmdBlock(&m_genericCommandBlock);
    FSAddClient(&m_genericClient, FS_ERROR_FLAG_ALL);
}
int CStorage::ReadFromGenericData(int slot, const std::wstring& name, void** data, unsigned int* size,
                                  GenericDataCallback callback, void* context)
{
    if (slot != -1 && data != NULL && size != NULL && m_genericState == 0)
    {
        m_genericFilename.assign(name);
        m_genericReadSize = size;
        m_genericContext = context;
        m_genericState = 7;
        m_genericSlot = slot;
        m_genericReadData = data;
        m_genericCallback = callback;
        return 1;
    }
    return 0;
}
int CStorage::WriteToGenericData(int slot, const std::wstring& name, void* data, unsigned int size,
                                 GenericDataCallback callback, void* context)
{
    if (slot != -1 && data != NULL && size != 0 && m_genericState == 0)
    {
        m_genericFilename = name;
        m_genericWriteData = data;
        m_genericDataSize = size;
        DebugPrintf("Writing data of size %d at 0x%016llx", size, data);
        DebugPrintf("Writing data of size %d at 0x%016llx", size, data);
        m_genericCallback = callback;
        m_genericSlot = slot;
        m_genericState = 1;
        m_genericContext = context;
        return 1;
    }
    return 0;
}
void CStorage::DeleteGenericData(int, const std::wstring&)
{
}
char* CStorage::wstringtochararray(const std::wstring& value)
{
    char* buffer = s_wideStringBuffer;
    std::wstring::size_type length = value.size();
    for (std::wstring::size_type index = 0; index < length; ++index)
        buffer[index] = static_cast<char>(value[index]);
    buffer[length] = 0;
    return buffer;
}
unsigned int CStorage::LoadGenericDataThreadProc(int slot)
{
    OSFastMutex_Lock(&m_storageMutex);
    if (gProfileInstance->IsSignedIn(slot) && !gProfileInstance->IsGuest(slot))
    {
        gProfileInstance->EnterInitSaveDirCS();
        unsigned char quadrant;
        if (gProfileInstance->initSaveDirectoryForQuadrant(slot, &quadrant))
        {
            FSFileHandle handle;
            int result = SAVEOpenFile(&m_genericClient, &m_genericCommandBlock, quadrant,
                                      wstringtochararray(m_genericFilename), "r", &handle, FS_ERROR_FLAG_ALL);
            FSStat stat;
            if (result == 0
                && FSGetStatFile(&m_genericClient, &m_genericCommandBlock, handle, &stat, FS_ERROR_FLAG_ALL) == 0)
            {
                m_genericLoadedSize = stat.size;
                m_genericLoadedData = (unsigned char*)memalign(64, (m_genericLoadedSize + 0x3fU) & ~0x3fU);
                if (FSReadFile(&m_genericClient, &m_genericCommandBlock, m_genericLoadedData, 1, m_genericLoadedSize,
                               handle, 0, FS_ERROR_FLAG_ALL)
                    == (int)m_genericLoadedSize)
                {
                    FSCloseFile(&m_genericClient, &m_genericCommandBlock, handle, FS_ERROR_FLAG_ALL);
                    m_genericState = 9;
                    gProfileInstance->LeaveInitSaveDirCS();
                    OSFastMutex_Unlock(&m_storageMutex);
                    return 0;
                }
            }
            gProfileInstance->LeaveInitSaveDirCS();
            m_genericState = 10;
            return 0;
        }
    }
    m_genericState = 10;
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_LoadGenericDataThreadProc(int slot, void*)
{
    s_storage->LoadGenericDataThreadProc(slot);
    return 0;
}
unsigned int CStorage::SaveGenericDataThreadProc(int slot)
{
    OSFastMutex_Lock(&m_storageMutex);
    if (gProfileInstance->IsSignedIn(slot) && !gProfileInstance->IsGuest(slot))
    {
        gProfileInstance->EnterInitSaveDirCS();
        unsigned char quadrant;
        if (gProfileInstance->initSaveDirectoryForQuadrant(slot, &quadrant))
        {
            FSFileHandle handle;
            int result = SAVEOpenFile(&m_genericClient, &m_genericCommandBlock, quadrant,
                                      wstringtochararray(m_genericFilename), "w", &handle, FS_ERROR_FLAG_ALL);
            if (result != 0)
            {
                SAVERollbackQuota(&m_genericClient, &m_genericCommandBlock, quadrant, FS_ERROR_FLAG_ALL);
                gProfileInstance->LeaveInitSaveDirCS();
                m_genericState = result == -12 ? 6 : 4;
                OSFastMutex_Unlock(&m_storageMutex);
                return 0;
            }
            unsigned char* alignedData = (unsigned char*)memalign(64, (m_genericDataSize + 0x3fU) & ~0x3fU);
            std::memcpy(alignedData, m_genericWriteData, m_genericDataSize);
            result = FSWriteFile(&m_genericClient, &m_genericCommandBlock, alignedData, 1, m_genericDataSize, handle, 0,
                                 FS_ERROR_FLAG_ALL);
            std::free(alignedData);
            FSCloseFile(&m_genericClient, &m_genericCommandBlock, handle, FS_ERROR_FLAG_ALL);
            if (result == (int)m_genericDataSize)
            {
                if (SAVEFlushQuota(&m_genericClient, &m_genericCommandBlock, quadrant, FS_ERROR_FLAG_ALL) == 0)
                {
                    m_genericState = 3;
                    gProfileInstance->LeaveInitSaveDirCS();
                    OSFastMutex_Unlock(&m_storageMutex);
                    return 0;
                }
                SAVERollbackQuota(&m_genericClient, &m_genericCommandBlock, quadrant, FS_ERROR_FLAG_ALL);
                m_genericState = 4;
            }
            else
            {
                SAVERollbackQuota(&m_genericClient, &m_genericCommandBlock, quadrant, FS_ERROR_FLAG_ALL);
                m_genericState = result == -12 ? 6 : 4;
            }
            gProfileInstance->LeaveInitSaveDirCS();
            OSFastMutex_Unlock(&m_storageMutex);
            return 0;
        }
    }
    m_genericState = 4;
    OSFastMutex_Unlock(&m_storageMutex);
    return 0;
}
int CStorage::_SaveGenericDataThreadProc(int slot, void*)
{
    s_storage->SaveGenericDataThreadProc(slot);
    return 0;
}
void CStorage::GenericDataTick()
{
    unsigned char* stack;
    switch (m_genericState)
    {
    case 0:
    case 2:
    case 8:
        break;
    case 1:
        m_genericState = 2;
        stack = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE));
        if (!C4JThreadImpl::OSCreateThread4J(&m_genericThread, _SaveGenericDataThreadProc, m_genericSlot, this,
                                             stack + STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_STACK_SIZE,
                                             STORAGE_THREAD_PRIORITY, OS_THREAD_ATTRIB_AFFINITY_ANY))
        {
            m_genericState = 5;
            return;
        }
        OSResumeThread(&m_genericThread);
        return;
    case 3:
        OSJoinThread(&m_genericThread, NULL);
        m_genericState = 0;
        m_genericWriteData = NULL;
        if (m_genericCallback != NULL)
            m_genericCallback(m_genericContext, m_genericSlot, (C4JStorage::eGeneralData_Callback_Status)3, 0);
        return;
    case 4:
        OSJoinThread(&m_genericThread, NULL);
    case 5:
        m_genericState = 0;
        m_genericWriteData = NULL;
        if (m_genericCallback != NULL)
            m_genericCallback(m_genericContext, m_genericSlot, (C4JStorage::eGeneralData_Callback_Status)4, 0);
        return;
    case 6:
        OSJoinThread(&m_genericThread, NULL);
        m_genericState = 0;
        m_genericWriteData = NULL;
        if (m_genericCallback != NULL)
            m_genericCallback(m_genericContext, m_genericSlot, (C4JStorage::eGeneralData_Callback_Status)5, 0);
        return;
    case 7:
        m_genericState = 8;
        stack = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(STORAGE_THREAD_STACK_SIZE));
        if (!C4JThreadImpl::OSCreateThread4J(&m_genericThread, _LoadGenericDataThreadProc, m_genericSlot, this,
                                             stack + STORAGE_THREAD_STACK_SIZE, STORAGE_THREAD_STACK_SIZE,
                                             STORAGE_THREAD_PRIORITY, OS_THREAD_ATTRIB_AFFINITY_ANY))
        {
            m_genericState = 11;
            return;
        }
        OSResumeThread(&m_genericThread);
        return;
    case 9:
        OSJoinThread(&m_genericThread, NULL);
        *m_genericReadSize = m_genericLoadedSize;
        *m_genericReadData = m_genericLoadedData;
        m_genericState = 0;
        m_genericCallback(m_genericContext, m_genericSlot, C4JStorage::eGeneralData_Callback_StatusValue, 0);
        m_genericLoadedData = NULL;
        return;
    case 10:
        OSJoinThread(&m_genericThread, NULL);
    case 11:
        *m_genericReadSize = 0;
        *m_genericReadData = NULL;
        m_genericState = 0;
        m_genericCallback(m_genericContext, m_genericSlot, (C4JStorage::eGeneralData_Callback_Status)1, 0);
        if (m_genericLoadedData != NULL)
        {
            std::free(m_genericLoadedData);
            m_genericLoadedData = NULL;
        }
        break;
    }
}
