#include "game/core/GameSettings.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
extern C4JStorage* gStorage;

GameSettings* gGameSettings[4] = {NULL, NULL, NULL, NULL};
unsigned char gDefaultActionMappings[3][25];
unsigned char gSecondaryActionMappings[3][25];
extern CInput* g_pInput;

void DefineActions()
{
    GameSettings::SetDefaultActionMapping(0, 0, 2);
    GameSettings::SetDefaultActionMapping(0, 1, 12);
    GameSettings::SetDefaultActionMapping(0, 2, 11);
    GameSettings::SetDefaultActionMapping(0, 3, 10);
    GameSettings::SetDefaultActionMapping(0, 4, 9);
    GameSettings::SetDefaultActionMapping(0, 5, 4);
    GameSettings::SetDefaultActionMapping(0, 6, 1);
    GameSettings::SetDefaultActionMapping(0, 7, 13);
    GameSettings::SetDefaultActionMapping(0, 8, 3);
    GameSettings::SetDefaultActionMapping(0, 9, 14);
    GameSettings::SetDefaultActionMapping(0, 10, 5);
    GameSettings::SetDefaultActionMapping(0, 11, 6);
    GameSettings::SetDefaultActionMapping(0, 12, 7);
    GameSettings::SetDefaultActionMapping(0, 13, 8);
    GameSettings::SetDefaultActionMapping(0, 14, 0);
    GameSettings::SetDefaultActionMapping(0, 15, 0);
    GameSettings::SetDefaultActionMapping(0, 16, 10);
    GameSettings::SetDefaultActionMapping(0, 17, 9);
    GameSettings::SetDefaultActionMapping(0, 18, 11);
    GameSettings::SetDefaultActionMapping(0, 19, 1);
    GameSettings::SetDefaultActionMapping(0, 20, 3);
    GameSettings::SetDefaultActionMapping(0, 21, 1);
    GameSettings::SetDefaultActionMapping(0, 22, 3);
    GameSettings::SetDefaultActionMapping(0, 23, 1);
    GameSettings::SetDefaultActionMapping(0, 24, 4);
    GameSettings::SetSecondaryActionMapping(0, 0, 2);
    GameSettings::SetSecondaryActionMapping(0, 1, 12);
    GameSettings::SetSecondaryActionMapping(0, 2, 11);
    GameSettings::SetSecondaryActionMapping(0, 3, 10);
    GameSettings::SetSecondaryActionMapping(0, 4, 9);
    GameSettings::SetSecondaryActionMapping(0, 5, 4);
    GameSettings::SetSecondaryActionMapping(0, 6, 1);
    GameSettings::SetSecondaryActionMapping(0, 7, 8);
    GameSettings::SetSecondaryActionMapping(0, 8, 3);
    GameSettings::SetSecondaryActionMapping(0, 9, 7);
    GameSettings::SetSecondaryActionMapping(0, 10, 0);
    GameSettings::SetSecondaryActionMapping(0, 11, 0);
    GameSettings::SetSecondaryActionMapping(0, 12, 0);
    GameSettings::SetSecondaryActionMapping(0, 13, 0);
    GameSettings::SetSecondaryActionMapping(0, 14, 0);
    GameSettings::SetSecondaryActionMapping(0, 15, 0);
    GameSettings::SetSecondaryActionMapping(0, 16, 10);
    GameSettings::SetSecondaryActionMapping(0, 17, 9);
    GameSettings::SetSecondaryActionMapping(0, 18, 11);
    GameSettings::SetSecondaryActionMapping(0, 19, 1);
    GameSettings::SetSecondaryActionMapping(0, 20, 3);
    GameSettings::SetSecondaryActionMapping(0, 21, 1);
    GameSettings::SetSecondaryActionMapping(0, 22, 3);
    GameSettings::SetSecondaryActionMapping(0, 23, 1);
    GameSettings::SetSecondaryActionMapping(0, 24, 4);
    GameSettings::SetDefaultActionMapping(1, 0, 9);
    GameSettings::SetDefaultActionMapping(1, 1, 11);
    GameSettings::SetDefaultActionMapping(1, 2, 12);
    GameSettings::SetDefaultActionMapping(1, 3, 5);
    GameSettings::SetDefaultActionMapping(1, 4, 6);
    GameSettings::SetDefaultActionMapping(1, 5, 4);
    GameSettings::SetDefaultActionMapping(1, 6, 1);
    GameSettings::SetDefaultActionMapping(1, 7, 14);
    GameSettings::SetDefaultActionMapping(1, 8, 3);
    GameSettings::SetDefaultActionMapping(1, 9, 10);
    GameSettings::SetDefaultActionMapping(1, 10, 5);
    GameSettings::SetDefaultActionMapping(1, 11, 6);
    GameSettings::SetDefaultActionMapping(1, 12, 7);
    GameSettings::SetDefaultActionMapping(1, 13, 8);
    GameSettings::SetDefaultActionMapping(1, 14, 0);
    GameSettings::SetDefaultActionMapping(1, 15, 0);
    GameSettings::SetDefaultActionMapping(1, 16, 6);
    GameSettings::SetDefaultActionMapping(1, 17, 6);
    GameSettings::SetDefaultActionMapping(1, 18, 12);
    GameSettings::SetDefaultActionMapping(1, 19, 1);
    GameSettings::SetDefaultActionMapping(1, 20, 3);
    GameSettings::SetDefaultActionMapping(1, 21, 1);
    GameSettings::SetDefaultActionMapping(1, 22, 3);
    GameSettings::SetDefaultActionMapping(1, 23, 1);
    GameSettings::SetDefaultActionMapping(1, 24, 4);
    GameSettings::SetSecondaryActionMapping(1, 0, 9);
    GameSettings::SetSecondaryActionMapping(1, 1, 11);
    GameSettings::SetSecondaryActionMapping(1, 2, 12);
    GameSettings::SetSecondaryActionMapping(1, 3, 5);
    GameSettings::SetSecondaryActionMapping(1, 4, 6);
    GameSettings::SetSecondaryActionMapping(1, 5, 4);
    GameSettings::SetSecondaryActionMapping(1, 6, 1);
    GameSettings::SetSecondaryActionMapping(1, 7, 8);
    GameSettings::SetSecondaryActionMapping(1, 8, 3);
    GameSettings::SetSecondaryActionMapping(1, 9, 10);
    GameSettings::SetSecondaryActionMapping(1, 10, 0);
    GameSettings::SetSecondaryActionMapping(1, 11, 0);
    GameSettings::SetSecondaryActionMapping(1, 12, 0);
    GameSettings::SetSecondaryActionMapping(1, 13, 0);
    GameSettings::SetSecondaryActionMapping(1, 14, 0);
    GameSettings::SetSecondaryActionMapping(1, 15, 0);
    GameSettings::SetSecondaryActionMapping(1, 16, 6);
    GameSettings::SetSecondaryActionMapping(1, 17, 6);
    GameSettings::SetSecondaryActionMapping(1, 18, 12);
    GameSettings::SetSecondaryActionMapping(1, 19, 1);
    GameSettings::SetSecondaryActionMapping(1, 20, 3);
    GameSettings::SetSecondaryActionMapping(1, 21, 1);
    GameSettings::SetSecondaryActionMapping(1, 22, 3);
    GameSettings::SetSecondaryActionMapping(1, 23, 1);
    GameSettings::SetSecondaryActionMapping(1, 24, 4);
    GameSettings::SetDefaultActionMapping(2, 0, 12);
    GameSettings::SetDefaultActionMapping(2, 1, 11);
    GameSettings::SetDefaultActionMapping(2, 2, 2);
    GameSettings::SetDefaultActionMapping(2, 3, 5);
    GameSettings::SetDefaultActionMapping(2, 4, 6);
    GameSettings::SetDefaultActionMapping(2, 5, 4);
    GameSettings::SetDefaultActionMapping(2, 6, 1);
    GameSettings::SetDefaultActionMapping(2, 7, 10);
    GameSettings::SetDefaultActionMapping(2, 8, 3);
    GameSettings::SetDefaultActionMapping(2, 9, 14);
    GameSettings::SetDefaultActionMapping(2, 10, 5);
    GameSettings::SetDefaultActionMapping(2, 11, 6);
    GameSettings::SetDefaultActionMapping(2, 12, 7);
    GameSettings::SetDefaultActionMapping(2, 13, 8);
    GameSettings::SetDefaultActionMapping(2, 14, 0);
    GameSettings::SetDefaultActionMapping(2, 15, 0);
    GameSettings::SetDefaultActionMapping(2, 16, 5);
    GameSettings::SetDefaultActionMapping(2, 17, 6);
    GameSettings::SetDefaultActionMapping(2, 18, 2);
    GameSettings::SetDefaultActionMapping(2, 19, 1);
    GameSettings::SetDefaultActionMapping(2, 20, 3);
    GameSettings::SetDefaultActionMapping(2, 21, 1);
    GameSettings::SetDefaultActionMapping(2, 22, 3);
    GameSettings::SetDefaultActionMapping(2, 23, 1);
    GameSettings::SetDefaultActionMapping(2, 24, 4);
    GameSettings::SetSecondaryActionMapping(2, 0, 12);
    GameSettings::SetSecondaryActionMapping(2, 1, 11);
    GameSettings::SetSecondaryActionMapping(2, 2, 2);
    GameSettings::SetSecondaryActionMapping(2, 3, 5);
    GameSettings::SetSecondaryActionMapping(2, 4, 6);
    GameSettings::SetSecondaryActionMapping(2, 5, 4);
    GameSettings::SetSecondaryActionMapping(2, 6, 1);
    GameSettings::SetSecondaryActionMapping(2, 7, 10);
    GameSettings::SetSecondaryActionMapping(2, 8, 3);
    GameSettings::SetSecondaryActionMapping(2, 9, 7);
    GameSettings::SetSecondaryActionMapping(2, 10, 0);
    GameSettings::SetSecondaryActionMapping(2, 11, 0);
    GameSettings::SetSecondaryActionMapping(2, 12, 0);
    GameSettings::SetSecondaryActionMapping(2, 13, 0);
    GameSettings::SetSecondaryActionMapping(2, 14, 0);
    GameSettings::SetSecondaryActionMapping(2, 15, 0);
    GameSettings::SetSecondaryActionMapping(2, 16, 5);
    GameSettings::SetSecondaryActionMapping(2, 17, 6);
    GameSettings::SetSecondaryActionMapping(2, 18, 2);
    GameSettings::SetSecondaryActionMapping(2, 19, 1);
    GameSettings::SetSecondaryActionMapping(2, 20, 3);
    GameSettings::SetSecondaryActionMapping(2, 21, 1);
    GameSettings::SetSecondaryActionMapping(2, 22, 3);
    GameSettings::SetSecondaryActionMapping(2, 23, 1);
    GameSettings::SetSecondaryActionMapping(2, 24, 4);
    int controller;
    for (controller = 0; controller < 2; ++controller)
    {
        if (g_pInput->IsCircleCrossSwapped() != 0)
        {
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuA, EJoypadButton_Face1);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOk, EJoypadButton_Face1);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuB, EJoypadButton_Face2);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuCancel, EJoypadButton_Face2);
        }
        else
        {
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuA, EJoypadButton_Face2);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOk, EJoypadButton_Face2);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuB, EJoypadButton_Face1);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuCancel, EJoypadButton_Face1);
        }
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuX, EJoypadButton_Face3);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuY, EJoypadButton_Face4);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuUp,
                                                  EJoypadButton_LeftStickUp | EJoypadButton_DPadUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuDown,
                                                  EJoypadButton_LeftStickDown | EJoypadButton_DPadDown);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuLeft,
                                                  EJoypadButton_LeftStickLeft | EJoypadButton_DPadLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuRight,
                                                  EJoypadButton_LeftStickRight | EJoypadButton_DPadRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuPageUp, EJoypadButton_LeftTrigger);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuPageDown,
                                                  EJoypadButton_RightTrigger);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuRightScroll,
                                                  EJoypadButton_RightBumper);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuLeftScroll,
                                                  EJoypadButton_LeftBumper);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuStart, EJoypadButton_Start);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuBack, EJoypadButton_Back);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOtherStickUp,
                                                  EJoypadButton_RightStickLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOtherStickDown,
                                                  EJoypadButton_RightStickRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOtherStickLeft,
                                                  EJoypadButton_RightStickUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOtherStickRight,
                                                  EJoypadButton_RightStickDown);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Jump, EJoypadButton_Face2);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MoveForward,
                                                  EJoypadButton_LeftStickUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MoveBackward,
                                                  EJoypadButton_LeftStickDown);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MoveLeft, EJoypadButton_LeftStickLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MoveRight,
                                                  EJoypadButton_LeftStickRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_LookUp, EJoypadButton_RightStickUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_LookDown,
                                                  EJoypadButton_RightStickDown);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_LookLeft,
                                                  EJoypadButton_RightStickLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_LookRight,
                                                  EJoypadButton_RightStickRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Use, EJoypadButton_LeftTrigger);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Action, EJoypadButton_RightTrigger);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_CycleHeldItemRight,
                                                  EJoypadButton_RightBumper);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_CycleHeldItemLeft,
                                                  EJoypadButton_LeftBumper);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Inventory, EJoypadButton_Face4);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Pause, EJoypadButton_Start);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Drop, EJoypadButton_Face1);
        if (controller == 0)
        {
            g_pInput->SetGameControllerTypeJoypadMaps(0, EControllerActions_Sneak, EJoypadButton_RightStickPress);
            g_pInput->SetGameControllerTypeJoypadMaps(0, EControllerActions_ChangeCameraMode,
                                                      EJoypadButton_LeftStickPress);
            g_pInput->SetGameControllerTypeJoypadMaps(0, EControllerActions_MenuStickPress,
                                                      EJoypadButton_LeftStickPress);
            g_pInput->SetGameControllerTypeJoypadMaps(0, EControllerActions_MenuOtherStickPress,
                                                      EJoypadButton_RightStickPress);
        }
        else
        {
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Sneak, EJoypadButton_DPadDown);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_ChangeCameraMode,
                                                      EJoypadButton_DPadUp);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuStickPress,
                                                      EJoypadButton_LeftStickPress);
            g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_MenuOtherStickPress,
                                                      EJoypadButton_RightStickPress);
        }
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Crafting, EJoypadButton_Face3);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_Scoreboard, EJoypadButton_Back);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_FlyLeft, EJoypadButton_DPadLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_FlyRight, EJoypadButton_DPadRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_FlyUp, EJoypadButton_DPadUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_FlyDown, EJoypadButton_DPadDown);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_AltFlyLeft, EJoypadButton_DPadLeft);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_AltFlyRight, EJoypadButton_DPadRight);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_AltFlyUp, EJoypadButton_DPadUp);
        g_pInput->SetGameControllerTypeJoypadMaps(controller, EControllerActions_AltFlyDown, EJoypadButton_DPadDown);
    }
}

uint32_t GameSettings::GetGameSettings(int profile, eGameSetting setting)
{
    if (profile == 255)
        profile = gProfile->GetPrimaryPad();
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        return gGameSettings[profile]->mMusicVolume;
    case eGameSetting_SoundVolume:
        return gGameSettings[profile]->mSoundVolume;
    case eGameSetting_Gamma:
        return gGameSettings[profile]->mGamma;
    case eGameSetting_Difficulty:
        return gGameSettings[profile]->mPackedSettings & 3;
    case eGameSetting_GameSensitivity:
        return gGameSettings[profile]->mGameSensitivity;
    case eGameSetting_ViewBob:
        return (gGameSettings[profile]->mPackedSettings >> 2) & 1;
    case eGameSetting_DisplayMii:
        return (gGameSettings[profile]->mPackedSettings >> 3) & 1;
    case eGameSetting_ControlScheme:
        return (gGameSettings[profile]->mPackedSettings >> 4) & 3;
    case eGameSetting_InvertLook:
        return (gGameSettings[profile]->mPackedSettings >> 6) & 1;
    case eGameSetting_SouthPaw:
        return (gGameSettings[profile]->mPackedSettings >> 7) & 1;
    case eGameSetting_SplitScreenVertical:
        return (gGameSettings[profile]->mPackedSettings >> 8) & 1;
    case eGameSetting_InterfaceSensitivity:
        return gGameSettings[profile]->mInterfaceSensitivity;
    case eGameSetting_SplitscreenNicknames:
        return (gGameSettings[profile]->mPackedSettings >> 9) & 1;
    case eGameSetting_Hints:
        return (gGameSettings[profile]->mPackedSettings >> 10) & 1;
    case eGameSetting_Autosave:
        return (gGameSettings[profile]->mPackedSettings >> 11) & 15;
    case eGameSetting_Tooltips:
        return (gGameSettings[profile]->mPackedSettings >> 15) & 1;
    case eGameSetting_InterfaceOpacity:
        return gGameSettings[profile]->mInterfaceOpacity;
    case eGameSetting_Clouds:
        return gGameSettings[profile]->mSettingFlags & 1;
    case eGameSetting_Online:
        return (gGameSettings[profile]->mSettingFlags >> 1) & 1;
    case eGameSetting_InviteOnly:
        return (gGameSettings[profile]->mSettingFlags >> 2) & 1;
    case eGameSetting_FriendsOfFriends:
        return (gGameSettings[profile]->mSettingFlags >> 3) & 1;
    case eGameSetting_DisplayUpdateMessage:
        return (gGameSettings[profile]->mSettingFlags >> 4) & 3;
    case eGameSetting_DisplayHUD:
        return (gGameSettings[profile]->mSettingFlags >> 7) & 1;
#ifdef MINECRAFT_PC
    case eGameSetting_PublicWorldTitle:
        return (gGameSettings[profile]->mSettingFlags >> 6) & 1;
#endif
    case eGameSetting_DisplayHand:
        return (gGameSettings[profile]->mSettingFlags >> 8) & 1;
    case eGameSetting_CustomSkinAnim:
        return (gGameSettings[profile]->mSettingFlags >> 9) & 1;
    case eGameSetting_DeathMessages:
        return (gGameSettings[profile]->mSettingFlags >> 10) & 1;
    case eGameSetting_HUDSize:
        return (gGameSettings[profile]->mSettingFlags >> 11) & 3;
    case eGameSetting_HUDSizeSplitscreen:
        return (gGameSettings[profile]->mSettingFlags >> 13) & 3;
    case eGameSetting_AnimatedCharacter:
        return (gGameSettings[profile]->mSettingFlags >> 15) & 1;
    case eGameSetting_EULAAccepted:
        return (gGameSettings[profile]->mSettingFlags >> 16) & 1;
    case eGameSetting_Reserved:
        return (gGameSettings[profile]->mSettingFlags >> 17) & 1;
    case eGameSetting_ClassicCrafting:
        return (gGameSettings[profile]->mSettingFlags >> 18) & 1;
    case eGameSetting_CaveSounds:
        return (gGameSettings[profile]->mSettingFlags >> 19) & 1;
    case eGameSetting_GameChat:
        return (gGameSettings[profile]->mSettingFlags >> 20) & 1;
    case eGameSetting_MinecartSounds:
        return (gGameSettings[profile]->mSettingFlags >> 21) & 1;
    case eGameSetting_ShowGlideGhost:
        return (gGameSettings[profile]->mSettingFlags >> 22) & 1;
    case eGameSetting_SecondaryControls:
        return (gGameSettings[profile]->mSettingFlags >> 23) & 1;
    case eGameSetting_PublicMiniGamePrompts:
        return (gGameSettings[profile]->mSettingFlags >> 24) & 3;
    case eGameSetting_AutoJump:
        return (gGameSettings[profile]->mSettingFlags >> 26) & 1;
    case eGameSetting_UpdateMessageSeen:
        return (gGameSettings[profile]->mSettingFlags >> 27) & 1;
    case eGameSetting_DisplayGameMessages:
        return (gGameSettings[profile]->mSettingFlags >> 28) & 1;
    case eGameSetting_DisplaySaveIcon:
        return (gGameSettings[profile]->mSettingFlags >> 29) & 1;
    case eGameSetting_FlyingViewRolling:
        return (gGameSettings[profile]->mSettingFlags >> 30) & 1;
    case eGameSetting_ShowGlideGhostPath:
        return gGameSettings[profile]->mSettingFlags >> 31;
    case eGameSetting_SkinAdjustments:
        return gGameSettings[profile]->mSkinFlags & 15;
    case eGameSetting_ControlMappings:
        return 0;
    default:
        return 0;
    }
}
uint8_t GameSettings::GetActionMapping(int profile, int action)
{
    return gGameSettings[profile]->mActionMappings[action];
}
void GameSettings::SetActionMapping(int profile, int action, unsigned char value)
{
    gGameSettings[profile]->mActionMappings[action] = value;
}

void GameSettings::UpdateActionMapping(int profile, int scheme, bool swapPrimaryButtons)
{
    for (int action = 0; action < 25; ++action)
    {
        unsigned char value = g_pInput->IsPadClassicController(profile) ? gSecondaryActionMappings[scheme][action]
                                                                        : gDefaultActionMappings[scheme][action];
        if (swapPrimaryButtons)
        {
            if (value == 1)
                value = 2;
            else if (value == 2)
                value = 1;
        }
        SetActionMapping(profile, action, value);
    }
}

uint32_t GameSettings::GetInputButton(unsigned int value)
{
    if (value == 0)
        return 0;
    if (value > 2)
    {
        static const unsigned int buttons[15] = {0,
                                                 0,
                                                 0,
                                                 EJoypadButton_Face3,
                                                 EJoypadButton_Face4,
                                                 EJoypadButton_DPadLeft,
                                                 EJoypadButton_DPadRight,
                                                 EJoypadButton_DPadUp,
                                                 EJoypadButton_DPadDown,
                                                 EJoypadButton_RightBumper,
                                                 EJoypadButton_LeftBumper,
                                                 EJoypadButton_RightTrigger,
                                                 EJoypadButton_LeftTrigger,
                                                 EJoypadButton_RightStickPress,
                                                 EJoypadButton_LeftStickPress};
        if (value > 14)
            return 0;
        return buttons[value];
    }
    return value;
}
uint32_t GameSettings::GetActionVal(unsigned char value)
{
    return GetInputButton(value & 15) | GetInputButton((value >> 4) & 255);
}
void GameSettings::ActionGameSettings(int profile, eGameSetting setting)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::music,
                                     static_cast<float>(gGameSettings[profile]->mMusicVolume) / 100.0f);
        return;
    case eGameSetting_SoundVolume:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::sound,
                                     static_cast<float>(gGameSettings[profile]->mSoundVolume) / 100.0f);
        return;
    case eGameSetting_Gamma:
        if (profile == gProfile->GetPrimaryPad())
        {
            const float gamma = static_cast<float>(gGameSettings[profile]->mGamma) * 327.68f;
            gRenderer->UpdateGamma(static_cast<unsigned short>(static_cast<int>(gamma) & 0xffff));
        }
        return;
    case eGameSetting_Difficulty:
        if (profile == gProfile->GetPrimaryPad())
        {
            minecraft->mOptions->toggle(Options::Option::difficulty, gGameSettings[profile]->mPackedSettings & 3);
            gApp.SetGameHostOption(eGameHostOption_Difficulty, minecraft->mOptions->mDifficulty);
        }
        return;
    case eGameSetting_GameSensitivity:
        minecraft->mOptions->set(Options::Option::sensitivity,
                                 static_cast<float>(gGameSettings[profile]->mGameSensitivity) / 100.0f);
        return;
    case eGameSetting_SouthPaw:
    {
        const bool swapAnalogSticks = (gGameSettings[profile]->mPackedSettings & 0x80) != 0;
        if (!swapAnalogSticks)
        {
            g_pInput->SetJoypadStickAxisMap(profile, 0, 0);
            g_pInput->SetJoypadStickAxisMap(profile, 1, 1);
            g_pInput->SetJoypadStickAxisMap(profile, 2, 2);
            g_pInput->SetJoypadStickAxisMap(profile, 3, 3);
            g_pInput->SetJoypadStickTriggerMap(profile, 0, 0);
            g_pInput->SetJoypadStickTriggerMap(profile, 1, 1);
        }
        else
        {
            g_pInput->SetJoypadStickAxisMap(profile, 0, 2);
            g_pInput->SetJoypadStickAxisMap(profile, 1, 3);
            g_pInput->SetJoypadStickAxisMap(profile, 2, 0);
            g_pInput->SetJoypadStickAxisMap(profile, 3, 1);
            g_pInput->SetJoypadStickTriggerMap(profile, 0, 1);
            g_pInput->SetJoypadStickTriggerMap(profile, 1, 0);
        }
    }
        return;
    case eGameSetting_SplitScreenVertical:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->updatePlayerViewportAssignments();
        return;
    case eGameSetting_DisplayMii:
        if (minecraft->mLevel != NULL && CGameNetworkManager::sInstance.IsHost()
            && profile == gProfile->GetPrimaryPad())
        {
            gApp.SetGameHostOption(eGameHostOption_Gamertags, (gGameSettings[profile]->mPackedSettings >> 3) & 1);
            gApp.SetXuiServerAction(profile, eXuiServerAction_ServerSettingChanged, NULL);
            PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->getPlayers()->begin();
                 it != players->getPlayers()->end(); ++it)
            {
                boost::shared_ptr<ServerPlayer> player = *it;
                player->mShowOnMaps = gApp.GetGameHostOption(eGameHostOption_Gamertags) != 0;
            }
        }
        return;
    case eGameSetting_SplitscreenNicknames:
    {
        if (minecraft->mLocalPlayers[0])
            gConsoleUIController.DisplayGamertag(0, minecraft->mLocalPlayers[0]->mViewport != 0);
        if (minecraft->mLocalPlayers[1])
            gConsoleUIController.DisplayGamertag(1, minecraft->mLocalPlayers[1]->mViewport != 0);
        if (minecraft->mLocalPlayers[2])
            gConsoleUIController.DisplayGamertag(2, minecraft->mLocalPlayers[2]->mViewport != 0);
        if (minecraft->mLocalPlayers[3])
            gConsoleUIController.DisplayGamertag(3, minecraft->mLocalPlayers[3]->mViewport != 0);
    }
        return;
    case eGameSetting_InterfaceOpacity:
        gConsoleUIController.RefreshTooltips(static_cast<unsigned int>(profile));
        return;
    case eGameSetting_Tooltips:
        gConsoleUIController.SetEnableTooltips(static_cast<unsigned int>(profile),
                                               (gGameSettings[profile]->mPackedSettings & 0x8000) != 0);
        return;
    case eGameSetting_ControlMappings:
    {
        const int scheme = static_cast<int>(GetGameSettings(profile, eGameSetting_ControlScheme));
        const bool swapPrimaryButtons = GetGameSettings(profile, eGameSetting_SecondaryControls) != 0;
        if (scheme > 0 || swapPrimaryButtons)
        {
            UpdateActionMapping(profile, scheme, swapPrimaryButtons);
            SetGameSettings(profile, eGameSetting_ControlScheme, 0);
            SetGameSettings(profile, eGameSetting_SecondaryControls, 0);
        }
        for (int action = 0; action < 25; ++action)
        {
            g_pInput->SetGameJoypadMaps(profile, static_cast<unsigned char>(EControllerActions_Jump + action),
                                        GetActionVal(GetActionMapping(profile, action)));
        }
    }
        return;
    default:
        return;
    }
}
void GameSettings::SetSettingsChanged(int profile)
{
    gGameSettings[profile]->mChanged = 1;
}
template <typename T>
void setGameSettingsHelper(int profile, eGameSetting setting, unsigned char value, T* field, T mask, unsigned int shift)
{
    const T oldValue = *field;
    const T newValue = static_cast<T>(value << shift);
    if ((mask & (oldValue ^ newValue)) != 0)
    {
        *field = static_cast<T>((oldValue & ~mask) | newValue);
        if (static_cast<unsigned int>(setting) > 3 || profile == gProfile->GetPrimaryPad())
            GameSettings::ActionGameSettings(profile, setting);
        GameSettings::SetSettingsChanged(profile);
    }
}

void GameSettings::SetGameSettings(int profile, eGameSetting setting, unsigned char value)
{
    GameSettings* gameSettings = gGameSettings[profile];
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mMusicVolume, 0xff, 0);
        break;
    case eGameSetting_SoundVolume:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mSoundVolume, 0xff, 0);
        break;
    case eGameSetting_Gamma:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mGamma, 0xff, 0);
        break;
    case eGameSetting_Difficulty:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x3, 0);
        break;
    case eGameSetting_GameSensitivity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mGameSensitivity, 0xff, 0);
        break;
    case eGameSetting_ViewBob:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x4, 2);
        break;
    case eGameSetting_ControlScheme:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x30, 4);
        break;
    case eGameSetting_InvertLook:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x40, 6);
        break;
    case eGameSetting_SouthPaw:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x80, 7);
        break;
    case eGameSetting_SplitScreenVertical:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x100, 8);
        break;
    case eGameSetting_DisplayMii:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x8, 3);
        break;
    case eGameSetting_InterfaceSensitivity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mInterfaceSensitivity, 0xff, 0);
        break;
    case eGameSetting_SplitscreenNicknames:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x200, 9);
        break;
    case eGameSetting_Hints:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x400, 10);
        break;
    case eGameSetting_Autosave:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x7800, 11);
        break;
    case eGameSetting_Tooltips:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x8000, 15);
        break;
    case eGameSetting_InterfaceOpacity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mInterfaceOpacity, 0xff, 0);
        break;
    case eGameSetting_Clouds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x1, 0);
        break;
    case eGameSetting_Online:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x2, 1);
        break;
    case eGameSetting_InviteOnly:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x4, 2);
        break;
    case eGameSetting_FriendsOfFriends:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8, 3);
        break;
    case eGameSetting_DisplayUpdateMessage:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x30, 4);
        break;
    case eGameSetting_DisplayHUD:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80, 7);
        break;
#ifdef MINECRAFT_PC
    case eGameSetting_PublicWorldTitle:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x40, 6);
        break;
#endif
    case eGameSetting_DisplayHand:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x100, 8);
        break;
    case eGameSetting_CustomSkinAnim:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x200, 9);
        break;
    case eGameSetting_DeathMessages:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x400, 10);
        break;
    case eGameSetting_HUDSize:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x1800, 11);
        break;
    case eGameSetting_HUDSizeSplitscreen:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x6000, 13);
        break;
    case eGameSetting_AnimatedCharacter:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8000, 15);
        break;
    case eGameSetting_EULAAccepted:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x10000, 16);
        break;
    case eGameSetting_Reserved:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x20000, 17);
        break;
    case eGameSetting_ClassicCrafting:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x40000, 18);
        break;
    case eGameSetting_CaveSounds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80000, 19);
        break;
    case eGameSetting_GameChat:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x100000, 20);
        break;
    case eGameSetting_MinecartSounds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x200000, 21);
        break;
    case eGameSetting_ShowGlideGhost:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x400000, 22);
        break;
    case eGameSetting_SecondaryControls:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x800000, 23);
        break;
    case eGameSetting_PublicMiniGamePrompts:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x3000000, 24);
        break;
    case eGameSetting_AutoJump:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x4000000, 26);
        break;
    case eGameSetting_UpdateMessageSeen:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8000000, 27);
        break;
    case eGameSetting_DisplayGameMessages:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x10000000, 28);
        break;
    case eGameSetting_DisplaySaveIcon:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x20000000, 29);
        break;
    case eGameSetting_FlyingViewRolling:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x40000000, 30);
        break;
    case eGameSetting_ShowGlideGhostPath:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80000000, 31);
        break;
    case eGameSetting_SkinAdjustments:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mSkinFlags, 0xf, 0);
        break;
    case eGameSetting_ControlMappings:
        break;
    default:
        break;
    }
}

uint8_t GameSettings::GetMinecraftLanguage(int profile)
{
    if (gGameSettings[profile] == NULL)
        return 0;
    return gGameSettings[profile]->mMinecraftLanguage;
}

uint8_t GameSettings::GetMinecraftLocale(int profile)
{
    if (gGameSettings[profile] == NULL)
        return 0;
    return gGameSettings[profile]->mMinecraftLocale;
}

void GameSettings::SetDefaultActionMapping(unsigned char scheme, int action, unsigned char value)
{
    gDefaultActionMappings[scheme][action] = value;
}

void GameSettings::SetSecondaryActionMapping(unsigned char scheme, int action, unsigned char value)
{
    gSecondaryActionMappings[scheme][action] = value;
}
void GameSettings::SetCustomisableControllerMapping(int profile, GameSettings* settings)
{
    for (int action = 0; action < 25; ++action)
    {
        settings->mActionMappings[action] = g_pInput->IsPadClassicController(profile)
                                                ? gSecondaryActionMappings[0][action]
                                                : gDefaultActionMappings[0][action];
    }
}
void GameSettings::updateToLatestVersionFrom(unsigned short version, int profile)
{
    switch (version)
    {
    case 10:
        mHiddenMashupWorlds = 0xffffffffu;
        updateToLatestVersionFrom(11, profile);
        break;
    case 11:
        mSettingFlags &= ~0x10000u;
        updateToLatestVersionFrom(12, profile);
        break;
    case 12:
        mMinecraftLanguage = 0;
        updateToLatestVersionFrom(13, profile);
        break;
    case 13:
    case 14:
    case 15:
    case 16:
        mSettingFlags |= 0x80000u;
        updateToLatestVersionFrom(17, profile);
        break;
    case 17:
        mSettingFlags &= ~0x40u;
        updateToLatestVersionFrom(18, profile);
        break;
    case 18:
    case 19:
    case 20:
        mSettingFlags |= 0x200000u;
        updateToLatestVersionFrom(21, profile);
        break;
    case 21:
    case 22:
        mSettingFlags |= 0x3000000u;
        mBattleMiniGameMap = 0xffffffffu;
        updateToLatestVersionFrom(24, profile);
        break;
    case 23:
        mBattleMiniGameMap = 0xffffffc0u;
        updateToLatestVersionFrom(24, profile);
        break;
    case 24:
        mSettingFlags |= 0x30000000u;
        updateToLatestVersionFrom(25, profile);
        break;
    case 25:
        updateToLatestVersionFrom(27, profile);
        break;
    case 26:
        mSettingFlags &= ~0x40000000u;
        updateToLatestVersionFrom(27, profile);
        break;
    case 27:
    case 28:
        SetCustomisableControllerMapping(profile, this);
        updateToLatestVersionFrom(29, profile);
    case 29:
        mSettingFlags |= 0x80400000u;
        updateToLatestVersionFrom(30, profile);
        break;
    case 30:
        mSettingFlags &= ~0x4000000u;
        mGlideMiniGameMap = 0xffffffffu;
        updateToLatestVersionFrom(31, profile);
        break;
    case 31:
        mSkinFlags = 9;
        updateToLatestVersionFrom(32, profile);
        break;
    case 32:
    case 33:
        mSettingFlags |= 0x30u;
        updateToLatestVersionFrom(34, profile);
        break;
    }
}

int GameSettings::OldProfileVersionCallback(void*, unsigned char* data, unsigned short version, int profile)
{
    if (version < 15)
    {
        StatsCounter* stats = Minecraft::GetInstance()->mStats[profile];
        stats->clear();
        stats->parseOld(version, data);
        stats->save(profile, true, data);
    }
    reinterpret_cast<GameSettings*>(data)->updateToLatestVersionFrom(version, profile);
    return 0;
}
void GameSettings::SetDefaultControls(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, eGameSetting_InvertLook, static_cast<unsigned char>(values->values[0] != 0));
    SetGameSettings(profile, eGameSetting_SouthPaw, values->flags);

    for (int action = 0; action < 25; ++action)
    {
        unsigned char value = g_pInput->IsPadClassicController(profile) ? gSecondaryActionMappings[0][action]
                                                                        : gDefaultActionMappings[0][action];
        SetActionMapping(profile, action, value);
        g_pInput->SetGameJoypadMaps(profile, static_cast<unsigned char>(EControllerActions_Jump + action),
                                    GetActionVal(value));
    }
    gGameSettings[profile]->mChanged = changed;
}
void GameSettings::SetDefaultGameOptions(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, eGameSetting_MusicVolume, 100);
    SetGameSettings(profile, eGameSetting_SoundVolume, 100);
    SetGameSettings(profile, eGameSetting_Gamma, 0x32);

    if (Minecraft::GetInstance()->mLevel == NULL)
    {
        SetGameSettings(profile, eGameSetting_Difficulty, 1);
    }

    SetGameSettings(profile, eGameSetting_GameSensitivity, 100);
    SetGameSettings(profile, eGameSetting_ViewBob, 1);
    SetGameSettings(profile, eGameSetting_ControlScheme, 0);
    SetGameSettings(profile, eGameSetting_SplitScreenVertical, 0);
    SetGameSettings(profile, eGameSetting_DisplayMii, 1);
    SetGameSettings(profile, eGameSetting_InterfaceSensitivity, 100);
    SetGameSettings(profile, eGameSetting_SplitscreenNicknames, 1);
    SetGameSettings(profile, eGameSetting_Hints, 1);
    SetGameSettings(profile, eGameSetting_Autosave, 2);
    SetGameSettings(profile, eGameSetting_Tooltips, 1);
    SetGameSettings(profile, eGameSetting_InterfaceOpacity, 0x50);
    SetGameSettings(profile, eGameSetting_Clouds, 1);
    SetGameSettings(profile, eGameSetting_Online, 1);
    SetGameSettings(profile, eGameSetting_InviteOnly, 0);
#ifdef MINECRAFT_PC
    SetGameSettings(profile, eGameSetting_PublicWorldTitle, 0);
#endif
    SetGameSettings(profile, eGameSetting_FriendsOfFriends, 0);
    SetGameSettings(profile, eGameSetting_DisplayHUD, 1);
    SetGameSettings(profile, eGameSetting_DisplayHand, 1);
    SetGameSettings(profile, eGameSetting_CustomSkinAnim, 1);
    SetGameSettings(profile, eGameSetting_DeathMessages, 1);
    SetGameSettings(profile, eGameSetting_HUDSize, 1);
    SetGameSettings(profile, eGameSetting_HUDSizeSplitscreen, 2);
    SetGameSettings(profile, eGameSetting_AnimatedCharacter, 1);

    GameSettings* current = gGameSettings[profile];
    for (int i = 0; i < 10; ++i)
    {
        current->mFavoriteSkins[i] = 0xffffffffu;
    }
    current->mFavoriteSkinsPos = 0;
    current->mHiddenMashupWorlds = 0xffffffffu;

    SetGameSettings(profile, eGameSetting_EULAAccepted, 0);
    if (gApp.GetGameStarted() == 0)
    {
        gGameSettings[profile]->mMinecraftLocale = 0;
        gGameSettings[profile]->mMinecraftLanguage = 0;
    }

    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(eMiniGameId_Battle, true);
    GameModeOptions hostOptions = miniGame->GetDefaultPlayerSettings().mHostOptions;
    gGameSettings[profile]->mGameModeOptions = hostOptions.Get(eCustomGameModeHostOption_All);

    SetGameSettings(profile, eGameSetting_ClassicCrafting, 0);
    SetGameSettings(profile, eGameSetting_CaveSounds, 1);
    SetGameSettings(profile, eGameSetting_MinecartSounds, 1);
    SetGameSettings(profile, eGameSetting_GameChat, 0);
    gGameSettings[profile]->mBattleMiniGameMap = 0xffffffffu;
    SetGameSettings(profile, eGameSetting_SecondaryControls, 0);
    SetGameSettings(profile, eGameSetting_PublicMiniGamePrompts, 3);
    SetGameSettings(profile, eGameSetting_DisplayGameMessages, 1);
    SetGameSettings(profile, eGameSetting_DisplaySaveIcon, 1);
    SetGameSettings(profile, eGameSetting_FlyingViewRolling, 0);
    SetGameSettings(profile, eGameSetting_ShowGlideGhost, 1);
    SetGameSettings(profile, eGameSetting_ShowGlideGhostPath, 1);
    SetGameSettings(profile, eGameSetting_AutoJump, 0);
    gGameSettings[profile]->mSkinFlags = 9;
    gGameSettings[profile]->mGlideMiniGameMap = 0xffffffffu;
    SetDefaultControls(values, profile, changed);
}
void GameSettings::InitGameSettings()
{
    for (int profile = 0; profile < 4; ++profile)
    {
        gGameSettings[profile] = reinterpret_cast<GameSettings*>(gStorage->GetGameDefinedProfileData(profile));
        *reinterpret_cast<unsigned char*>(gGameSettings[profile]) = 0;
        SetDefaultGameOptions(gStorage->GetDashboardProfileSettings(profile), profile, false);
    }
}

void GameSettings::ClearGameSettingsChangedFlag(int profile)
{
    gGameSettings[profile]->mChanged = 0;
}
void GameSettings::ApplyGameSettingsChanged(int profile)
{
    ActionGameSettings(profile, eGameSetting_MusicVolume);
    ActionGameSettings(profile, eGameSetting_SoundVolume);
    ActionGameSettings(profile, eGameSetting_Gamma);
    ActionGameSettings(profile, eGameSetting_Difficulty);
    ActionGameSettings(profile, eGameSetting_GameSensitivity);
    ActionGameSettings(profile, eGameSetting_ViewBob);
    ActionGameSettings(profile, eGameSetting_ControlScheme);
    ActionGameSettings(profile, eGameSetting_InvertLook);
    ActionGameSettings(profile, eGameSetting_SouthPaw);
    ActionGameSettings(profile, eGameSetting_SecondaryControls);
    ActionGameSettings(profile, eGameSetting_SplitScreenVertical);
    ActionGameSettings(profile, eGameSetting_DisplayMii);
    ActionGameSettings(profile, eGameSetting_InterfaceSensitivity);
    ActionGameSettings(profile, eGameSetting_SplitscreenNicknames);
    ActionGameSettings(profile, eGameSetting_Hints);
    ActionGameSettings(profile, eGameSetting_InterfaceOpacity);
    ActionGameSettings(profile, eGameSetting_Tooltips);
    ActionGameSettings(profile, eGameSetting_Clouds);
    ActionGameSettings(profile, eGameSetting_DisplayHUD);
    ActionGameSettings(profile, eGameSetting_DisplayHand);
    ActionGameSettings(profile, eGameSetting_CustomSkinAnim);
    ActionGameSettings(profile, eGameSetting_DeathMessages);
    ActionGameSettings(profile, eGameSetting_HUDSize);
    ActionGameSettings(profile, eGameSetting_HUDSizeSplitscreen);
    ActionGameSettings(profile, eGameSetting_AnimatedCharacter);
    ActionGameSettings(profile, eGameSetting_EULAAccepted);
    ActionGameSettings(profile, eGameSetting_DisplayGameMessages);
    ActionGameSettings(profile, eGameSetting_DisplaySaveIcon);
    ActionGameSettings(profile, eGameSetting_FlyingViewRolling);
    ActionGameSettings(profile, eGameSetting_ShowGlideGhost);
    ActionGameSettings(profile, eGameSetting_ShowGlideGhostPath);
    ActionGameSettings(profile, eGameSetting_ControlMappings);
}

void GameSettings::CheckGameSettingsChanged(bool changed, int profile)
{
    if (profile == 255)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            CheckGameSettingsChanged(changed, i);
        }
        return;
    }
    if (gGameSettings[profile]->mChanged)
    {
        gStorage->WriteToProfile(profile, true, changed);
        gGameSettings[profile]->mChanged = 0;
    }
}
uint32_t GameSettings::GetInputButton(int profile, bool secondary, EControllerActions action)
{
    const unsigned int mapping = GetActionMapping(profile, action - EControllerActions_Jump);
    return GetInputButton(secondary ? ((mapping >> 4) & 255) : (mapping & 0x0f));
}

void GameSettings::SetPlayerCape(int profile, unsigned int id, bool changed)
{
    GameSettings* current = gGameSettings[profile];
    unsigned int oldId = current->mCapeId;
    current->mCapeId = id;
    if (changed)
        current->mChanged = 1;
    if (Minecraft::GetInstance()->mLocalPlayers[profile] != NULL)
        Minecraft::GetInstance()->mLocalPlayers[profile].get()->setAndBroadcastCustomCape(id);
    if (!changed)
        gGameSettings[profile]->mCapeId = oldId;
}
void GameSettings::SetPlayerCape(int profile, const std::wstring& path)
{
    SetPlayerCape(profile, Player::getCapeIdFromPath(path), true);
}
std::wstring GameSettings::GetPlayerCapeName(int profile)
{
    return Player::getCapePathFromId(gGameSettings[profile]->mCapeId);
}
uint32_t GameSettings::IsAllowedSkin(int profile)
{
    GameSettings* settings = gGameSettings[profile];
    if (static_cast<int32_t>(settings->mSkinId) >= 0)
        return 1;

    wchar_t skinPath[256];
    swprintf(skinPath, 256, L"dlcskin%08d.png", settings->mSkinId & 0x7fffffff);
    DLCPack* pack = gApp.mDLCManager.getPackContainingSkin(skinPath);
    if (pack != NULL)
    {
        DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(pack->getSkinFile(skinPath));
        bool smallSkinBlocked = false;
        const bool skinIsFree = skinFile->getParameterAsBool(DLCManager::eDLCParameterType_Free);
        const bool licensed = pack->hasPurchasedPack();
        if (Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
            && (skinFile->getSkinAdjustments().mGameFlags & 1) != 0)
            smallSkinBlocked = true;
        if ((skinIsFree || licensed) && !smallSkinBlocked)
            return 1;
    }
    return 0;
}
uint32_t GameSettings::GetPlayerCapeId(int profile)
{
    unsigned int id = 0;
    if (IsAllowedSkin(profile))
        id = gGameSettings[profile]->mCapeId;
    return id;
}

void GameSettings::SetPlayerSkin(int profile, unsigned int id, bool changed)
{
    GameSettings* current = gGameSettings[profile];
    unsigned int oldId = current->mSkinId;
    current->mSkinId = id;
    if (changed)
    {
        current->mChanged = 1;
        telemetryManagerInstance->RecordSkinChanged(profile, id);
    }
    if (Minecraft::GetInstance()->mLocalPlayers[profile] != NULL)
        Minecraft::GetInstance()->mLocalPlayers[profile].get()->setAndBroadcastCustomSkin(id);
    if (!changed)
        gGameSettings[profile]->mSkinId = oldId;
}
void GameSettings::SetPlayerSkin(int profile, const std::wstring& path)
{
    SetPlayerSkin(profile, CMinecraftApp::getSkinIdFromPath(path), true);
}
std::wstring GameSettings::GetPlayerSkinName(int profile)
{
    return CMinecraftApp::getSkinPathFromId(gGameSettings[profile]->mSkinId);
}
uint32_t GameSettings::GetPlayerSkinId(int profile)
{
    unsigned int id = 0;
    if (IsAllowedSkin(profile))
        id = gGameSettings[profile]->mSkinId;
    return id;
}

void GameSettings::SetPlayerFavoriteSkin(int profile, int index, unsigned int id)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mFavoriteSkins[index] = id;
}

uint32_t GameSettings::GetPlayerFavoriteSkin(int profile, int index)
{
    return gGameSettings[profile]->mFavoriteSkins[index];
}

uint8_t GameSettings::GetPlayerFavoriteSkinsPos(int profile)
{
    return gGameSettings[profile]->mFavoriteSkinsPos;
}

uint32_t GameSettings::GetPlayerFavoriteSkinsPos(int profile, unsigned int id)
{
    GameSettings* current = gGameSettings[profile];
    for (int i = 0; i < 10; ++i)
    {
        if (current->mFavoriteSkins[i] == id)
            return i;
    }
    return 0;
}

void GameSettings::SetPlayerFavoriteSkinsPos(int profile, int value)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mFavoriteSkinsPos = static_cast<unsigned char>(value);
}

uint32_t GameSettings::GetPlayerFavoriteSkinsCount(int profile)
{
    GameSettings* current = gGameSettings[profile];
    for (int i = 0; i < 10; ++i)
    {
        if (current->mFavoriteSkins[i] == 0xffffffffu)
            return i;
    }
    return 10;
}

void GameSettings::ValidateFavoriteSkins(int profile)
{
    uint32_t count = GetPlayerFavoriteSkinsCount(profile);
    uint32_t validCount = 0;
    for (uint32_t i = 0; i < count; ++i)
    {
        bool isDefaultSkin = false;
        bool hasPurchasedPack = false;
        DLCFile* skinFile = gApp.mDLCManager.getSkinFile(
            static_cast<int>(GetPlayerFavoriteSkin(profile, static_cast<int>(i))), isDefaultSkin, hasPurchasedPack);
        if (isDefaultSkin || (skinFile != NULL && (hasPurchasedPack || static_cast<DLCSkinFile*>(skinFile)->isFree())))
        {
            gGameSettings[profile]->mFavoriteSkins[validCount] = gGameSettings[profile]->mFavoriteSkins[i];
            ++validCount;
        }
    }
    for (uint32_t i = validCount; i < 10; ++i)
    {
        gGameSettings[profile]->mFavoriteSkins[i] = 0xffffffffu;
    }
}

void GameSettings::HideMashupPackWorld(int profile, unsigned int id)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mHiddenMashupWorlds &= ~(1u << (id - 0x400));
}

void GameSettings::SetBattleMiniGameMapSelection(int profile, unsigned int value)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mBattleMiniGameMap = value;
}
uint32_t GameSettings::GetBattleMiniGameMapSelection(int profile)
{
    return gGameSettings[profile]->mBattleMiniGameMap;
}
void GameSettings::SetGlideMiniGameMapSelection(int profile, unsigned int value)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mGlideMiniGameMap = value;
}
uint32_t GameSettings::GetGlideMiniGameMapSelection(int profile)
{
    return gGameSettings[profile]->mGlideMiniGameMap;
}
uint32_t GameSettings::GetTumbleMiniGameMapSelection()
{
    if (SharedConstants::IS_HALLOWEEN_SEASON)
        return 2;
    if (SharedConstants::IS_CHRISTMAS)
        return 4;
    return SharedConstants::IS_CHRISTMAS_SEASON ? 8 : 1;
}
void GameSettings::SetMinecraftLanguage(int profile, unsigned char value)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mMinecraftLanguage = value;
}
void GameSettings::SetMinecraftLocale(int profile, unsigned char value)
{
    GameSettings* current = gGameSettings[profile];
    current->mChanged = 1;
    current->mMinecraftLocale = value;
}
void GameSettings::SetGameSettings(int profile, eCustomGameModeHostOption option, unsigned int value)
{
    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(eMiniGameId_Battle, true);
    GameModeOptions options(gGameSettings[profile]->mGameModeOptions);
    if (option == eCustomGameModeHostOption_All)
        gGameSettings[profile]->mHostOptionVersion = miniGame->CurrentHostOptionVersion();
    if (options.Get(option) != value)
    {
        options.Set(option, value);
        GameSettings* current = gGameSettings[profile];
        current->mChanged = 1;
        current->mGameModeOptions = options.Get(eCustomGameModeHostOption_All);
    }
}
uint32_t GameSettings::GetGameSettings(eGameSetting setting)
{
    return GetGameSettings(gProfile->GetPrimaryPad(), setting);
}
uint32_t GameSettings::GetGameSettingsDebugMask(int, bool)
{
    return 0;
}
uint32_t GameSettings::IsDebugSettingEnabled(eDebugSetting, int)
{
    return 0;
}
void GameSettings::SetGameSettingsDebugMask(int, unsigned int)
{
}
uint32_t GameSettings::GetLoggingMask()
{
    int profile = gProfile->GetPrimaryPad();
    return gGameSettings[profile]->mLoggingMask;
}
void GameSettings::SetSpecialTutorialCompletionFlag(int profile, int flag)
{
    if (static_cast<unsigned int>(flag) >= 32)
        return;
    GameSettings* current = gGameSettings[profile];
    if (current == NULL)
        return;
    current->mSpecialTutorialFlags |= 1u << flag;
}

void GameSettings::SetTutorialCompletionFlag(int profile, int flag)
{
    GameSettings* current = gGameSettings[profile];
    current->mTutorialFlags[flag >> 3] |= 1 << (7 - flag % 8);
    current->mChanged = 1;
}

bool GameSettings::GetTutorialCompletionFlag(int profile, int flag)
{
    unsigned int mask = 1 << (7 - flag % 8);
    return (gGameSettings[profile]->mTutorialFlags[flag >> 3] & mask) == mask;
}
