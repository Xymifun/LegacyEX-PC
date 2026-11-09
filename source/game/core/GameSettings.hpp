#ifndef GAME_CORE_GAMESETTINGS_HPP
#define GAME_CORE_GAMESETTINGS_HPP
#include <stdint.h>
#include <string>
#include "platform/wiiu/C4JStorage.hpp"
#include "game/core/GameModeOptions.hpp"
enum eGameSetting
{
    eGameSetting_MusicVolume,
    eGameSetting_SoundVolume,
    eGameSetting_Gamma,
    eGameSetting_Difficulty,
    eGameSetting_GameSensitivity,
    eGameSetting_InterfaceSensitivity,
    eGameSetting_ViewBob,
    eGameSetting_ControlScheme,
    eGameSetting_InvertLook,
    eGameSetting_SouthPaw,
    eGameSetting_SecondaryControls,
    eGameSetting_SplitScreenVertical,
    eGameSetting_DisplayMii,
    eGameSetting_Autosave,
    eGameSetting_SplitscreenNicknames,
    eGameSetting_Hints,
    eGameSetting_InterfaceOpacity,
    eGameSetting_Tooltips,
    eGameSetting_Clouds,
    eGameSetting_Online,
    eGameSetting_InviteOnly,
    eGameSetting_FriendsOfFriends,
    eGameSetting_DisplayUpdateMessage,
    eGameSetting_DisplayHUD,
    eGameSetting_DisplayHand,
    eGameSetting_CustomSkinAnim,
    eGameSetting_DeathMessages,
    eGameSetting_HUDSize,
    eGameSetting_HUDSizeSplitscreen,
    eGameSetting_AnimatedCharacter,
    eGameSetting_EULAAccepted,
    eGameSetting_Reserved,
    eGameSetting_ClassicCrafting,
    eGameSetting_CaveSounds,
    eGameSetting_GameChat,
    eGameSetting_MinecartSounds,
    eGameSetting_PublicMiniGamePrompts,
    eGameSetting_UpdateMessageSeen,
    eGameSetting_DisplayGameMessages,
    eGameSetting_DisplaySaveIcon,
    eGameSetting_FlyingViewRolling,
    eGameSetting_ControlMappings,
    eGameSetting_ShowGlideGhost,
    eGameSetting_ShowGlideGhostPath,
    eGameSetting_AutoJump,
    eGameSetting_SkinAdjustments
#ifdef MINECRAFT_PC
    ,
    eGameSetting_PublicWorldTitle
#endif
};
enum EControllerActions
{
    EControllerActions_MenuA = 0,
    EControllerActions_MenuB,
    EControllerActions_MenuX,
    EControllerActions_MenuY,
    EControllerActions_MenuUp,
    EControllerActions_MenuDown,
    EControllerActions_MenuRight,
    EControllerActions_MenuLeft,
    EControllerActions_MenuPageUp,
    EControllerActions_MenuPageDown,
    EControllerActions_MenuRightScroll,
    EControllerActions_MenuLeftScroll,
    EControllerActions_MenuStickPress,
    EControllerActions_MenuOtherStickPress,
    EControllerActions_MenuOtherStickUp,
    EControllerActions_MenuOtherStickDown,
    EControllerActions_MenuOtherStickLeft,
    EControllerActions_MenuOtherStickRight,
    EControllerActions_MenuStart,
    EControllerActions_MenuBack,
    EControllerActions_MenuOk,
    EControllerActions_MenuCancel,
    EControllerActions_Pause,
    EControllerActions_Scoreboard,
    EControllerActions_MoveForward,
    EControllerActions_MoveBackward,
    EControllerActions_MoveLeft,
    EControllerActions_MoveRight,
    EControllerActions_LookUp,
    EControllerActions_LookDown,
    EControllerActions_LookLeft,
    EControllerActions_LookRight,
    EControllerActions_Jump,
    EControllerActions_Use,
    EControllerActions_Action,
    EControllerActions_CycleHeldItemLeft,
    EControllerActions_CycleHeldItemRight,
    EControllerActions_Inventory,
    EControllerActions_Drop,
    EControllerActions_Sneak,
    EControllerActions_Crafting,
    EControllerActions_ChangeCameraMode,
    EControllerActions_FlyLeft,
    EControllerActions_FlyRight,
    EControllerActions_FlyUp,
    EControllerActions_FlyDown,
    EControllerActions_Sprint,
    EControllerActions_PickBlock,
    EControllerActions_SpectatePrevious,
    EControllerActions_SpectateNext,
    EControllerActions_SpectateNoise,
    EControllerActions_SpectateCancel,
    EControllerActions_ConfirmReady,
    EControllerActions_Vote,
    EControllerActions_RestartSection,
    EControllerActions_RestartRace,
    EControllerActions_LookBehind,
    EControllerActions_AltFlyLeft,
    EControllerActions_AltFlyRight,
    EControllerActions_AltFlyUp,
    EControllerActions_AltFlyDown
};
enum EJoypadButton
{
    EJoypadButton_Face1 = 1,
    EJoypadButton_Face2 = 2,
    EJoypadButton_Face3 = 4,
    EJoypadButton_Face4 = 8,
    EJoypadButton_Start = 16,
    EJoypadButton_Back = 32,
    EJoypadButton_RightBumper = 64,
    EJoypadButton_LeftBumper = 128,
    EJoypadButton_RightStickPress = 256,
    EJoypadButton_LeftStickPress = 512,
    EJoypadButton_DPadUp = 1024,
    EJoypadButton_DPadDown = 2048,
    EJoypadButton_DPadLeft = 4096,
    EJoypadButton_DPadRight = 8192,
    EJoypadButton_LeftStickRight = 16384,
    EJoypadButton_LeftStickLeft = 32768,
    EJoypadButton_RightStickRight = 65536,
    EJoypadButton_RightStickLeft = 131072,
    EJoypadButton_RightStickDown = 262144,
    EJoypadButton_RightStickUp = 524288,
    EJoypadButton_LeftStickDown = 1048576,
    EJoypadButton_LeftStickUp = 2097152,
    EJoypadButton_RightTrigger = 4194304,
    EJoypadButton_LeftTrigger = 8388608
};
enum eDebugSetting
{
    eDebugSetting_First = 0
};
class GameSettings
{
    unsigned char mChanged;
    unsigned char mMusicVolume;
    unsigned char mSoundVolume;
    unsigned char mGameSensitivity;
    unsigned char mGamma;
    unsigned char mSkinFlags;
    unsigned short mPackedSettings;
    unsigned int mUnknown0x08;
    unsigned char mTutorialFlags[64];
    unsigned int mSkinId;
    unsigned char mInterfaceSensitivity;
    unsigned char mInterfaceOpacity;
    unsigned short mReserved;
    unsigned int mSettingFlags;
    unsigned int mSpecialTutorialFlags;
    unsigned int mCapeId;
    unsigned int mFavoriteSkins[10];
    unsigned char mFavoriteSkinsPos;
    unsigned char mFavoriteSkinsCount;
    unsigned short mReserved2;
    unsigned int mHiddenMashupWorlds;
    unsigned char mMinecraftLanguage;
    unsigned char mMinecraftLocale;
    unsigned short mReserved3;
    unsigned int mGameModeOptions;
    unsigned char mHostOptionVersion;
    unsigned char mReserved4[3];
    unsigned int mBattleMiniGameMap;
    unsigned int mLoggingMask;
    unsigned char mActionMappings[25];
    unsigned char mUnknownBD;
    unsigned char mUnknownBE;
    unsigned char mUnknownBF;
    unsigned int mGlideMiniGameMap;

public:
    static uint32_t GetGameSettings(int, eGameSetting);
    static uint8_t GetActionMapping(int, int);
    static void SetActionMapping(int, int, unsigned char);
    static void UpdateActionMapping(int profile, int scheme, bool swapPrimaryButtons);
    static uint32_t GetInputButton(unsigned int);
    static uint32_t GetActionVal(unsigned char);
    static void ActionGameSettings(int, eGameSetting);
    static void SetSettingsChanged(int);
    static void SetGameSettings(int, eGameSetting, unsigned char);
    static uint8_t GetMinecraftLanguage(int);
    static uint8_t GetMinecraftLocale(int);
    static void SetDefaultActionMapping(unsigned char scheme, int action, unsigned char value);
    static void SetSecondaryActionMapping(unsigned char scheme, int action, unsigned char value);
    static void SetCustomisableControllerMapping(int profile, GameSettings* settings);
    void updateToLatestVersionFrom(unsigned short, int);
    static int OldProfileVersionCallback(void*, unsigned char*, unsigned short, int);
    static void SetDefaultControls(C4JStorage::PROFILESETTINGS*, int, bool);
    static void SetDefaultGameOptions(C4JStorage::PROFILESETTINGS*, int, bool);
    static void InitGameSettings();
    static void ClearGameSettingsChangedFlag(int);
    static void ApplyGameSettingsChanged(int);
    static void CheckGameSettingsChanged(bool, int);
    static uint32_t GetInputButton(int profile, bool secondary, EControllerActions action);
    static void SetPlayerCape(int, unsigned int, bool);
    static void SetPlayerCape(int, const std::wstring&);
    static std::wstring GetPlayerCapeName(int);
    static uint32_t IsAllowedSkin(int);
    static uint32_t GetPlayerCapeId(int);
    static void SetPlayerSkin(int, unsigned int, bool);
    static void SetPlayerSkin(int, const std::wstring&);
    static std::wstring GetPlayerSkinName(int);
    static uint32_t GetPlayerSkinId(int);
    static void SetPlayerFavoriteSkin(int, int, unsigned int);
    static uint32_t GetPlayerFavoriteSkin(int, int);
    static uint8_t GetPlayerFavoriteSkinsPos(int);
    static uint32_t GetPlayerFavoriteSkinsPos(int, unsigned int);
    static void SetPlayerFavoriteSkinsPos(int, int);
    static uint32_t GetPlayerFavoriteSkinsCount(int);
    static void ValidateFavoriteSkins(int);
    static void HideMashupPackWorld(int, unsigned int);
    static void SetBattleMiniGameMapSelection(int, unsigned int);
    static uint32_t GetBattleMiniGameMapSelection(int);
    static void SetGlideMiniGameMapSelection(int, unsigned int);
    static uint32_t GetGlideMiniGameMapSelection(int);
    static uint32_t GetTumbleMiniGameMapSelection();
    static void SetMinecraftLanguage(int, unsigned char);
    static void SetMinecraftLocale(int, unsigned char);
    static void SetGameSettings(int profile, eCustomGameModeHostOption option, unsigned int value);
    static uint32_t GetGameSettings(eGameSetting setting);
    static uint32_t GetGameSettingsDebugMask(int, bool);
    static uint32_t IsDebugSettingEnabled(eDebugSetting, int);
    static void SetGameSettingsDebugMask(int, unsigned int);
    static uint32_t GetLoggingMask();
    static void SetSpecialTutorialCompletionFlag(int, int);
    static void SetTutorialCompletionFlag(int, int);
    static bool GetTutorialCompletionFlag(int, int);
    static void ActionDebugMask(int, bool);
    GameSettings();
    ~GameSettings();
    static void EnableMashupPackWorlds(int);
    static uint32_t GetMashupPackWorlds(int);
    static uint64_t GetGameSettings(int, eCustomGameModeHostOption);
    static void SetLoggingMask(unsigned int);
    static void ResetAllTutorialCompletionFlags(int);
};

#endif
