#include "platform/wiiu/CProfile.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcAccount.hpp"
#endif

#include <cstdarg>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cafe/sci.h>
#include <coreinit/fastmutex.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <nn/save.h>
#include <nn/act/client_cpp.h>
#include <nn/fp/fp_cpp.h>

#include "game/core/CAchievements.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#if defined(RECON_NATIVE)
#include "local_network.hpp"
#endif

namespace nn
{
namespace act
{
unsigned char GetSlotNoEx(const ACTUuid&) asm("GetSlotNoEx__Q2_2nn3actFRC7ACTUuid");
}
}

PlayerUID getPlayerUID(int, bool);
int getSlotId(int);
extern "C" int SCIGetParentalEnable(unsigned char*);
extern "C" int SCIGetParentalAccountEShopPurchase(unsigned char*, unsigned char);
extern "C" int SCIGetParentalAccountNetCommunicationOnGame(unsigned char*, unsigned char);
extern "C" int _SCIReadSysConfig(const char*, unsigned int, unsigned int, void*);
int VerifyCountryRange(unsigned int*);
int SCIGetISOResource(char*, unsigned int, const char*);
void* allocAligned();
void stateChangeCallback(FSClient*, FSVolumeState, void*);

extern C4JStorage* gStorage;

C_4JProfile* gProfile;
CProfile* gProfileInstance = NULL;

static char sCountryCodeData[0x300];

PlayerUID getPlayerUID(int quadrant, bool includeOffline)
{
    PlayerUID uid(0);
    if (gProfileInstance->IsValidPlayer(quadrant))
        gProfileInstance->GetXUID(quadrant, &uid, includeOffline);
    return uid;
}

int getSlotId(int quadrant)
{
    PlayerUID uid = getPlayerUID(quadrant, false);
    if ((uid.flags & 0x80000000u) != 0)
        return nn::act::GetSlotNoEx(uid.uuid);
    return 0;
}

void* allocAligned()
{
    void* data = MEMAllocFromDefaultHeapEx(0x40, 0x40);
    std::memset(data, 0, 0x40);
    return data;
}

int SCIGetCafeCountry(unsigned int* country)
{
    int result = _SCIReadSysConfig("cafe.cntry_reg", 3, 4, country);
    VerifyCountryRange(country);
    return result;
}

const char* SCIGetCountryCodeA2(unsigned int country)
{
    if (country < 0x100 && SCIGetISOResource(sCountryCodeData, sizeof(sCountryCodeData), "country.txt") == 1)
    {
        const char* end = sCountryCodeData + country * 3 - 1;
        while (*++end != '\0')
        {
        }
        if (end != sCountryCodeData + country * 3)
            return sCountryCodeData + country * 3;
    }
    return NULL;
}

int SCIGetParentalEnable(unsigned char* enabled)
{
    int result = _SCIReadSysConfig("parent.enable", 1, 1, enabled);
    if (*enabled >= 2)
        *enabled = 1;
    else if (static_cast<signed char>(*enabled) < 0)
        *enabled = 0;
    return result;
}

int SCIGetUCParentalAccountSlotNum(unsigned char accountSlot)
{
    return accountSlot;
}

int SCIGetParentalAccountEShopPurchase(unsigned char* blocked, unsigned char accountSlot)
{
    int slot = SCIGetUCParentalAccountSlotNum(accountSlot);
    if (slot == 0)
        return -5;

    char key[0x40];
    std::snprintf(key, sizeof(key), "p.acct.%d.eshop.purchase", slot);
    int result = _SCIReadSysConfig(key, 1, 1, blocked);
    if (*blocked >= 2)
        *blocked = 1;
    else if (static_cast<signed char>(*blocked) < 0)
        *blocked = 0;
    return result;
}

int SCIGetParentalAccountNetCommunicationOnGame(unsigned char* blocked, unsigned char accountSlot)
{
    int slot = SCIGetUCParentalAccountSlotNum(accountSlot);
    if (slot == 0)
        return -5;

    char key[0x40];
    std::snprintf(key, sizeof(key), "p.acct.%d.net.communication.on.game", slot);
    int result = _SCIReadSysConfig(key, 1, 1, blocked);
    if (*blocked >= 2)
        *blocked = 1;
    else if (static_cast<signed char>(*blocked) < 0)
        *blocked = 0;
    return result;
}

static inline unsigned long long getSystemMilliseconds()
{
    return OSTicksToMilliseconds(OSGetTime());
}

CProfile::PROFILE_DETAILS::PROFILE_DETAILS() : signedIn(false)
{
}

CProfile::CONTEXTVAL::CONTEXTVAL() : boost::unordered_map<int, int>(11)
{
}

CProfile::CProfile()
    : mMiiData(new MiiData),
      mLoadedMiiData(mMiiData),
      mPresenceContexts(11),
      mPresenceStrings(11),
      mPresenceContextStrings(11),
      mInitialisedSaveDirectories(0)
{
    gProfileInstance = this;
    mProfileProcessState = eProfileProcessState_Idle;
    mInitialisationData = 0;
    mPrimaryProfileReady = true;
    mPrimaryPlayerChanged = false;
    mProfileFlowState[0] = 0;
    mProfileFlowState[1] = 0;
    mProfileFlowState[2] = 0;
    mProfileFlowState[3] = 0;
    SetSignInChange(false);
    mSignInCountdown = 0.0f;
    mLastSignInTick = GetTickCount();
    mSignedInUserCount = 0;
    mSignedInUsersMask = 0;
    mRequestExtra = false;
    mRequestOnline = false;
    mRequestGuest = false;
    mRequestSubAccount = false;
    mRequestFromGame = false;
    mLockedProfile = -1;
    mProfileResult[0] = -1;
    mProfileResult[1] = -1;
    mProfileResult[2] = -1;
    mProfileResult[3] = -1;
    mProfileResult[5] = -1;
    mProfileResult[6] = -1;
    mProfileResult[7] = -1;
    mProfileResult[8] = -1;
    mProfileResult[9] = -1;
    mProfileResult[10] = -1;
    mProfileResult[11] = -1;
    mProfileResult[12] = -1;
    mGetStringHandler = NULL;
    mParentalControlPending = false;
    mParentalControlState[0] = 0;
    mParentalControlState[1] = 0;
    mParentalControlState[2] = 0;
    mAccountSelectionPending = false;
    mAccountSelectionState[0] = 0;
    mAccountSelectionState[1] = 0;
    mAccountSelectionState[2] = 0;
    mAccountResult[0] = 0;
    mAccountResult[1] = 0;
    mAccountResult[2] = 0;
    mAccountResult[3] = 0;
    std::memset(mParentalControls, 0, sizeof(mParentalControls));
    mSignInChoicesPending = false;
    mSignInChoice = 0;
    mSignInChoiceExtra = 0;
    mServiceId = NULL;
    mMinimumAge = 99;
    mAccountReady = false;
    mProfileReady = false;
    mNetworkAvailable = false;
    mNetworkStatus = false;
    mFriendPresenceOnline = false;
    InitializeCriticalSection(&mProfileCriticalSection);
    InitializeCriticalSection(&mInitSaveDirCriticalSection);
    mMiiData = new MiiData;
    *mLoadedMiiData = *mMiiData;
    SystemOnCreate();
    AwardManagerOnCreate();
    RichPresenceOnCreate();
}

void CProfile::SetSignInChange(bool changed)
{
    mSignInChange = changed;
}

void CProfile::Initialise(const void*, const void*, unsigned short, unsigned int, unsigned int, unsigned int*, int,
                          unsigned int*)
{
    nn::act::Initialize();
    nn::act::GetUuid(reinterpret_cast<char*>(&mAccountUuid));
    TryToConnectOnline();
    mNetworkAccount = nn::act::IsNetworkAccount();
    if (mNetworkAccount)
    {
        nn::act::GetAccountId(mNetworkAccountId);
        mPrincipalId = nn::act::GetPrincipalId();
    }
    else
    {
        mPrincipalId = 0;
    }
    mProfileReady = true;
}

int CProfile::GetLockedProfile()
{
    return mLockedProfile;
}

void CProfile::SetLockedProfile(int quadrant)
{
    if (mLockedProfile != quadrant)
    {
        if (quadrant >= 0)
            OnSignin(quadrant);
        mLockedProfile = quadrant;
    }
}

bool CProfile::IsSignedIn(int quadrant)
{
    return quadrant >= 0 && quadrant < 4 && mProfiles[quadrant].signedIn;
}

bool CProfile::IsSignedInLive(int quadrant)
{
    return IsSignedIn(quadrant) && mNetworkAvailable && mFriendPresenceOnline;
}

bool CProfile::IsSigningIn(int quadrant)
{
    return mProfiles[quadrant].signingIn;
}

bool CProfile::IsGuest(int quadrant)
{
    return mProfiles[quadrant].mii->mIsGuestMii;
}

int CProfile::RequestSignInUI(bool online, bool guest, bool subAccount, bool trial, bool fromGame,
                              SignInUIHandler handler, void* context, int quadrant)
{
    return RequestSignInUI(online, guest, subAccount, trial, false, fromGame, handler, context, quadrant);
}

int CProfile::RequestSignInUI(bool online, bool guest, bool subAccount, bool trial, bool force, bool fromGame,
                              SignInUIHandler handler, void* context, int quadrant)
{
    mRequestOnline = online;
    mRequestGuest = guest;
    mRequestSubAccount = subAccount;
    mRequestForce = force;
    mRequestTrial = trial;
    mRequestFromGame = fromGame;
    mSignInUIHandler = handler;
    mSignInUIContext = context;
#ifdef MINECRAFT_PC
    int choice = 0;
    for (int slot = 0; slot < 4; ++slot)
    {
        if (!IsSignedIn(slot))
        {
            choice = slot;
            break;
        }
    }
    if (IsSignedIn(quadrant))
    {
        SetSignInChoicesComplete(true, quadrant, quadrant);
        return 0;
    }
    MiiDataEntry* guestMii = NULL;
    for (unsigned int index = 0; index < mLoadedMiiData->mGuestMiis.size() && guestMii == NULL; ++index)
    {
        MiiDataEntry* candidate = &mLoadedMiiData->mGuestMiis[index];
        bool used = false;
        for (int slot = 0; slot < 4; ++slot)
            used = used || (mProfiles[slot].signedIn && mProfiles[slot].mii == candidate);
        if (!used)
            guestMii = candidate;
    }
    SetSignInChoicesComplete(guestMii != NULL && SignInUser(guestMii, quadrant) >= 0, choice, quadrant);
    return 0;
#else
    mSignInChoicesHandler(quadrant);
    return 0;
#endif
}

void CProfile::checkParentalControl(int quadrant)
{
    unsigned char slot = nn::act::GetSlotNo();
    ParentalControlState& state = mParentalControls[quadrant];
    if (!state.loaded)
    {
        unsigned char result = 0;
        if (SCIGetParentalEnable(&result) == 1)
            state.enabled = result == 1;
        if (SCIGetParentalAccountEShopPurchase(&result, slot) == 1)
            state.purchasesBlocked = result == 1;
        if (SCIGetParentalAccountNetCommunicationOnGame(&result, slot) == 1)
            state.communicationBlocked = result == 1;
        state.loaded = true;
    }
}

int CProfile::SignInUser(MiiDataEntry* mii, int pad)
{
    if (mii != NULL)
    {
        for (int quadrant = 0; quadrant < 4; ++quadrant)
        {
            if (mProfiles[quadrant].signedIn
                && std::memcmp(mProfiles[quadrant].mii->mCreatorId, mii->mCreatorId, 10) == 0)
                return -1;
        }
    }
    int quadrant = 0;
    while (quadrant < 4 && mProfiles[quadrant].signedIn)
        ++quadrant;
    if (quadrant == 4)
    {
        if (mii != NULL)
            SetSignInChange(true);
        return -1;
    }
    PROFILE_DETAILS& profile = mProfiles[quadrant];
    profile.mii = mii;
    profile.signedIn = true;
    if (mii != NULL)
    {
        profile.onlineAccount = mii->mIsAccountMii && mii->mUuid == mAccountUuid;
#ifdef MINECRAFT_PC
        if (mii->mIsGuestMii)
        {
            std::string name;
            for (unsigned int guest = 0; guest < mLoadedMiiData->mGuestMiis.size(); ++guest)
            {
                if (&mLoadedMiiData->mGuestMiis[guest] == mii)
                    name = PcGuestName(static_cast<int>(guest) + 1);
            }
            memset(profile.gamertag, 0, sizeof(profile.gamertag));
            if (name.empty())
            {
                for (int index = 0; index < 11; ++index)
                    profile.gamertag[index] = mii->mName[index];
            }
            for (size_t index = 0; index < name.size() && index < 31; ++index)
                profile.gamertag[index] = static_cast<unsigned char>(name[index]);
        }
        else
#endif
        if (mNetworkAccount)
        {
            if (quadrant == 0)
                std::swprintf((wchar_t*)profile.gamertag, 32, L"%s", mNetworkAccountId);
            else
                std::swprintf((wchar_t*)profile.gamertag, 32, L"%s (%d)", mNetworkAccountId, quadrant + 1);
        }
        checkParentalControl(quadrant);
    }
    OnSignin(pad);
    g_pInputInstance->LockPad(pad, quadrant);
    if (mii != NULL)
        SetSignInChange(true);
    return quadrant;
}

void CProfile::SignOutUser(FFLCreateID& createId)
{
    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        if (mProfiles[quadrant].signedIn && std::memcmp(mProfiles[quadrant].mii->mCreatorId, &createId, 10) == 0)
        {
            mSignOutQueue.push_back(quadrant);
            return;
        }
    }
}

void CProfile::SignOutUser(int quadrant)
{
    if (quadrant >= 0 && quadrant < 4 && IsSignedIn(quadrant))
        mSignOutQueue.push_back(quadrant);
}

void CProfile::SetUserSigningIn(int quadrant, bool signingIn)
{
    mProfiles[quadrant].signingIn = signingIn;
}

int CProfile::DisplayOfflineProfile(OfflineProfileHandler handler, void* context, int quadrant)
{
    mOfflineProfileHandler = handler;
    mOfflineProfileContext = context;
    mOfflineProfileQuadrant = quadrant;
    mProfileProcessState = eProfileProcessState_OfflineProfile;
    return 0;
}

void CProfile::RequestConvertOfflineToGuestUI(SignInUIHandler handler, void* context, int quadrant)
{
    RequestSignInUI(false, false, false, false, true, true, handler, context, quadrant);
}

void CProfile::SetPrimaryPlayerChanged(bool changed)
{
    mPrimaryPlayerChanged = changed;
}

int CProfile::QuerySigninStatus()
{
    mSignedInUserCount = 0;
    mSignedInUsersMask = 0;
    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        if (mProfiles[quadrant].signedIn)
        {
            ++mSignedInUserCount;
            mSignedInUsersMask |= 1 << quadrant;
        }
    }
    return 0;
}

PlayerUID CProfile::QuadrantOfflineXUID(int quadrant)
{
    MiiDataEntry* mii = mProfiles[quadrant].mii;
    if (mii == NULL)
        return PlayerUID(0);
    if (mii->mIsAccountMii)
        return PlayerUID(mii->mUuid, mProfiles[quadrant].onlineAccount, g_pInputInstance->IsPadDRCController(quadrant));
#ifdef MINECRAFT_PC
    if (mii->mIsGuestMii)
        return PlayerUID(mii->mUuid, mProfiles[quadrant].onlineAccount, g_pInputInstance->IsPadDRCController(quadrant));
#endif
    FFLCreateID* createId = (FFLCreateID*)mii->mCreatorId;
    return PlayerUID(*createId, mPrincipalId, mii->mIsGuestMii, g_pInputInstance->IsPadDRCController(quadrant));
}

PlayerUID CProfile::QuadrantOnlineXUID(int quadrant)
{
    if (mFriendPresenceOnline)
        return QuadrantOfflineXUID(quadrant);
    return PlayerUID(0);
}

void CProfile::GetXUID(int quadrant, PlayerUID* uid, bool online)
{
    *uid = online ? QuadrantOnlineXUID(quadrant) : QuadrantOfflineXUID(quadrant);
}

int CProfile::AreXUIDSEqual(PlayerUID first, PlayerUID second)
{
    return first == second;
}

bool CProfile::AllowedToPlayMultiplayer(int quadrant)
{
    return IsSignedInLive(quadrant);
}

void CProfile::StartTrialGame()
{
}

void CProfile::AllowedPlayerCreatedContent(int quadrant, bool singlePlayer, int* allowed, int* communicationAllowed)
{
    int result = true;
    for (int player = 0; player < 4; ++player)
    {
        if (!singlePlayer || player == quadrant)
        {
            if (IsSignedIn(player) && !IsGuest(player))
            {
                bool contentRestricted = false;
                GetChatAndContentRestrictions(player, true, NULL, &contentRestricted, NULL);
                result = !contentRestricted;
            }
            if (!result)
            {
                break;
            }
        }
    }
    *allowed = result;
    *communicationAllowed = result;
}

bool CProfile::CanViewPlayerCreatedContent(int quadrant, bool singlePlayer, PlayerUID*, unsigned int)
{
    bool result = true;
    for (int player = 0; player < 4; ++player)
    {
        if (!singlePlayer || player == quadrant)
        {
            int allowed = 1;
            int communicationAllowed = 1;
            AllowedPlayerCreatedContent(player, true, &allowed, &communicationAllowed);
            if (!allowed && !communicationAllowed)
            {
                result = false;
                break;
            }
        }
    }
    return result;
}

bool CProfile::GetProfileAvatar(int quadrant, AvatarHandler handler, void* context)
{
    AvatarRequest request = {quadrant, context, handler};
    mAvatarRequests.push_back(request);
    return true;
}

void CProfile::CancelProfileAvatarRequest()
{
}

void CProfile::SetGetStringFunc(GetStringHandler handler)
{
    mGetStringHandler = handler;
}

void CProfile::SetPlayerListTitleID(int titleId)
{
    mPlayerListTitleId = titleId;
}

void CProfile::SetFatalTrophyErrorID(int stringId)
{
    mFatalTrophyErrorId = stringId;
}

wchar_t* CProfile::GetMiiName(int quadrant)
{
    static wchar_t empty[] = L"";
    return IsSignedIn(quadrant) ? (wchar_t*)mProfiles[quadrant].mii->mName : empty;
}

wchar_t* CProfile::GetGamertag(int quadrant)
{
#if defined(RECON_NATIVE)
    return GetMiiName(quadrant);
#else
    if (!IsSignedInLive(quadrant))
        return GetMiiName(quadrant);
    return (wchar_t*)mProfiles[quadrant].gamertag;
#endif
}

std::wstring CProfile::GetDisplayName(int quadrant)
{
    return std::wstring(GetGamertag(quadrant));
}

char* CProfile::GetNetworkAccountID()
{
    return mNetworkAccountId;
}

void CProfile::SetSignInChoicesComplete(bool accepted, int choice, int extra)
{
    mSignInChoicesAccepted = accepted;
    mSignInChoice = choice;
    mSignInChoiceExtra = extra;
    mSignInChoicesPending = true;
}

void CProfile::ResetProfileProcessState()
{
    mProfileProcessState = eProfileProcessState_Idle;
    gStorage->SetSaveDisabled(false);
}

bool CProfile::GetSignInChange()
{
    return mSignInChange;
}

void CProfile::CheckForSignInChange()
{
    if (GetSignInChange())
    {
        SetSignInChange(false);
        if (QuerySigninStatus() == 0)
            CallSignInChangeCallback(mPrimaryPlayerChanged);
        else
        {
            mPrimaryPlayerChanged = true;
            CallSignInChangeCallback(true);
        }
    }
    if (mSignInChoicesPending)
    {
        mSignInUIHandler(mSignInUIContext, mSignInChoicesAccepted, mSignInChoice, mSignInChoiceExtra);
        mSignInChoicesPending = false;
    }
}

bool CProfile::PrimaryPlayerChanged()
{
    return mPrimaryPlayerChanged;
}

bool CProfile::PrimaryPlayerHasSignedOutDialogBox(bool)
{
    return false;
}

void CProfile::SetProfileProcessState(EProfileProcessState state)
{
    mProfileProcessState = state;
}

void CProfile::Tick()
{
    TickSystem();
    CheckForSystemNotifications();
    TickRichPresence();
    CheckForSignInChange();
    AwardManagerTick();
    while (!mAvatarRequests.empty())
    {
        AvatarRequest& request = mAvatarRequests.back();
        if (request.handler != NULL)
        {
            MiiDataEntry* mii = mProfiles[request.quadrant].mii;
            request.handler(request.context, NULL, mii == NULL ? 0 : mii->mIconTexture);
            request.handler = NULL;
        }
        mAvatarRequests.pop_back();
    }
    if (!mSignOutQueue.empty())
    {
        int quadrant = mSignOutQueue.front();
        if (gStorage->IsOptionsWriteInProgressOrPending(quadrant) == 0)
        {
            if (mProfiles[quadrant].signedIn)
            {
                mProfiles[quadrant].signedIn = false;
                std::memset(&mProfiles[quadrant], 0, sizeof(PROFILE_DETAILS));
                g_pInputInstance->UnlockPad(quadrant);
            }
            SetSignInChange(true);
            mSignOutQueue.pop_front();
        }
    }
    if (mProfileProcessState == eProfileProcessState_Idle)
    {
        SetLeaderboardDisable(false);
        if (PrimaryPlayerChanged() && GetLockedProfile() != -1)
            SetLockedProfile(-1);
        else
            SetPrimaryPlayerChanged(false);
    }
    else if (mProfileProcessState == eProfileProcessState_Unknown7)
    {
        SetProfileProcessState(eProfileProcessState_Unknown12);
    }
    else if (mProfileProcessState == eProfileProcessState_Unknown11)
    {
        if (PrimaryPlayerChanged() && PrimaryPlayerHasSignedOutDialogBox(true))
            SetLockedProfile(-1);
    }
    else if (mProfileProcessState == eProfileProcessState_Upsell)
    {
        DebugPrintf("-----------------------------SonyCommerce_UpgradeTrial removed for now\n");
        SetProfileProcessState(eProfileProcessState_Unknown12);
    }
}

void CProfile::SetFriendPresenceOnline(bool online)
{
    mFriendPresenceOnline = online;
}

void CProfile::SetNetworkStatus(bool online)
{
    mFriendPresenceOnline = online;
    mNetworkAvailable = online;
    if (mNetworkStatus != online)
    {
        mNetworkStatus = online;
        SetSignInChange(true);
    }
}

void CProfile::SetNetworkStatusWithoutSigninChange(bool online)
{
    mFriendPresenceOnline = online;
    mNetworkAvailable = online;
    mNetworkStatus = online;
}

void CProfile::TryToConnectOnline()
{
#if defined(RECON_NATIVE)
    SetNetworkStatus(native_local_network::start(0));
#endif
}

unsigned int CProfile::GetSignedInUsersMask()
{
    return mSignedInUsersMask;
}

void CProfile::RefreshChatAndContentRestrictions(ProfileReadErrorHandler, void*)
{
}

bool CProfile::GetChatAndContentRestrictions(int, bool* chatRestricted, bool* contentRestricted, int* age)
{
    if (chatRestricted != NULL)
        *chatRestricted = false;
    if (contentRestricted != NULL)
        *contentRestricted = false;
    if (age != NULL)
        *age = 100;
    return true;
}

bool CProfile::GetChatAndContentRestrictions(int, bool, bool* chatRestricted, bool* contentRestricted, int* age)
{
    if (chatRestricted != NULL)
        *chatRestricted = false;
    if (contentRestricted != NULL)
        *contentRestricted = false;
    if (age != NULL)
        *age = 100;
    return true;
}

void CProfile::SetServiceID(char* serviceId)
{
    mServiceId = serviceId;
}

void CProfile::HandleNetworkTicket(int, void*)
{
}

void CProfile::SetMinimumAge(int age)
{
    mMinimumAge = age;
}

unsigned int CProfile::getUserID(int)
{
    return 0;
}

int CProfile::getQuadrant(ACTUuid&)
{
    return -1;
}

void CProfile::SetSignInChoicesCallback(SignInChoicesHandler handler)
{
    mSignInChoicesHandler = handler;
}

std::vector<ProfileSignInEntry>* CProfile::GetSignInEntries()
{
    return &mSignInEntries;
}

void CProfile::LoadAllMiiData()
{
    mLoadedMiiData->LoadAllData();
    MiiDataEntry* accountMii = NULL;
    for (unsigned int index = 0; index < mLoadedMiiData->mSysUserMiis.size(); ++index)
    {
        if (mLoadedMiiData->mSysUserMiis[index].mUuid == mAccountUuid)
            accountMii = &mLoadedMiiData->mSysUserMiis[index];
    }
    mAccountCreateId = *reinterpret_cast<const FFLCreateID*>(accountMii->mCreatorId);
    SignInUser(accountMii, -1);
}

void CProfile::FreeAllMiiData()
{
    mLoadedMiiData->FreeAllData();
}

int CProfile::GetNumSysUserMiis()
{
    return (int)mLoadedMiiData->mSysUserMiis.size();
}

int CProfile::GetNumFavouriteMiis()
{
    return (int)mLoadedMiiData->mMiiMakerMiis.size();
}

int CProfile::GetNumGuestMiis()
{
    return (int)mLoadedMiiData->mGuestMiis.size();
}

MiiDataEntry* CProfile::GetSysUserMiiData(int index)
{
    return &mLoadedMiiData->mSysUserMiis[index];
}

MiiDataEntry* CProfile::GetFavouriteMiiData(int index)
{
    return &mLoadedMiiData->mMiiMakerMiis[index];
}

MiiDataEntry* CProfile::GetGuestMiiData(int index)
{
    return &mLoadedMiiData->mGuestMiis[index];
}

bool CProfile::IsHasAchievementsFile(int quadrant)
{
    CAchievements* achievements = getAchievements(quadrant);
    return achievements != NULL && achievements->mHasAchievementsFile;
}

void CProfile::ShowSigninUI()
{
    if (!IsSystemUIDisplayed())
    {
        SetSignInChange(false);
        SetSysUIShowing(true);
        SetProfileProcessState(eProfileProcessState_ProcessSignIn);
    }
}

int CProfile::GetNumSignedInUsers()
{
    return mSignedInUserCount;
}

bool CProfile::ProcessSignIn()
{
    if (GetNumSignedInUsers() == 0)
        return false;
    if (GetPrimaryPad() != -1 && IsValidPlayer(GetPrimaryPad()) && IsSignedIn(GetPrimaryPad()))
        return true;
    SetPrimaryPad(-1);
    return false;
}

bool CProfile::IsValidPlayer(int quadrant)
{
    return (mSignedInUsersMask & (1 << quadrant)) != 0;
}

void CProfile::GetLocalPlayerInfo(nn::pia::transport::Station::PlayerInfo*, int)
{
}

void CProfile::Terminate()
{
}

void CProfile::RichPresenceRegisterPresenceString(int id, int stringId)
{
    mPresenceStrings[id] = stringId;
}

void CProfile::RichPresenceRegisterContext(int id, int contextId)
{
    mPresenceContexts[id] = contextId;
    for (int quadrant = 0; quadrant < 4; ++quadrant)
        mContextValues[quadrant][id] = -1;
}

void CProfile::RichPresenceRegisterContextString(int context, int value, int stringId)
{
    mPresenceContextStrings[context][value] = stringId;
}

void CProfile::RichPresenceInit(int, int titleId)
{
    mRichPresenceTitleId = titleId;
    nn::Result result = nn::fp::Initialize();
    unsigned int resultValue = static_cast<NNResult>(result).value;
    unsigned int level = (resultValue >> 27) & 3;
    unsigned int moduleMask = level == 3 ? 0x07f00000 : 0x1ff00000;
    unsigned int descriptionMask = level == 3 ? 0x3ff : 0xfffff;
    unsigned int description = resultValue & descriptionMask;
    if ((resultValue & moduleMask) == 0x00c00000 && description > 0x31ff && description < 0x1f400)
    {
        unsigned int errorCode = nn::fp::ResultToErrorCode(result);
        DebugPrintf("\"%s @%d\" failed. errorCode = %03u-%04u\n", "RichPresenceInit", 44, errorCode / 10000,
                    errorCode % 10000);
    }
    mLastPresenceUpdate = getSystemMilliseconds() - 600000;
    mLastGameModeUpdate = 0;
    mCurrentPresenceString = L"";
    mPresenceUpdatePending = false;
    mPresenceUpdateDelay = false;
    mGameModeUpdateReady = false;
}

const wchar_t* CProfile::getStringFromID(int stringId)
{
    return mRichPresenceStringHandler(stringId);
}

void CProfile::CreatePresenceString(int quadrant, std::wstring& text)
{
    text.assign(getStringFromID(mPresenceStrings[mCurrentGameActivity[quadrant]]));
    for (boost::unordered_map<int, int>::const_iterator context = mPresenceContexts.begin();
         context != mPresenceContexts.end(); ++context)
    {
        const wchar_t* formatToken = getStringFromID(context->second);
        std::wstring::size_type position = text.find(formatToken);
        if (position != std::wstring::npos)
        {
            int setting = mContextValues[quadrant][context->first];
            const wchar_t* replacement = getStringFromID(mPresenceContextStrings[context->first][setting]);
            text.replace(position, std::wcslen(formatToken), replacement);
        }
    }
}

void CProfile::SetPresenceString(int presence)
{
    if (presence != GetPrimaryPad())
        return;
    std::wstring text;
    CreatePresenceString(presence, text);
    if (text == mCurrentPresenceString)
    {
        mPresenceUpdatePending = false;
        return;
    }

    unsigned long long now = getSystemMilliseconds();
    unsigned int interval = 600000;
    if (mGameMode.joinAvailabilityFlag == 1)
        interval = 120000;
    long long elapsed = now - mLastPresenceUpdate;
    if (elapsed <= interval && !mGameModeUpdateReady)
    {
        if (!mPresenceUpdatePending)
        {
            DebugPrintf("Setting m_bPresenceUpdatePending = true - %ls %.02f secs since the last one\n", text.c_str(),
                        (float)elapsed / 1000.0f);
            mPresenceUpdatePending = true;
        }
        return;
    }

    nn::Result result;
    if (!mGameModeUpdateReady && !mPresenceUpdateDelay)
    {
        DebugPrintf("nn::fp::UpdateGameModeDescription");
        result = nn::fp::UpdateGameModeDescription(reinterpret_cast<const char16_t*>(text.c_str()));
    }
    else
    {
        DebugPrintf("nn::fp::UpdateGameMode - %ls\n", text.c_str());
        result = nn::fp::UpdateGameMode(&mGameMode, reinterpret_cast<const char16_t*>(text.c_str()), 0);
    }
    if (result.IsSuccess())
    {
        DebugPrintf(" succeeded %.02f secs since the last one\n", (float)elapsed / 1000.0f);
        mLastPresenceUpdate = now;
        mPresenceUpdatePending = false;
        mGameModeUpdateReady = false;
        mPresenceUpdateDelay = false;
        mCurrentPresenceString = text;
    }
    else
    {
        DebugPrintf("Failed!!!!\n");
    }
    unsigned int resultValue = static_cast<NNResult>(result).value;
    unsigned int level = (resultValue >> 27) & 3;
    unsigned int moduleMask = level == 3 ? 0x07f00000 : 0x1ff00000;
    unsigned int descriptionMask = level == 3 ? 0x3ff : 0xfffff;
    if ((resultValue & moduleMask) == 0x00c00000 && (resultValue & descriptionMask) > 0x31ff
        && (resultValue & descriptionMask) < 0x1f400)
    {
        unsigned int errorCode = nn::fp::ResultToErrorCode(result);
        gProfileInstance->DebugPrintf("\"%s @%d\" failed. errorCode = %03u-%04u\n", "SetPresenceString", 197,
                                      errorCode / 10000, errorCode % 10000);
    }
}

void CProfile::SetRichPresenceContextValue(int context, int value, int setting)
{
    if (context == 0 && mContextValues[0][value] != setting)
    {
        mContextValues[0][value] = setting;
        SetPresenceString(0);
    }
}

void CProfile::SetCurrentGameActivity(int activity, int presence, bool force)
{
    if (activity == 0)
    {
        if (mCurrentGameActivity[0] != presence)
        {
            mCurrentGameActivity[0] = presence;
            SetPresenceString(0);
        }
        if (force)
        {
            for (int quadrant = 1; quadrant < 4; ++quadrant)
            {
                if (mCurrentGameActivity[quadrant] != 0)
                {
                    mCurrentGameActivity[quadrant] = 0;
                    SetPresenceString(quadrant);
                }
            }
        }
    }
}

void CProfile::SetRichPresenceSettingFn(RichPresenceSettingHandler handler)
{
    mRichPresenceSettingHandler = handler;
}

void CProfile::SetRichPresenceGameModeData(nn::fp::GameMode& gameMode)
{
    if (std::memcmp(&mGameMode, &gameMode, sizeof(mGameMode)) == 0)
    {
        DebugPrintf("CProfile::SetRichPresenceGameModeData - data hasn't changed, not updating gamemode\n");
        return;
    }
    mGameMode = gameMode;
    mPresenceUpdateDelay = true;
    mLastGameModeUpdate = getSystemMilliseconds();
}

void CProfile::SetFriendPresenceChangeCallback(FriendPresenceHandler handler, void* context)
{
    mFriendPresenceHandler = handler;
    mFriendPresenceContext = context;
}

void CProfile::SetRichPresenceGetStringCallback(RichPresenceStringHandler handler)
{
    mRichPresenceStringHandler = handler;
}

void CProfile::RichPresenceOnCreate()
{
    mRichPresenceTitleId = 0;
    mRichPresenceSettingHandler = NULL;
    mFriendPresenceHandler = NULL;
}

void CProfile::TickRichPresence()
{
    if (mPresenceUpdateDelay && getSystemMilliseconds() - mLastGameModeUpdate > 2000)
    {
        DebugPrintf("Setting m_bGameModeUpdateReady true\n");
        mPresenceUpdateDelay = false;
        mGameModeUpdateReady = true;
    }
    if (mPresenceUpdatePending || mGameModeUpdateReady)
        SetPresenceString(GetPrimaryPad());
}

int CProfile::GetPrimaryPad()
{
    return mPrimaryPad;
}

void CProfile::SetPrimaryPad(int pad)
{
    g_pInputInstance->LockPad(pad, 0);
    mPrimaryPad = 0;
}

void CProfile::ClearGameUsers()
{
    g_pInputInstance->UnlockPad(0);
    for (int quadrant = 1; quadrant < 4; ++quadrant)
    {
        if (IsSignedIn(quadrant))
            SignOutUser(quadrant);
    }
}

bool CProfile::IsFullVersion()
{
    return true;
}

void CProfile::SetFullVersion(bool fullVersion)
{
    mFullVersion = fullVersion;
}

void CProfile::SetSignInChangeCallback(SignInChangeHandler handler, void* context)
{
    mSignInChangeHandler = handler;
    mSignInChangeContext = context;
}

void CProfile::SetNotificationsCallback(NotificationsHandler handler, void* context)
{
    mNotificationsHandler = handler;
    mNotificationsContext = context;
}

bool CProfile::RegionIsNorthAmerica()
{
    return false;
}

bool CProfile::LocaleIsUSorCanada()
{
    return false;
}

int CProfile::GetLiveConnectionStatus()
{
    return mLiveConnectionStatus;
}

bool CProfile::IsSystemUIDisplayed()
{
    return mSystemUIShowing;
}

void CProfile::SetProfileReadErrorCallback(ProfileReadErrorHandler handler, void* context)
{
    mProfileReadErrorHandler = handler;
    mProfileReadErrorContext = context;
}

void CProfile::SetUpsellType(eUpsellType type, int upsellParam)
{
    mUpsellType = type;
    mUpsellParam = upsellParam;
}

void CProfile::DisplayFullVersionPurchase(bool fromTrial, int, int upsellParam)
{
    SetUpsellType(eUpsellType_None, upsellParam);
    if (fromTrial)
        mUpsellFromTrial = true;
    gProfileInstance->SetProfileProcessState(eProfileProcessState_Upsell);
}

void CProfile::SetUpsellCallback(UpsellHandler handler, void* context)
{
    mUpsellHandler = handler;
    mUpsellContext = context;
}

void CProfile::SetDebugFullOverride(bool enabled)
{
    mDebugFullOverride = enabled;
}

void CProfile::DebugPrintf(const char*, ...)
{
}

void CProfile::SystemOnCreate()
{
    mDebugFullOverride = true;
    mSystemUIShowing = false;
}

void CProfile::SetSysUIShowing(bool showing)
{
    mSystemUIShowing = showing;
}

void CProfile::CheckForSystemNotifications()
{
}

void CProfile::SetLeaderboardDisable(bool disabled)
{
    mLeaderboardDisable = disabled;
}

void CProfile::CallSignInChangeCallback(bool primaryChanged)
{
    if (mSignInChangeHandler != NULL)
        mSignInChangeHandler(mSignInChangeContext, primaryChanged, mSignedInUsersMask);
}

void CProfile::TickSystem()
{
}

bool CProfile::AddAwardManagerTask(CAchievements* achievements)
{
    OSFastMutex_Lock(&mAwardMutex);
    mAwardQueue.push_back(achievements);
    OSFastMutex_Unlock(&mAwardMutex);
    return true;
}

int CProfile::GetRegistrationId(int achievementId) const
{
    return mAwardRegistrations[achievementId].registrationId;
}

void CProfile::RegisterAward(int achievementId, int registrationId, eAwardType, int callbackValue, int callbackContext)
{
    AwardRegistration& registration = mAwardRegistrations[achievementId];
    registration.registrationId = registrationId;
    registration.callbackValue = callbackValue;
    registration.callbackContext = callbackContext;
}

int CProfile::GetAwardId(int achievementId)
{
    return achievementId;
}

eAwardType CProfile::GetAwardType(int)
{
    return eAwardType_None;
}

bool CProfile::CanBeAwarded(int, int)
{
    return true;
}

void CProfile::OnSignout(int quadrant)
{
    if (quadrant >= 0 && quadrant < 4 && IsValidPlayer(quadrant))
    {
        mAchievements.insert(mAchievementsByPad[quadrant]);
        mAchievementsByPad[quadrant] = NULL;
    }
}

void CProfile::OnSignin(int quadrant)
{
    if (quadrant < 0 || quadrant >= 4 || !IsValidPlayer(quadrant))
        return;

    PlayerUID uid = getPlayerUID(quadrant, false);
    CAchievements*& achievements = mAchievementsByPad[quadrant];
    if (achievements == NULL)
    {
        achievements = new CAchievements(quadrant);
    }
    else if (achievements->mPlayerUID != uid)
    {
        OnSignout(quadrant);
        OnSignin(quadrant);
    }
}

CAchievements* CProfile::getAchievements(int quadrant)
{
    if (quadrant < 0 || quadrant >= 4)
        return NULL;
    OnSignin(quadrant);
    return mAchievementsByPad[quadrant];
}

CAchievements* CProfile::getAchievementsByPad(int quadrant) const
{
    if (quadrant < 0 || quadrant >= 4)
        return NULL;
    return mAchievementsByPad[quadrant];
}

bool CProfile::callAwardManagerCallback(int quadrant, int achievementId) const
{
    CAchievements* achievements = getAchievementsByPad(quadrant);
    if (achievements != NULL && achievements->mIsSystemPlayer)
    {
        if (mAwardHandler != NULL)
        {
            const AwardRegistration& registration = mAwardRegistrations[achievementId];
            mAwardHandler(mAwardContext, quadrant, achievementId, registration.callbackValue,
                          registration.callbackContext);
            return true;
        }
        return false;
    }
    return false;
}

void CProfile::Award(int quadrant, int achievementId, bool)
{
    PlayerUID uid = getPlayerUID(quadrant, false);
    CAchievements* achievements = getAchievements(quadrant);
    if (achievementId < 0 || achievementId >= 128 || achievements == NULL)
    {
        std::wstring uidString = uid.toString();
        DebugPrintf("<CProfile::IsAwardsFlagSet> Cannot award trophy.Player '%ls', iQuad=%i, iAward=%i, pAch=%p.\n",
                    uidString.c_str(), quadrant, achievementId, achievements);
        return;
    }
    if (!achievements->IsIdle())
    {
        achievements->addPendingAward(achievementId);
        return;
    }
    if (!achievements->SetAwardedBit(achievementId, true))
        callAwardManagerCallback(quadrant, achievementId);
}

bool CProfile::IsAwardsFlagSet(int quadrant, int achievementId)
{
    PlayerUID uid = getPlayerUID(quadrant, false);
    CAchievements* achievements = getAchievements(quadrant);
    return achievements != NULL && achievementId >= 0 && achievementId < 128
           && achievements->GetAwardedBit(achievementId);
}

void CProfile::SetOnAwardHandler(AwardHandler handler, void* context)
{
    mAwardHandler = handler;
    mAwardContext = context;
}

CAchievements* CProfile::nextAwardManagerTask()
{
    CAchievements* achievements = NULL;
    OSFastMutex_Lock(&mAwardMutex);
    if (!mAwardQueue.empty())
    {
        achievements = mAwardQueue.front();
        mAwardQueue.pop_front();
    }
    OSFastMutex_Unlock(&mAwardMutex);
    return achievements;
}

int CProfile::openAwardManagerFile(int& file, const char* mode, unsigned char slot, const char* filename)
{
    FSFileHandle handle = static_cast<FSFileHandle>(file);
    FSStatus result = SAVEOpenFile(&mFileSystemClient, &mFileSystemCommand, slot, filename, mode, &handle,
                                   FS_ERROR_FLAG_ALL);
    std::memcpy(&file, &handle, sizeof(file));
    return result;
}

void CProfile::EnterInitSaveDirCS()
{
    EnterCriticalSection(&mInitSaveDirCriticalSection);
}

void CProfile::LeaveInitSaveDirCS()
{
    LeaveCriticalSection(&mInitSaveDirCriticalSection);
}

int CProfile::initSaveDirectory(unsigned char slot)
{
    nn::act::GetNumOfAccounts();
    unsigned int slotMask = 1U << slot;
    if ((mInitialisedSaveDirectories & slotMask) == 0)
    {
        mInitialisedSaveDirectories |= slotMask;
        if (slot != nn::act::GetSlotNo())
        {
            EnterInitSaveDirCS();
            int result = SAVEInitSaveDir(slot);
            LeaveInitSaveDirCS();
            if (result != 0)
                return false;
        }
    }
    return true;
}

void CProfile::closeAwardManagerFile(int& file)
{
    if (file != -1)
        FSCloseFile(&mFileSystemClient, &mFileSystemCommand, file, FS_ERROR_FLAG_ALL);
}

void CProfile::flushSaveDirectory(unsigned char slot)
{
    nn::act::GetNumOfAccounts();
    SAVEFlushQuota(&mFileSystemClient, &mFileSystemCommand, slot, FS_ERROR_FLAG_ALL);
}

int CProfile::AwardManagerThreadProc(int, void* argument)
{
    CProfile* profile = (CProfile*)argument;
    gProfileInstance->DebugPrintf("<CAwardManager> Thread Starting: %p.\n", argument);
    for (;;)
    {
        OSSleepTicks(OSMillisecondsToTicks(70));
        CAchievements* achievements = profile->nextAwardManagerTask();
        if (achievements == NULL)
            continue;

        const char* filename = achievements->GetFilename();
        unsigned int state = achievements->GetState();
        int file = -1;
        if (state == 2)
        {
            profile->initSaveDirectory(achievements->mSlotId);
            if (profile->openAwardManagerFile(file, "w+", achievements->mSlotId, filename) == 0)
            {
                uint8_t* data = static_cast<uint8_t*>(allocAligned());
                achievements->mHasAchievementsFile = true;
                achievements->WriteTo(data, 16);
                FSWriteFile(&profile->mFileSystemClient, &profile->mFileSystemCommand, data, 1, 16, file, 0,
                            FS_ERROR_FLAG_ALL);
                profile->closeAwardManagerFile(file);
                MEMFreeToDefaultHeap(data);
                if (achievements->mSlotId != nn::act::GetSlotNo())
                    profile->flushSaveDirectory(achievements->mSlotId);
            }
            else
            {
                achievements->WriteTo(NULL, 0);
                profile->closeAwardManagerFile(file);
            }
        }
        else if (state == 1)
        {
            profile->initSaveDirectory(achievements->mSlotId);
            if (profile->openAwardManagerFile(file, "r", achievements->mSlotId, filename) == 0)
            {
                FSStat stat;
                if (FSGetStatFile(&profile->mFileSystemClient, &profile->mFileSystemCommand, file, &stat,
                                  FS_ERROR_FLAG_ALL)
                    == 0)
                {
                    uint8_t* data = static_cast<uint8_t*>(allocAligned());
                    int bytesRead = FSReadFile(&profile->mFileSystemClient, &profile->mFileSystemCommand, data, 1, 16,
                                               file, 0, FS_ERROR_FLAG_ALL);
                    achievements->mHasAchievementsFile = true;
                    achievements->ReadFrom(data, bytesRead);
                    profile->closeAwardManagerFile(file);
                    MEMFreeToDefaultHeap(data);
                    continue;
                }
            }
            achievements->ReadFrom(NULL, 0);
            profile->closeAwardManagerFile(file);
        }
    }
}

void CProfile::AwardManagerOnCreate()
{
    mAwardHandler = NULL;
    mAwardContext = NULL;
    OSFastMutex_Init(&mAwardMutex, "AwardManager");
    FSInitCmdBlock(&mFileSystemCommand);
    FSAddClient(&mFileSystemClient, FS_ERROR_FLAG_ALL);
    FSStateChangeParams notification = {stateChangeCallback, NULL, 0};
    FSSetStateChangeNotification(&mFileSystemClient, &notification);
    mAwardThreadStack = MEMAllocFromDefaultHeap(60000);
    mAwardThreadStackTop = (unsigned char*)mAwardThreadStack + 60000;
    if (C4JThreadImpl::OSCreateThread4J(&mAwardThread, AwardManagerThreadProc, 0, this, mAwardThreadStackTop, 60000, 16,
                                        7)
        != 0)
        OSResumeThread(&mAwardThread);
    for (int index = 0; index < 128; ++index)
    {
        mAwardRegistrations[index].registrationId = -1;
        mAwardRegistrations[index].callbackValue = -1;
        mAwardRegistrations[index].callbackContext = -1;
    }
}

void CProfile::AwardManagerTick()
{
    std::set<CAchievements*>::iterator deleting = mAchievements.begin();
    while (deleting != mAchievements.end())
    {
        CAchievements* achievements = *deleting;
        if (!achievements->IsIdle())
        {
            ++deleting;
        }
        else if (achievements->hasPendingAwards())
        {
            std::set<int> awardsNotRegistered;
            achievements->awardPending(awardsNotRegistered);
            ++deleting;
        }
        else if (achievements->IsDirty())
        {
            achievements->StartWrite();
            ++deleting;
        }
        else
        {
            mAchievements.erase(deleting++);
        }
    }

    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        CAchievements* achievements = getAchievements(quadrant);
        if (achievements == NULL)
            continue;
        if (achievements->hasPendingAwards())
        {
            std::set<int> awardsNotRegistered;
            achievements->awardPending(awardsNotRegistered);
            for (std::set<int>::const_iterator award = awardsNotRegistered.begin(); award != awardsNotRegistered.end();
                 ++award)
                callAwardManagerCallback(quadrant, *award);
        }
        else if (achievements->IsIdle())
        {
            achievements->StartWrite();
        }
    }
}

bool CProfile::initSaveDirectoryForQuadrant(int quadrant, unsigned char* slot)
{
    *slot = (unsigned char)getSlotId(quadrant);
    return initSaveDirectory(*slot);
}
