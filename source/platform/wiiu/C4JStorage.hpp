#ifndef SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP
#define SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <coreinit/filesystem.h>

struct FSDirEntry : FSDirectoryEntry
{
};

struct SAVE_INFO
{
    char filename[32];
    wchar_t title[128];
    FSTime modifiedTime;
    unsigned char* thumbnail;
    unsigned int thumbnailSize;
    unsigned int nandSize;
    unsigned int reserved;
};

struct SAVE_DETAILS
{
    int count;
#ifdef MINECRAFT_PC
    long long nandSize;
#else
    int nandSize;
#endif
    SAVE_INFO* saves;
    int reserved;
};

class C4JStorage
{
public:
    enum PROFILESETTING
    {
        PROFILESETTING_YAXIS_INVERSION = 0,
        PROFILESETTING_CONTROLLER_SENSITIVITY = 1,
        PROFILESETTING_VIBRATION = 2,
        PROFILESETTING_SWAP_STICKS = 3
    };

    struct PROFILESETTINGS
    {
        unsigned int values[3];
        unsigned char flags;
    };

    enum EMessageResult
    {
        EMessageResult_Cancelled = 3,
        EMessageResult_Accept,
        EMessageResult_Decline,
        EMessageResult_ThirdOption,
        EMessageResult_FourthOption
    };

    struct DLC_TMS_DETAILS;
    struct TMSPP_FILEDATA;

    enum eGlobalStorage
    {
        eGlobalStorageValue = 0
    };
    enum eTMS_FileType
    {
        eTMS_FileTypeValue = 0
    };
    enum ESavingMessage
    {
        ESavingMessage_0 = 0,
        ESavingMessage_2 = 2
    };
    enum ESaveIncompleteType
    {
        ESaveIncompleteType_0 = 0,
        ESaveIncompleteType_1 = 1,
        ESaveIncompleteType_2 = 2
    };
    enum ESaveGameState
    {
        ESaveGame_Idle,
        ESaveGame_Save,
        ESaveGame_SaveComplete,
        ESaveGame_SaveFailed,
        ESaveGame_SaveCancelled,
        ESaveGame_SaveIncomplete,
        ESaveGame_SaveIncompleteWaiting,
        ESaveGame_Load,
        ESaveGame_LoadComplete,
        ESaveGame_LoadFailed,
        ESaveGame_Delete,
        ESaveGame_DeleteComplete,
        ESaveGame_DeleteFailed,
        ESaveGame_Rename,
        ESaveGame_RenameComplete,
        ESaveGame_RenameFailed,
        ESaveGame_Copy,
        ESaveGame_CopyComplete,
        ESaveGame_CopyFailed,
        ESaveGame_CopyFailedNoSpace,
        ESaveGame_CopyFailedOther,
        ESaveGame_GetThumbnail
    };
    enum eOptionsCallback
    {
        eOptions_Callback_Idle,
        eOptions_Callback_Write,
        eOptions_Callback_Write_Fail_NoSpace,
        eOptions_Callback_Write_Fail,
        eOptions_Callback_Read,
        eOptions_Callback_Read_Fail,
        eOptions_Callback_Read_FileNotFound,
        eOptions_Callback_Read_Corrupt,
        eOptions_Callback_Read_CorruptDeletePending,
        eOptions_Callback_Read_CorruptDeleted
    };
    enum eGeneralData_Callback_Status
    {
        eGeneralData_Callback_StatusValue = 0
    };

    typedef int (*SavingMessageCallback)(void*, ESavingMessage, int);
    typedef void (*IncompleteSaveCallback)(void*, ESaveIncompleteType);
    typedef int (*DefaultOptionsCallback)(void*, PROFILESETTINGS*, int);
    typedef int (*OptionsDataCallback)(void*, int, unsigned short, eOptionsCallback);
    typedef int (*OldProfileVersionCallback)(void*, unsigned char*, unsigned short, int);
    typedef int (*GenericDataCallback)(void*, int, eGeneralData_Callback_Status, int);
    typedef int (*SaveCallback)(void*, bool);
    typedef int (*LoadCallback)(void*, bool, bool);
    typedef int (*SavesInfoCallback)(void*, SAVE_DETAILS*, bool);
    typedef int (*RenameCallback)(void*, bool, ESaveGameState);
    typedef int (*CopyCallback)(void*, bool, ESaveGameState);
    typedef bool (*CopyProgressCallback)(void*, int);
    typedef int (*DirectoryOpenCallback)(const char*, int*);
    typedef int (*DirectoryReadCallback)(int*, FSDirEntry*);
    typedef int (*DirectoryCloseCallback)(int*);
    typedef int (*DLCOffersCallback)(void*, unsigned int, unsigned int, int);
    typedef int (*OffersCallback)(void*, int, unsigned int, unsigned int);
    typedef int (*InstalledDLCCallback)(void*, int, int);
    typedef int (*DLCMountCallback)(void*, int, unsigned int, unsigned int);
    typedef void (*DLCUpgradeCallback)(void*, bool);

    virtual void Tick() = 0;
    virtual void Init(unsigned int, const wchar_t*, const char*, int, SavingMessageCallback, void*, const char*,
                      bool*) = 0;
    virtual void SetGameSaveFolderTitle(const wchar_t*) = 0;
    virtual void SetSaveCacheFolderTitle(const wchar_t*) = 0;
    virtual void SetOptionsFolderTitle(const wchar_t*) = 0;
    virtual void SetCorruptSaveName(const wchar_t*) = 0;
    virtual void SetGameSaveFolderPrefix(const char*) = 0;
    virtual void SetMaxSaves(int) = 0;
    virtual void SetDefaultImages(unsigned char*, unsigned int, unsigned char*, unsigned int, unsigned char*,
                                  unsigned int) = 0;
    virtual void SetIncompleteSaveCallback(IncompleteSaveCallback, void*) = 0;
    virtual void SetSaveDisabled(bool) = 0;
    virtual unsigned char GetSaveDisabled() = 0;
    virtual void ResetSaveData() = 0;
    virtual int DoesSaveExist(bool*) = 0;
    virtual int EnoughSpaceForAMinSaveGame() = 0;
    virtual ESaveGameState GetSaveState() = 0;
    virtual int GetSavesInfo(int, SavesInfoCallback, void*, const char*) = 0;
    virtual SAVE_DETAILS* ReturnSavesInfo() = 0;
    virtual void ClearSavesInfo() = 0;
    virtual ESaveGameState LoadSaveDataThumbnail(SAVE_INFO*, int (*)(void*, unsigned char*, unsigned int), void*) = 0;
    virtual ESaveGameState LoadSaveData(SAVE_INFO*, LoadCallback, void*) = 0;
    virtual int GetSaveSize() = 0;
    virtual void GetSaveData(void*, unsigned int*) = 0;
    virtual int GetSaveUniqueNumber(int*) = 0;
    virtual int GetSaveUniqueFilename(char*) = 0;
    virtual void SetSaveUniqueFilename(char*) = 0;
    virtual int GetSaveUniqueFileDir(char*) = 0;
    virtual void SetSaveTitle(const wchar_t*) = 0;
#ifdef MINECRAFT_PC
    virtual const wchar_t* GetSaveTitle() = 0;
#endif
    virtual void* AllocateSaveData(unsigned int) = 0;
    virtual void SetSaveDataSize(unsigned int) = 0;
    virtual void GetDefaultSaveImage(unsigned char**, unsigned int*) = 0;
    virtual void GetDefaultSaveThumbnail(unsigned char**, unsigned int*) = 0;
    virtual void SetSaveImages(unsigned char*, unsigned int, unsigned char*, unsigned int, unsigned char*,
                               unsigned int) = 0;
    virtual ESaveGameState SaveSaveData(SaveCallback, void*) = 0;
    virtual void ContinueIncompleteOperation() = 0;
    virtual void CancelIncompleteOperation() = 0;
    virtual ESaveGameState DeleteSaveData(SAVE_INFO*, SaveCallback, void*) = 0;
    virtual ESaveGameState CopySaveData(SAVE_INFO*, CopyCallback, CopyProgressCallback, void*) = 0;
    virtual ESaveGameState RenameSaveData(int, wchar_t*, SaveCallback, void*) = 0;
    virtual void GetSaveImage(unsigned char**, int*) = 0;
    virtual void GetSaveThumbnail(unsigned char**, int*) = 0;
    virtual void InitialiseProfileData(unsigned short, unsigned int, unsigned int, unsigned int*, int,
                                       unsigned int*) = 0;
    virtual int SetDefaultOptionsCallback(DefaultOptionsCallback, void*) = 0;
    virtual void SetOptionsDataCallback(OptionsDataCallback, void*) = 0;
    virtual int SetOldProfileVersionCallback(OldProfileVersionCallback, void*) = 0;
    virtual PROFILESETTINGS* GetDashboardProfileSettings(int) = 0;
    virtual unsigned char* GetGameDefinedProfileData(int) = 0;
    virtual void ReadFromProfile(int, int) = 0;
    virtual void WriteToProfile(int, bool, bool) = 0;
    virtual void DeleteOptionsData(int) = 0;
    virtual void ForceQueuedProfileWrites(int) = 0;
    virtual int IsOptionsWriteInProgressOrPending(int) = 0;
    virtual void InitialiseGenericData() = 0;
    virtual int ReadFromGenericData(int, const std::wstring&, void**, unsigned int*, GenericDataCallback, void*) = 0;
    virtual int WriteToGenericData(int, const std::wstring&, void*, unsigned int, GenericDataCallback, void*) = 0;
    virtual void DeleteGenericData(int, const std::wstring&) = 0;
    virtual void SetSaveDeviceSelected(unsigned int device, bool selected);
    virtual int GetSaveDeviceSelected(unsigned int) = 0;
    virtual void ClearDLCOffers() = 0;
    virtual int ReadTMSFile(int, eGlobalStorage, eTMS_FileType, wchar_t*, unsigned char**, unsigned int*,
                            int (*)(void*, wchar_t*, int, bool, eTMS_FileType), void*, int) = 0;
    virtual int WriteTMSFile(int, eGlobalStorage, wchar_t*, unsigned char*, unsigned int) = 0;
    virtual int DeleteTMSFile(int, eGlobalStorage, wchar_t*) = 0;
    virtual int GetDLCOffers(int pad, DLCOffersCallback callback, void* context, unsigned int contentId) = 0;
    virtual void SetOpenDirFn(DirectoryOpenCallback) = 0;
    virtual void SetReadDirFn(DirectoryReadCallback) = 0;
    virtual void SetCloseDirFn(DirectoryCloseCallback) = 0;
    virtual void SetDLCPackageRoot(const char*) = 0;
    virtual int GetInstalledDLC(int, InstalledDLCCallback, void*) = 0;
    virtual intptr_t GetDLC(unsigned int) = 0;
    virtual int GetAvailableDLCCount(int) = 0;
    virtual int MountInstalledDLC(int, unsigned int, DLCMountCallback, void*, const char*) = 0;
    virtual int UnmountInstalledDLC(const char*) = 0;
    virtual int GetMountedDLCFileList(const char*, std::vector<std::string>&) = 0;
    virtual int GetMountedDLCIndex() = 0;
    virtual std::string GetMountedPath(std::string) = 0;
    virtual void SetDLCProductCode(const char*) = 0;
    virtual void SetProductUpgradeKey(const char*) = 0;
    virtual int CheckForTrialUpgradeKey(DLCUpgradeCallback, void*) = 0;
};

#endif
