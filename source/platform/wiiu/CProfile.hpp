#ifndef PLATFORM_WIIU_CPROFILE_HPP
#define PLATFORM_WIIU_CPROFILE_HPP

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <nn/ffl/miidata.h>
#include <coreinit/filesystem.h>
#include <nn/pia.h>
#include <nn/fp/fp_cpp.h>

#include "game/core/CriticalSection.hpp"
#include "game/util/ACTUuid.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include <nn/ffl/FFLiFsClient.h>
#include "platform/wiiu/MiiData.hpp"

class CAchievements;

enum eAwardType
{
    eAwardType_None = 0
};

enum eUpsellType
{
    eUpsellType_None = 0
};

enum eUpsellResponse
{
    eUpsellResponse_None = 0
};

struct ProfileSignInEntry
{
    int quadrant;
    MiiDataEntry* mii;
    bool online;
    bool guest;
};

class C_4JProfile
{
public:
    typedef int (*SignInUIHandler)(void*, bool, int, int);
    typedef int (*OfflineProfileHandler)(void*, bool, int);
    typedef int (*AvatarHandler)(void*, unsigned char*, unsigned int);
    typedef const wchar_t* (*GetStringHandler)(int);
    typedef void (*SignInChangeHandler)(void*, bool, unsigned int);
    typedef void (*NotificationsHandler)(void*, unsigned int, unsigned int);
    typedef void (*ProfileReadErrorHandler)(void*);
    typedef void (*AwardHandler)(void*, int, int, int, int);
    typedef void (*FriendPresenceHandler)(unsigned int, void*);
    typedef wchar_t* (*RichPresenceStringHandler)(int);
    typedef int (*RichPresenceSettingHandler)(const void*);
    typedef void (*UpsellHandler)(void*, eUpsellType, eUpsellResponse, int);
    typedef int (*SignInChoicesHandler)(int);

    virtual void Initialise(const void*, const void*, unsigned short, unsigned int, unsigned int, unsigned int*, int,
                            unsigned int*) = 0;
    virtual int GetLockedProfile() = 0;
    virtual void SetLockedProfile(int) = 0;
    virtual bool IsSignedIn(int) = 0;
    virtual bool IsSignedInLive(int) = 0;
    virtual bool IsSigningIn(int) = 0;
    virtual bool IsGuest(int) = 0;
    virtual int RequestSignInUI(bool, bool, bool, bool, bool, SignInUIHandler, void*, int) = 0;
    virtual int SignInUser(MiiDataEntry*, int) = 0;
    virtual void SignOutUser(FFLCreateID&) = 0;
    virtual void SignOutUser(int) = 0;
    virtual void SetUserSigningIn(int, bool) = 0;
    virtual int DisplayOfflineProfile(OfflineProfileHandler, void*, int) = 0;
    virtual void RequestConvertOfflineToGuestUI(SignInUIHandler, void*, int) = 0;
    virtual void SetPrimaryPlayerChanged(bool) = 0;
    virtual int QuerySigninStatus() = 0;
    virtual void GetXUID(int, PlayerUID*, bool) = 0;
    virtual int AreXUIDSEqual(PlayerUID, PlayerUID) = 0;
    virtual bool AllowedToPlayMultiplayer(int) = 0;
    virtual void StartTrialGame() = 0;
    virtual void AllowedPlayerCreatedContent(int, bool, int*, int*) = 0;
    virtual bool CanViewPlayerCreatedContent(int, bool, PlayerUID*, unsigned int) = 0;
    virtual bool GetProfileAvatar(int, AvatarHandler, void*) = 0;
    virtual void CancelProfileAvatarRequest() = 0;
    virtual void SetGetStringFunc(GetStringHandler) = 0;
    virtual void SetPlayerListTitleID(int) = 0;
    virtual void SetFatalTrophyErrorID(int) = 0;
    virtual int GetPrimaryPad() = 0;
    virtual void SetPrimaryPad(int) = 0;
    virtual void ClearGameUsers() = 0;
    virtual wchar_t* GetMiiName(int) = 0;
    virtual wchar_t* GetGamertag(int) = 0;
    virtual std::wstring GetDisplayName(int) = 0;
    virtual char* GetNetworkAccountID() = 0;
    virtual bool IsFullVersion() = 0;
    virtual void SetFullVersion(bool) = 0;
    virtual void SetSignInChangeCallback(SignInChangeHandler, void*) = 0;
    virtual void SetSignInChoicesComplete(bool, int, int) = 0;
    virtual void SetNotificationsCallback(NotificationsHandler, void*) = 0;
    virtual bool RegionIsNorthAmerica() = 0;
    virtual bool LocaleIsUSorCanada() = 0;
    virtual int GetLiveConnectionStatus() = 0;
    virtual bool IsSystemUIDisplayed() = 0;
    virtual void SetProfileReadErrorCallback(ProfileReadErrorHandler, void*) = 0;
    virtual void ResetProfileProcessState() = 0;
    virtual void Tick() = 0;
    virtual void RegisterAward(int, int, eAwardType, int, int) = 0;
    virtual int GetAwardId(int) = 0;
    virtual eAwardType GetAwardType(int) = 0;
    virtual bool CanBeAwarded(int, int) = 0;
    virtual void Award(int, int, bool) = 0;
    virtual bool IsAwardsFlagSet(int, int) = 0;
    virtual void SetOnAwardHandler(AwardHandler, void*) = 0;
    virtual void Terminate() = 0;
    virtual void RichPresenceRegisterPresenceString(int, int) = 0;
    virtual void RichPresenceRegisterContext(int, int) = 0;
    virtual void RichPresenceRegisterContextString(int, int, int) = 0;
    virtual void RichPresenceInit(int, int) = 0;
    virtual void SetRichPresenceContextValue(int, int, int) = 0;
    virtual void SetCurrentGameActivity(int, int, bool) = 0;
    virtual void SetRichPresenceSettingFn(RichPresenceSettingHandler) = 0;
    virtual void SetRichPresenceGameModeData(nn::fp::GameMode&) = 0;
    virtual void SetFriendPresenceChangeCallback(FriendPresenceHandler, void*) = 0;
    virtual void SetRichPresenceGetStringCallback(RichPresenceStringHandler) = 0;
    virtual void SetFriendPresenceOnline(bool) = 0;
    virtual void DisplayFullVersionPurchase(bool, int, int) = 0;
    virtual void SetUpsellCallback(UpsellHandler, void*) = 0;
    virtual void SetDebugFullOverride(bool) = 0;
    virtual void SetNetworkStatus(bool) = 0;
    virtual void SetNetworkStatusWithoutSigninChange(bool) = 0;
    virtual void TryToConnectOnline() = 0;
    virtual unsigned int GetSignedInUsersMask() = 0;
    virtual void RefreshChatAndContentRestrictions(ProfileReadErrorHandler, void*) = 0;
    virtual bool GetChatAndContentRestrictions(int, bool, bool*, bool*, int*) = 0;
    virtual void SetServiceID(char*) = 0;
    virtual void HandleNetworkTicket(int, void*) = 0;
    virtual void SetMinimumAge(int) = 0;
    virtual unsigned int getUserID(int) = 0;
    virtual int getQuadrant(ACTUuid&) = 0;
    virtual void SetSignInChoicesCallback(SignInChoicesHandler) = 0;
    virtual std::vector<ProfileSignInEntry>* GetSignInEntries() = 0;
    virtual void LoadAllMiiData() = 0;
    virtual void FreeAllMiiData() = 0;
    virtual int GetNumSysUserMiis() = 0;
    virtual int GetNumFavouriteMiis() = 0;
    virtual int GetNumGuestMiis() = 0;
    virtual MiiDataEntry* GetSysUserMiiData(int) = 0;
    virtual MiiDataEntry* GetFavouriteMiiData(int) = 0;
    virtual MiiDataEntry* GetGuestMiiData(int) = 0;
    virtual bool IsHasAchievementsFile(int) = 0;
    virtual void GetLocalPlayerInfo(nn::pia::transport::Station::PlayerInfo*, int) = 0;
};

class CProfile : public C_4JProfile
{
public:
    enum EProfileProcessState
    {
        eProfileProcessState_Idle = 0,
        eProfileProcessState_ShowSignIn = 1,
        eProfileProcessState_ProcessSignIn = 2,
        eProfileProcessState_QuerySignIn = 3,
        eProfileProcessState_SignOut = 4,
        eProfileProcessState_Unknown7 = 7,
        eProfileProcessState_OfflineProfile = 8,
        eProfileProcessState_Unknown11 = 11,
        eProfileProcessState_Unknown12 = 12,
        eProfileProcessState_Upsell = 13
    };

    struct AvatarRequest
    {
        int quadrant;
        void* context;
        AvatarHandler handler;
    };

    struct PROFILE_DETAILS
    {
        PROFILE_DETAILS();

        bool signedIn;
        bool signingIn;
        bool onlineAccount;
        unsigned char state;
        MiiDataEntry* mii;
        unsigned char reservedBeforeGamertag[4];
        unsigned short gamertag[32];
        unsigned char reservedAfterGamertag[4];
    };

    struct CONTEXTVAL : public boost::unordered_map<int, int>
    {
        CONTEXTVAL();
    };

    struct AwardManagerTask
    {
        CAchievements* achievements;
        int operation;
        int file;
        int state;
        unsigned int dataSize;
        unsigned char data[76];
    };

    struct AwardRegistration
    {
        int registrationId;
        int callbackValue;
        int callbackContext;
    };

    CProfile();
    ~CProfile();

    virtual void Initialise(const void*, const void*, unsigned short, unsigned int, unsigned int, unsigned int*, int,
                            unsigned int*);
    virtual int GetLockedProfile();
    virtual void SetLockedProfile(int);
    virtual bool IsSignedIn(int);
    virtual bool IsSignedInLive(int);
    virtual bool IsSigningIn(int);
    virtual bool IsGuest(int);
    virtual int RequestSignInUI(bool, bool, bool, bool, bool, SignInUIHandler, void*, int);
    int RequestSignInUI(bool, bool, bool, bool, bool, bool, SignInUIHandler, void*, int);
    virtual int SignInUser(MiiDataEntry*, int);
    virtual void SignOutUser(FFLCreateID&);
    virtual void SignOutUser(int);
    virtual void SetUserSigningIn(int, bool);
    virtual int DisplayOfflineProfile(OfflineProfileHandler, void*, int);
    virtual void RequestConvertOfflineToGuestUI(SignInUIHandler, void*, int);
    virtual void SetPrimaryPlayerChanged(bool);
    virtual int QuerySigninStatus();
    virtual void GetXUID(int, PlayerUID*, bool);
    virtual int AreXUIDSEqual(PlayerUID, PlayerUID);
    virtual bool AllowedToPlayMultiplayer(int);
    virtual void StartTrialGame();
    virtual void AllowedPlayerCreatedContent(int, bool, int*, int*);
    virtual bool CanViewPlayerCreatedContent(int, bool, PlayerUID*, unsigned int);
    virtual bool GetProfileAvatar(int, AvatarHandler, void*);
    virtual void CancelProfileAvatarRequest();
    virtual void SetGetStringFunc(GetStringHandler);
    virtual void SetPlayerListTitleID(int);
    virtual void SetFatalTrophyErrorID(int);
    virtual int GetPrimaryPad();
    virtual void SetPrimaryPad(int);
    virtual void ClearGameUsers();
    virtual wchar_t* GetMiiName(int);
    virtual wchar_t* GetGamertag(int);
    virtual std::wstring GetDisplayName(int);
    virtual char* GetNetworkAccountID();
    virtual bool IsFullVersion();
    virtual void SetFullVersion(bool);
    virtual void SetSignInChangeCallback(SignInChangeHandler, void*);
    virtual void SetSignInChoicesComplete(bool, int, int);
    virtual void SetNotificationsCallback(NotificationsHandler, void*);
    virtual bool RegionIsNorthAmerica();
    virtual bool LocaleIsUSorCanada();
    virtual int GetLiveConnectionStatus();
    virtual bool IsSystemUIDisplayed();
    virtual void SetProfileReadErrorCallback(ProfileReadErrorHandler, void*);
    virtual void ResetProfileProcessState();
    virtual void Tick();
    virtual void RegisterAward(int, int, eAwardType, int, int);
    virtual int GetAwardId(int);
    virtual eAwardType GetAwardType(int);
    virtual bool CanBeAwarded(int, int);
    virtual void Award(int, int, bool);
    virtual bool IsAwardsFlagSet(int, int);
    virtual void SetOnAwardHandler(AwardHandler, void*);
    virtual void Terminate();
    virtual void RichPresenceRegisterPresenceString(int, int);
    virtual void RichPresenceRegisterContext(int, int);
    virtual void RichPresenceRegisterContextString(int, int, int);
    virtual void RichPresenceInit(int, int);
    virtual void SetRichPresenceContextValue(int, int, int);
    virtual void SetCurrentGameActivity(int, int, bool);
    virtual void SetRichPresenceSettingFn(RichPresenceSettingHandler);
    virtual void SetRichPresenceGameModeData(nn::fp::GameMode&);
    virtual void SetFriendPresenceChangeCallback(FriendPresenceHandler, void*);
    virtual void SetRichPresenceGetStringCallback(RichPresenceStringHandler);
    virtual void SetFriendPresenceOnline(bool);
    virtual void DisplayFullVersionPurchase(bool, int, int);
    virtual void SetUpsellCallback(UpsellHandler, void*);
    virtual void SetDebugFullOverride(bool);
    virtual void SetNetworkStatus(bool);
    virtual void SetNetworkStatusWithoutSigninChange(bool);
    virtual void TryToConnectOnline();
    virtual unsigned int GetSignedInUsersMask();
    virtual void RefreshChatAndContentRestrictions(ProfileReadErrorHandler, void*);
    virtual bool GetChatAndContentRestrictions(int, bool, bool*, bool*, int*);
    virtual void SetServiceID(char*);
    virtual void HandleNetworkTicket(int, void*);
    virtual void SetMinimumAge(int);
    virtual unsigned int getUserID(int);
    virtual int getQuadrant(ACTUuid&);
    virtual void SetSignInChoicesCallback(SignInChoicesHandler);
    virtual std::vector<ProfileSignInEntry>* GetSignInEntries();
    virtual void LoadAllMiiData();
    virtual void FreeAllMiiData();
    virtual int GetNumSysUserMiis();
    virtual int GetNumFavouriteMiis();
    virtual int GetNumGuestMiis();
    virtual MiiDataEntry* GetSysUserMiiData(int);
    virtual MiiDataEntry* GetFavouriteMiiData(int);
    virtual MiiDataEntry* GetGuestMiiData(int);
    virtual bool IsHasAchievementsFile(int);
    virtual void GetLocalPlayerInfo(nn::pia::transport::Station::PlayerInfo*, int);

    void SetSignInChange(bool);
    void checkParentalControl(int);
    PlayerUID QuadrantOfflineXUID(int);
    PlayerUID QuadrantOnlineXUID(int);
    bool GetChatAndContentRestrictions(int, bool*, bool*, int*);
    void CheckForSignInChange();
    bool GetSignInChange();
    bool PrimaryPlayerChanged();
    bool PrimaryPlayerHasSignedOutDialogBox(bool);
    void SetProfileProcessState(EProfileProcessState);
    void ShowSigninUI();
    int GetNumSignedInUsers();
    bool ProcessSignIn();
    bool IsValidPlayer(int);
    const wchar_t* getStringFromID(int);
    void CreatePresenceString(int, std::wstring&);
    void SetPresenceString(int);
    void RichPresenceOnCreate();
    void TickRichPresence();
    void SetUpsellType(eUpsellType, int);
    void DebugPrintf(const char*, ...);
    void SystemOnCreate();
    void SetSysUIShowing(bool);
    void CheckForSystemNotifications();
    void SetLeaderboardDisable(bool);
    void CallSignInChangeCallback(bool);
    void TickSystem();
    bool AddAwardManagerTask(CAchievements*);
    int GetRegistrationId(int) const;
    void OnSignout(int);
    void OnSignin(int);
    CAchievements* getAchievements(int);
    CAchievements* getAchievementsByPad(int) const;
    bool callAwardManagerCallback(int, int) const;
    CAchievements* nextAwardManagerTask();
    int openAwardManagerFile(int&, const char*, unsigned char, const char*);
    void EnterInitSaveDirCS();
    void LeaveInitSaveDirCS();
    int initSaveDirectory(unsigned char);
    void closeAwardManagerFile(int&);
    void flushSaveDirectory(unsigned char);
    static int AwardManagerThreadProc(int, void*);
    void AwardManagerOnCreate();
    void AwardManagerTick();
    bool initSaveDirectoryForQuadrant(int, unsigned char*);

    void CheckEvents();
    void UpdateApplicationConnected();
    void tickParentalControlResults();
    void CheckSystemUI();
    bool AllowSignIns();
    void SetAllowSignIns(bool);
    void SecondaryPlayerHasSignedOut(bool);
    float GetSignInCountdown();
    void SetSignInCountdown(float);
    bool OfflinePlayersSignedIn();
    bool PrimaryPlayerIsSignedInOffline();
    void SetReadErrorFlag(int, bool);
    void DisplaySubSignIn();
    void checkNPAvailability(int);
    wchar_t* ConvertCharToWChar(char*);
    int GetRequestedSignInQuadrant();
    void RequestTicketChatAndContentRestrictions(int);
    int getQuadrant(unsigned int);
    void SetOnlineID(int, unsigned int, bool);
    unsigned long long currentTimeMillis();
    static void PrintFriendPresence(unsigned int);
    bool GetLicenseChange();
    void SetLicenseChange(bool);
    void ContentLicenseChanged();
    void SetLicenseRequired(bool);
    bool GetLicenseRequired();
    eUpsellType GetUpsellType();
    void CallUpsellCallback(eUpsellResponse);
    void SetLiveConnectionStatus(long);
    bool PlayerAcceptsOfflineLeaderboard();
    void NotificationsCallback(unsigned int, unsigned int);
    void CallProfileReadErrorCallback();
    CAchievements* getAchievementsByUID(PlayerUID) const;
    void AwardManagerOnDestroy();
    void AwardManagerFlush(int);

private:
    MiiData* mMiiData;
    MiiData* mLoadedMiiData;
    int mFatalTrophyErrorId;
    EProfileProcessState mProfileProcessState;
    unsigned int mInitialisationData;
    ACTUuid mAccountUuid;
    FFLCreateID mAccountCreateId;
    struct ParentalControlState
    {
        bool loaded;
        bool enabled;
        bool purchasesBlocked;
        bool communicationBlocked;
    } mParentalControls[4];
    unsigned char mProfileAlignment[2];
    PROFILE_DETAILS mProfiles[4];
    int mProfileResult[13];
    unsigned char mPrimaryState[16];
    bool mPrimaryProfileReady;
    bool mSignInChange;
    bool mPrimaryPlayerChanged;
    unsigned char mPrimaryAlignment;
    float mSignInCountdown;
    unsigned int mLastSignInTick;
    unsigned int mSignedInUserCount;
    unsigned int mSignedInUsersMask;
    bool mRequestExtra;
    bool mRequestOnline;
    bool mRequestGuest;
    bool mRequestSubAccount;
    bool mRequestForce;
    bool mRequestTrial;
    bool mRequestFromGame;
    unsigned char mRequestAlignment;
    std::vector<ProfileSignInEntry> mSignInEntries;
    int mLockedProfile;
    std::deque<int> mSignOutQueue;
    SignInUIHandler mSignInUIHandler;
    SignInChoicesHandler mSignInChoicesHandler;
    void* mSignInUIContext;
    unsigned int mSignInReserved;
    bool mSignInChoicesPending;
    bool mSignInChoicesAccepted;
    int mSignInChoice;
    int mSignInChoiceExtra;
    OfflineProfileHandler mOfflineProfileHandler;
    void* mOfflineProfileContext;
    int mOfflineProfileQuadrant;
    unsigned int mProfileFlowState[5];
    GetStringHandler mGetStringHandler;
    int mPlayerListTitleId;
    unsigned char mAccountFlowState[16];
    bool mParentalControlPending;
    unsigned char mParentalControlState[3];
    bool mAccountSelectionPending;
    unsigned char mAccountSelectionState[3];
    int mAccountResult[4];
    char* mServiceId;
    int mMinimumAge;
    bool mAccountReady;
    bool mProfileReady;
    bool mNetworkTicketReady;
    bool mNetworkAvailable;
    bool mNetworkStatus;
    bool mNetworkStatusPending;
    bool mFriendPresenceOnline;
    bool mNetworkStateReserved;
    bool mNetworkAccount;
    char mNetworkAccountId[19];
    unsigned int mPrincipalId;
    OSFastMutex mProfileCriticalSection;
    OSFastMutex mInitSaveDirCriticalSection;
    std::vector<AvatarRequest> mAvatarRequests;
    unsigned char mProductState[4];
    bool mFullVersion;
    bool mLicenseState;
    bool mUpsellFromTrial;
    bool mSystemUIShowing;
    unsigned char mSystemNotificationState[2];
    bool mLeaderboardDisable;
    bool mDebugFullOverride;
    int mPrimaryPad;
    int mLiveConnectionStatus;
    SignInChangeHandler mSignInChangeHandler;
    void* mSignInChangeContext;
    ProfileReadErrorHandler mProfileReadErrorHandler;
    void* mProfileReadErrorContext;
    NotificationsHandler mNotificationsHandler;
    void* mNotificationsContext;
    UpsellHandler mUpsellHandler;
    void* mUpsellContext;
    eUpsellType mUpsellType;
    int mUpsellParam;
    unsigned int mRichPresenceData[14];
    boost::unordered_map<int, int> mPresenceContexts;
    boost::unordered_map<int, int> mPresenceStrings;
    boost::unordered_map<int, boost::unordered_map<int, int> > mPresenceContextStrings;
    unsigned int mPresenceReserved;
    unsigned long long mLastPresenceUpdate;
    unsigned long long mLastGameModeUpdate;
    bool mPresenceUpdatePending;
    bool mPresenceUpdateDelay;
    bool mGameModeUpdateReady;
    unsigned char mPresenceAlignment;
    nn::fp::GameMode mGameMode;
    std::wstring mCurrentPresenceString;
    int mCurrentGameActivity[4];
    CONTEXTVAL mContextValues[4];
    int mRichPresenceTitleId;
    unsigned int mRichPresenceState;
    RichPresenceSettingHandler mRichPresenceSettingHandler;
    FriendPresenceHandler mFriendPresenceHandler;
    void* mFriendPresenceContext;
    RichPresenceStringHandler mRichPresenceStringHandler;
    AwardHandler mAwardHandler;
    void* mAwardContext;
    std::set<CAchievements*> mAchievements;
    CAchievements* mAchievementsByPad[4];
    unsigned int mInitialisedSaveDirectories;
    AwardRegistration mAwardRegistrations[128];
    std::deque<CAchievements*> mAwardQueue;
    OSThread mAwardThread;
    void* mAwardThreadStack;
    void* mAwardThreadStackTop;
    FSClient mFileSystemClient;
    FSCmdBlock mFileSystemCommand;
    OSFastMutex mAwardMutex;
};

extern C_4JProfile* gProfile;
extern CProfile* gProfileInstance;

#endif
