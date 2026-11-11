#include "game/ui/UIScene_ControlsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/FJ_ControlsMenu.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcKeyBindings.hpp"
#include "platform/wiiu/CProfile.hpp"
#endif

extern ConsoleUIController gConsoleUIController;

#ifdef MINECRAFT_PC
namespace
{
const int KEY_ROW_FIRST = 100;

bool IsKeyRow(int id)
{
    return id >= KEY_ROW_FIRST && id < KEY_ROW_FIRST + PcKeyBindingCount();
}

int KeyRowSize()
{
    return gApp.GetLocalPlayerCount() > 1 ? 20 : 30;
}

std::wstring KeyRowText(int code)
{
    if (code == kPcKeyNone)
        return std::wstring(L" ") + L10N::GetString(IDS_CONTROLS_NONE);
    return PcKeyCodeMarkup(code, KeyRowSize());
}

std::wstring SharedBindingHelp(PcKeyAction action, int code)
{
    std::wstring others;
    for (int other = 0; other < PcKeyBindingCount(); ++other)
    {
        if (other == action || PcKeyBinding(static_cast<PcKeyAction>(other)) != code)
            continue;
        if (!others.empty())
            others += L", ";
        others += PcKeyBindingLabel(static_cast<PcKeyAction>(other));
    }
    if (others.empty())
        return std::wstring();
    return PcKeyCodeName(code) + L" is also bound to: " + others;
}
}
#endif

UIScene_ControlsMenu::UIScene_ControlsMenu(int player, void* data, UILayer* layer) : UIScene(player, layer)
{
    initialiseMovie();
    Init(player);
}

UIScene_ControlsMenu::~UIScene_ControlsMenu()
{
#ifdef MINECRAFT_PC
    if (m_keyboard && m_editingButton)
        PcInputCancelKeyCapture();
#endif
    if (m_mappingsChanged)
    {
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
        GameSettings::ActionGameSettings(m_playerId, eGameSetting_ControlMappings);
    }
}

std::wstring UIScene_ControlsMenu::GetControlsString(int player, EControllerActions action, bool& mapped)
{
    std::wstring result;
    if (GameSettings::GetInputButton(player, false, action) == 0 && action >= EControllerActions_Jump
        && action <= EControllerActions_LookBehind)
    {
        result = L" ";
        result += L10N::GetString(IDS_CONTROLS_NONE);
        mapped = false;
    }
    else
    {
        result = gApp.GetActionReplacement(player, (unsigned char)action);
        mapped = true;
    }
    return result;
}

void UIScene_ControlsMenu::SetControlsHelp(int, int control, std::wstring help)
{
    UIString text(L"");
    if (!help.empty())
    {
        HtmlString html(help, eMinecraftColour_HTMLColor_White, false, false, 0);
        text = UIString(gApp.FormatHTMLString(m_playerId, html.ToString(), -1));
    }
    std::wstring helpText(text.c_str());
    FJ_ControlsMenu* menu = static_cast<FJ_ControlsMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetControlsHelp(control, helpText);
}

void UIScene_ControlsMenu::Init(int player)
{
    bool notInGame = Minecraft::GetInstance()->mLevel == NULL;
    {
        std::wstring versionString = MiniGameDef::GetCustomGameModeById(eMiniGameId_Battle, true)->getVersionString();
        wchar_t* versionText = new wchar_t[128];
        std::swprintf(versionText, 128, L"%ls %ls %ls", L"1.12.2177.0", versionString.c_str(), L"");
        m_version.init(UIString(versionText));
        delete[] versionText;
    }

    bool canFly = false;
    if (!notInGame && Minecraft::GetInstance()->mLocalPlayers[m_playerId] != NULL)
    {
        if (Minecraft::GetInstance()->mLocalPlayers[m_playerId]->mAbilities.mMayfly)
            canFly = true;
    }
    m_canFly = canFly;

#ifdef MINECRAFT_PC
    m_keyboard = KeyboardMode();
    if (m_keyboard)
    {
        InitKeyboard(player);
        return;
    }
#endif

    bool mapped = true;
    std::wstring controlString = L"";
    m_controlsList.AddNewButton(L10N::GetString(IDS_RESET_TO_DEFAULTS), EControl_ResetToDefaults, true);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_MOVEMENT), EControl_Label);

    m_controlsList.AddNewCheckbox(L10N::GetString(IDS_INVERT_LOOK), EControl_InvertLook,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_InvertLook) != 0);
    m_controlsList.AddNewCheckbox(L10N::GetString(IDS_SOUTHPAW), EControl_SouthPaw,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) != 0);

    if (GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw))
    {
        controlString = GetControlsString(player, EControllerActions_MoveRight, mapped);
        m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOOK), controlString, mapped, EControl_Look);
        controlString = GetControlsString(player, EControllerActions_LookDown, mapped);
        m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_MOVE), controlString, mapped, EControl_Move);
    }
    else
    {
        controlString = GetControlsString(player, EControllerActions_LookDown, mapped);
        m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOOK), controlString, mapped, EControl_Look);
        controlString = GetControlsString(player, EControllerActions_MoveRight, mapped);
        m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_MOVE), controlString, mapped, EControl_Move);
    }

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_STANDARD), EControl_Label);

    controlString = GetControlsString(player, EControllerActions_Use, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_USE), controlString, mapped, EControl_Use);
    controlString = GetControlsString(player, EControllerActions_Action, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_ACTION), controlString, mapped, EControl_Action);
    controlString = GetControlsString(player, EControllerActions_Jump, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(m_canFly ? IDS_CONTROLS_JUMPFLY : IDS_CONTROLS_JUMP),
                                          controlString, mapped, EControl_Jump);
    controlString = GetControlsString(player, EControllerActions_Drop, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_DROP), controlString, mapped, EControl_Drop);
    controlString = GetControlsString(player, EControllerActions_Inventory, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_INVENTORY), controlString, mapped,
                                          EControl_Inventory);
    controlString = GetControlsString(player, EControllerActions_Crafting, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CRAFTING), controlString, mapped,
                                          EControl_Crafting);
    controlString = GetControlsString(player, EControllerActions_CycleHeldItemLeft, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_HELDITEM_LEFT), controlString, mapped,
                                          EControl_CycleHeldItemLeft);
    controlString = GetControlsString(player, EControllerActions_CycleHeldItemRight, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_HELDITEM_RIGHT), controlString, mapped,
                                          EControl_CycleHeldItemRight);
    controlString = GetControlsString(player, EControllerActions_Sneak, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(m_canFly ? IDS_CONTROLS_SNEAKFLY : IDS_CONTROLS_SNEAK),
                                          controlString, mapped, EControl_Sneak);
    controlString = GetControlsString(player, EControllerActions_Sprint, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPRINT), controlString, mapped, EControl_Sprint);
    controlString = GetControlsString(player, EControllerActions_PickBlock, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_PICKBLOCK), controlString, mapped,
                                          EControl_PickBlock);
    controlString = GetControlsString(player, EControllerActions_ChangeCameraMode, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_THIRDPERSON), controlString, mapped,
                                          EControl_ChangeCameraMode);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_CREATIVE), EControl_Label);

    controlString = GetControlsString(player, EControllerActions_FlyLeft, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CREATIVE_FLY_LEFT), controlString, mapped,
                                          EControl_FlyLeft);
    controlString = GetControlsString(player, EControllerActions_FlyRight, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CREATIVE_FLY_RIGHT), controlString, mapped,
                                          EControl_FlyRight);
    controlString = GetControlsString(player, EControllerActions_FlyUp, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CREATIVE_FLY_UP), controlString, mapped,
                                          EControl_FlyUp);
    controlString = GetControlsString(player, EControllerActions_FlyDown, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CREATIVE_FLY_DOWN), controlString, mapped,
                                          EControl_FlyDown);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_MG_LOBBY), EControl_Label);

    controlString = GetControlsString(player, EControllerActions_ConfirmReady, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOBBY_READY), controlString, mapped,
                                          EControl_ConfirmReady);
    controlString = GetControlsString(player, EControllerActions_Vote, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOBBY_VOTE_SELECT), controlString, mapped,
                                          EControl_Vote);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_SPECTATING), EControl_Label);

    controlString = GetControlsString(player, EControllerActions_SpectatePrevious, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_PREV), controlString, mapped,
                                          EControl_SpectatePrevious);
    controlString = GetControlsString(player, EControllerActions_SpectateNext, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_NEXT), controlString, mapped,
                                          EControl_SpectateNext);
    controlString = GetControlsString(player, EControllerActions_SpectateNoise, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_NOISE), controlString, mapped,
                                          EControl_SpectateNoise);
    controlString = GetControlsString(player, EControllerActions_SpectateCancel, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_CANCEL), controlString, mapped,
                                          EControl_SpectateCancel);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(IDS_CONTROLS_SECTION_GLIDE), EControl_Label);

    controlString = GetControlsString(player, EControllerActions_RestartSection, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_RESTART_SECTION), controlString, mapped,
                                          EControl_RestartSection);
    controlString = GetControlsString(player, EControllerActions_RestartRace, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_RESTART_RACE), controlString, mapped,
                                          EControl_RestartRace);
    controlString = GetControlsString(player, EControllerActions_LookBehind, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_LOOK_BEHIND), controlString, mapped,
                                          EControl_LookBehind);

    m_controlsList.EnableItem(EControl_Look, false, true);
    m_controlsList.EnableItem(EControl_Move, false, true);

    m_buttonCount = 0;
    m_firstButton = 0;
    m_editingButton = false;
    m_unusedEditFlag = false;
    m_updateControls = false;
    m_showAssignHelp = false;
    m_secondButton = 0;
    m_firstButtonMask = 0;
    m_clearHelp = false;
    m_mappingsChanged = false;
    m_lastEditAction = -1;

    SetControlsHelp(player, 0, L"");
    m_checkMapping = true;
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_ControlsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_version.setupControl(this, root, std::string("Version"));
        m_controls.push_back(&m_version);
    }
    {
        m_controlsList.setupControl(this, root, std::string("ControlsMultiList"));
        m_controls.push_back(&m_controlsList);
    }
    return 1;
}

void UIScene_ControlsMenu::updateTooltips()
{
#ifdef MINECRAFT_PC
    if (m_keyboard)
    {
        if (m_editingButton)
        {
            gConsoleUIController.SetTooltips(m_playerId, -1, IDS_TOOLTIPS_CANCEL, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                             -1, -1, -1, -1, -1, false, false);
            return;
        }
        const int selection = m_controlsList.getCurrentSelection();
        int tooltip = IDS_TOOLTIPS_SELECT;
        if (m_controlsList.getItemCount() == 0 || !m_controlsList.IsEnabled(selection))
            tooltip = -1;
        int unbind = -1;
        if (IsKeyRow(selection) && PcKeyBinding(static_cast<PcKeyAction>(selection - KEY_ROW_FIRST)) != kPcKeyNone)
            unbind = static_cast<int>(ePcString_Unbind);
        gConsoleUIController.SetTooltips(m_playerId, tooltip, IDS_TOOLTIPS_BACK, unbind, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, false, false);
        return;
    }
#endif
    if (m_editingButton)
    {
        gConsoleUIController.SetTooltips(m_playerId, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, IDS_TOOLTIPS_CANCEL, -1,
                                         -1, -1, -1, -1, false, false);
    }
    else if (m_controlsList.getItemCount() > 0)
    {
        int tooltip = IDS_TOOLTIPS_SELECT;
        if (!m_controlsList.IsEnabled(m_controlsList.getCurrentSelection()))
            tooltip = -1;
        gConsoleUIController.SetTooltips(m_playerId, tooltip, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, false, false);
    }
}

void UIScene_ControlsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

int UIScene_ControlsMenu::GetControlsAction(EControl control)
{
    static const unsigned char actions[] = {EControllerActions_Use,
                                            EControllerActions_Action,
                                            EControllerActions_Jump,
                                            EControllerActions_Drop,
                                            EControllerActions_Inventory,
                                            EControllerActions_Crafting,
                                            EControllerActions_CycleHeldItemLeft,
                                            EControllerActions_CycleHeldItemRight,
                                            EControllerActions_Sneak,
                                            EControllerActions_Sprint,
                                            EControllerActions_PickBlock,
                                            EControllerActions_ChangeCameraMode,
                                            EControllerActions_FlyLeft,
                                            EControllerActions_FlyRight,
                                            EControllerActions_FlyUp,
                                            EControllerActions_FlyDown,
                                            EControllerActions_SpectatePrevious,
                                            EControllerActions_SpectateNext,
                                            EControllerActions_SpectateNoise,
                                            EControllerActions_SpectateCancel,
                                            EControllerActions_ConfirmReady,
                                            EControllerActions_Vote,
                                            EControllerActions_RestartSection,
                                            EControllerActions_RestartRace,
                                            EControllerActions_LookBehind};

    switch (control)
    {
    case EControl_Look:
        return GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) ? EControllerActions_MoveRight
                                                                                : EControllerActions_LookDown;
    case EControl_Move:
        return GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) ? EControllerActions_LookDown
                                                                                : EControllerActions_MoveRight;
    case EControl_Use:
    case EControl_Action:
    case EControl_Jump:
    case EControl_Drop:
    case EControl_Inventory:
    case EControl_Crafting:
    case EControl_CycleHeldItemLeft:
    case EControl_CycleHeldItemRight:
    case EControl_Sneak:
    case EControl_Sprint:
    case EControl_PickBlock:
    case EControl_ChangeCameraMode:
    case EControl_FlyLeft:
    case EControl_FlyRight:
    case EControl_FlyUp:
    case EControl_FlyDown:
    case EControl_SpectatePrevious:
    case EControl_SpectateNext:
    case EControl_SpectateNoise:
    case EControl_SpectateCancel:
    case EControl_ConfirmReady:
    case EControl_Vote:
    case EControl_RestartSection:
    case EControl_RestartRace:
    case EControl_LookBehind:
        return actions[control - EControl_Use];
    default:
        return '=';
    }
}

std::wstring UIScene_ControlsMenu::GetControlsString(EControl control, bool& mapped)
{
    std::wstring result = L"";
    if (GetControlsAction(control) < '=')
        result = GetControlsString(m_playerId, (EControllerActions)GetControlsAction(control), mapped);
    return result;
}

void UIScene_ControlsMenu::UpdateControls(int player)
{
    bool mapped = true;
    std::wstring controlString = L"";
    m_controlsList.SetCheckboxValue(EControl_InvertLook,
                                    GameSettings::GetGameSettings(m_playerId, eGameSetting_InvertLook) != 0, false);
    m_controlsList.SetCheckboxValue(EControl_SouthPaw,
                                    GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) != 0, false);
    controlString = GetControlsString(EControl_Look, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Look, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Move, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Move, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Use, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Use, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Action, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Action, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Jump, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Jump, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Drop, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Drop, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Inventory, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Inventory, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Crafting, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Crafting, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_CycleHeldItemLeft, mapped);
    m_controlsList.SetSecondaryLabel(EControl_CycleHeldItemLeft, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_CycleHeldItemRight, mapped);
    m_controlsList.SetSecondaryLabel(EControl_CycleHeldItemRight, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Sneak, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Sneak, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Sprint, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Sprint, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_PickBlock, mapped);
    m_controlsList.SetSecondaryLabel(EControl_PickBlock, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_ChangeCameraMode, mapped);
    m_controlsList.SetSecondaryLabel(EControl_ChangeCameraMode, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_FlyLeft, mapped);
    m_controlsList.SetSecondaryLabel(EControl_FlyLeft, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_FlyRight, mapped);
    m_controlsList.SetSecondaryLabel(EControl_FlyRight, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_FlyUp, mapped);
    m_controlsList.SetSecondaryLabel(EControl_FlyUp, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_FlyDown, mapped);
    m_controlsList.SetSecondaryLabel(EControl_FlyDown, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_SpectatePrevious, mapped);
    m_controlsList.SetSecondaryLabel(EControl_SpectatePrevious, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_SpectateNext, mapped);
    m_controlsList.SetSecondaryLabel(EControl_SpectateNext, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_SpectateNoise, mapped);
    m_controlsList.SetSecondaryLabel(EControl_SpectateNoise, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_SpectateCancel, mapped);
    m_controlsList.SetSecondaryLabel(EControl_SpectateCancel, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_ConfirmReady, mapped);
    m_controlsList.SetSecondaryLabel(EControl_ConfirmReady, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_Vote, mapped);
    m_controlsList.SetSecondaryLabel(EControl_Vote, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_RestartSection, mapped);
    m_controlsList.SetSecondaryLabel(EControl_RestartSection, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_RestartRace, mapped);
    m_controlsList.SetSecondaryLabel(EControl_RestartRace, UIString(controlString), true, mapped, false);
    controlString = GetControlsString(EControl_LookBehind, mapped);
    m_controlsList.SetSecondaryLabel(EControl_LookBehind, UIString(controlString), true, mapped, false);
    m_updateControls = false;
}

int UIScene_ControlsMenu::ResetControlsWarningDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL)
    {
        if (result == C4JStorage::EMessageResult_Accept)
        {
            GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId),
                                             scene->m_playerId, true);
            GameSettings::CheckGameSettingsChanged(true, pad);
            scene->UpdateControls(scene->m_playerId);
        }
        else if (result == C4JStorage::EMessageResult_Decline || result == C4JStorage::EMessageResult_Cancelled)
        {
            scene->navigateBack();
        }
    }
    return 0;
}

void UIScene_ControlsMenu::CheckMapping(int player)
{
    if (g_pInput->IsPadClassicController(player) == 0)
        return;
    for (int control = EControl_Use; control <= EControl_LookBehind; ++control)
    {
        int mapping = GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(control))
                                                                 - EControllerActions_Jump);
        int secondButton = mapping & 0xf;
        int firstButton = (mapping >> 4) & 0xff;
        if (secondButton == 0xd || secondButton == 0xe || firstButton == 0xd || firstButton == 0xe)
        {
            int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
            gConsoleUIController.RequestAlertMessage(
                IDS_CONTROLS_CONTROLLER_WARNING_TITLE, IDS_CONTROLS_CONTROLLER_WARNING_TEXT, parameters, 2, m_playerId,
                ResetControlsWarningDialogReturned, (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL,
                true);
            return;
        }
    }
}

void UIScene_ControlsMenu::tick()
{
#ifdef MINECRAFT_PC
    if (m_keyboard && m_editingButton)
    {
        const int code = PcInputTakeKeyCapture();
        if (code != -2)
            FinishKeyCapture(code);
    }
    else if (!m_editingButton && KeyboardMode() != m_keyboard)
    {
        if (m_mappingsChanged)
        {
            GameSettings::CheckGameSettingsChanged(true, m_playerId);
            GameSettings::ActionGameSettings(m_playerId, eGameSetting_ControlMappings);
        }
        m_controlsList.clearList();
        Init(m_playerId);
        updateTooltips();
    }
#endif
    if (m_checkMapping)
    {
        CheckMapping(m_playerId);
        m_checkMapping = false;
    }
    if (m_updateControls)
        UpdateControls(m_playerId);
    if (m_showAssignHelp)
    {
        SetControlsHelp(m_playerId, m_controlsList.getCurrentSelection(),
                        L10N::GetString(IDS_CONTROLS_BUTTON_ASSIGN_HELP));
        m_showAssignHelp = false;
    }
    if (m_clearHelp)
    {
        SetControlsHelp(m_playerId, 0, L"");
        m_clearHelp = false;
    }
    UIScene::tick();
}

std::wstring UIScene_ControlsMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"ControlsSplit";
    return L"Controls";
}

void UIScene_ControlsMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second == 1)
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InvertLook, checked);
    else if (second == 2)
    {
        if ((GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) != 0) != checked)
            m_updateControls = true;
        GameSettings::SetGameSettings(m_playerId, eGameSetting_SouthPaw, checked);
    }
}

int UIScene_ControlsMenu::ResetDefaultsDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL && result == C4JStorage::EMessageResult_Decline)
    {
#ifdef MINECRAFT_PC
        if (scene->m_keyboard)
        {
            PcKeyBindingsReset();
            scene->UpdateKeyboard();
            scene->updateTooltips();
            return 0;
        }
#endif
        GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId), scene->m_playerId,
                                         true);
        GameSettings::CheckGameSettingsChanged(true, pad);
        scene->UpdateControls(scene->m_playerId);
    }
    return 0;
}

int UIScene_ControlsMenu::ControlsWarningDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL && result == C4JStorage::EMessageResult_Accept)
    {
        GameSettings::CheckGameSettingsChanged(true, pad);
        GameSettings::ActionGameSettings(pad, eGameSetting_ControlMappings);
        scene->m_mappingsChanged = false;
        scene->navigateBack();
    }
    return 0;
}

void UIScene_ControlsMenu::handleReload()
{
    Init(m_playerId);
}

unsigned int UIScene_ControlsMenu::CheckControls(int player, EControl control)
{
    bool found = false;
    int conflicting = 0;
    int mapping = GameSettings::GetActionMapping(player, GetControlsAction(control) - EControllerActions_Jump);

    if (control >= EControl_Use && control <= EControl_FlyDown)
    {
        for (int candidate = EControl_Use; candidate <= EControl_FlyDown; ++candidate)
        {
            if (candidate != control
                && mapping
                       == GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(candidate))
                                                                     - EControllerActions_Jump))
            {
                found = true;
                conflicting = candidate;
                GameSettings::SetActionMapping(
                    player, GetControlsAction(static_cast<EControl>(conflicting)) - EControllerActions_Jump, 0);
                break;
            }
        }
    }
    else if (control >= EControl_ConfirmReady && control <= EControl_Vote)
    {
        for (int candidate = EControl_ConfirmReady; candidate <= EControl_Vote; ++candidate)
        {
            if (candidate != control
                && mapping
                       == GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(candidate))
                                                                     - EControllerActions_Jump))
            {
                found = true;
                conflicting = candidate;
                GameSettings::SetActionMapping(
                    player, GetControlsAction(static_cast<EControl>(conflicting)) - EControllerActions_Jump, 0);
                break;
            }
        }
    }
    else if (control >= EControl_SpectatePrevious && control <= EControl_SpectateCancel)
    {
        for (int candidate = EControl_SpectatePrevious; candidate <= EControl_SpectateCancel; ++candidate)
        {
            if (candidate != control
                && mapping
                       == GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(candidate))
                                                                     - EControllerActions_Jump))
            {
                found = true;
                conflicting = candidate;
                GameSettings::SetActionMapping(
                    player, GetControlsAction(static_cast<EControl>(conflicting)) - EControllerActions_Jump, 0);
                break;
            }
        }
    }
    else if (control >= EControl_RestartSection && control <= EControl_LookBehind)
    {
        for (int candidate = EControl_RestartSection; candidate <= EControl_LookBehind; ++candidate)
        {
            if (candidate != control
                && mapping
                       == GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(candidate))
                                                                     - EControllerActions_Jump))
            {
                found = true;
                conflicting = candidate;
                GameSettings::SetActionMapping(
                    player, GetControlsAction(static_cast<EControl>(conflicting)) - EControllerActions_Jump, 0);
                break;
            }
        }
    }

    if (found)
    {
        std::wstring text = L" ";
        text += L10N::GetString(IDS_CONTROLS_NONE);
        m_controlsList.SetSecondaryLabel(conflicting, UIString(text), true, false, false);
        m_controlsList.HighlightItem(conflicting, false);
        SetControlsHelp(player, conflicting, L10N::GetString(IDS_CONTROLS_BUTTON_ASSIGN_CONFLICT));
    }
    return found;
}

void UIScene_ControlsMenu::HandleButtonEdit(int player, int action, bool repeat, bool pressed, bool released)
{
    if (action == EControllerActions_MenuBack)
    {
        if (!repeat && pressed && !released)
        {
            bool mapped = true;
            std::wstring controlString = GetControlsString((EControl)m_controlsList.getCurrentSelection(), mapped);
            m_controlsList.SetSecondaryLabel(m_controlsList.getCurrentSelection(), UIString(controlString), true,
                                             mapped, false);
            m_editingButton = false;
            updateTooltips();
            m_firstButtonMask = 0;
            m_buttonCount = 0;
            m_secondButton = 0;
            m_firstButton = 0;
            m_lastEditAction = -1;
            SetControlsHelp(player, 0, L"");
        }
        return;
    }

    if (m_buttonCount != 0 && !repeat && !pressed && released)
    {
        for (int button = EControllerActions_MenuA; button <= EControllerActions_MenuOtherStickPress; ++button)
        {
            if (g_pInput->GetValue(player, static_cast<unsigned char>(button), true)
                || g_pInput->ButtonDown(player, static_cast<unsigned char>(button)))
                return;
        }

        unsigned char mapping = (m_buttonCount == 1)
                                    ? m_firstButton
                                    : static_cast<unsigned char>(m_secondButton | (m_firstButton << 4));
        GameSettings::SetActionMapping(
            player, GetControlsAction((EControl)m_controlsList.getCurrentSelection()) - EControllerActions_Jump,
            mapping);
        GameSettings::SetSettingsChanged(player);
        m_mappingsChanged = true;

        bool mapped = true;
        std::wstring controlString = GetControlsString((EControl)m_controlsList.getCurrentSelection(), mapped);
        m_controlsList.SetSecondaryLabel(m_controlsList.getCurrentSelection(), UIString(controlString), true, mapped,
                                         false);
        m_editingButton = false;
        m_unusedEditFlag = false;
        updateTooltips();
        m_firstButton = 0;
        m_firstButtonMask = 0;
        m_lastEditAction = -1;
        m_secondButton = 0;
        m_buttonCount = 0;

        if (CheckControls(player, (EControl)m_controlsList.getCurrentSelection()) == 0)
            SetControlsHelp(player, 0, L"");
        return;
    }

    if (action < EControllerActions_MenuA || action > EControllerActions_MenuOtherStickPress || !pressed
        || m_lastEditAction == action)
        return;

    unsigned int bit;
    unsigned char id;
    switch (action)
    {
    case EControllerActions_MenuA:
        if (g_pInput->IsCircleCrossSwapped())
        {
            bit = 2;
            id = 2;
        }
        else
        {
            bit = 1;
            id = 1;
        }
        break;
    case EControllerActions_MenuB:
        if (g_pInput->IsCircleCrossSwapped())
        {
            bit = 1;
            id = 1;
        }
        else
        {
            bit = 2;
            id = 2;
        }
        break;
    case EControllerActions_MenuX:
        bit = 4;
        id = 3;
        break;
    case EControllerActions_MenuY:
        bit = 8;
        id = 4;
        break;
    case EControllerActions_MenuUp:
        if (g_pInput->GetJoypadStick_Menu_LY(player) != 0.0f)
            return;
        bit = 0x400;
        id = 7;
        break;
    case EControllerActions_MenuDown:
        if (g_pInput->GetJoypadStick_Menu_LY(player) != 0.0f)
            return;
        bit = 0x800;
        id = 8;
        break;
    case EControllerActions_MenuRight:
        if (g_pInput->GetJoypadStick_Menu_LX(player) != 0.0f)
            return;
        bit = 0x2000;
        id = 6;
        break;
    case EControllerActions_MenuLeft:
        if (g_pInput->GetJoypadStick_Menu_LX(player) != 0.0f)
            return;
        bit = 0x1000;
        id = 5;
        break;
    case EControllerActions_MenuPageUp:
        bit = 0x800000;
        id = 0xc;
        break;
    case EControllerActions_MenuPageDown:
        bit = 0x400000;
        id = 0xb;
        break;
    case EControllerActions_MenuRightScroll:
        bit = 0x40;
        id = 9;
        break;
    case EControllerActions_MenuLeftScroll:
        bit = 0x80;
        id = 0xa;
        break;
    case EControllerActions_MenuStickPress:
        bit = 0x200;
        id = 0xe;
        break;
    case EControllerActions_MenuOtherStickPress:
        bit = 0x100;
        id = 0xd;
        break;
    default:
        return;
    }

    if (m_buttonCount == 0)
    {
        m_firstButtonMask = bit;
        m_firstButton = id;
        std::wstring text = L"..." + gApp.GetSingleActionReplacement(player, bit);
        m_controlsList.SetSecondaryLabel(m_controlsList.getCurrentSelection(), UIString(text), true, true, false);
        m_buttonCount++;
    }
}

unsigned int UIScene_ControlsMenu::CheckControlsOnExit(int player)
{
    for (int control = EControl_Use; control <= EControl_LookBehind; ++control)
    {
        if (control == EControl_Sprint || control == EControl_PickBlock
            || (control >= EControl_FlyLeft && control <= EControl_FlyDown))
            continue;
        if (GameSettings::GetActionMapping(player, GetControlsAction((EControl)control) - EControllerActions_Jump) == 0)
        {
            m_controlsList.SetSecondaryLabel(control, UIString(L10N::GetString(IDS_CONTROLS_NONE)), true, false, false);
            m_controlsList.HighlightItem(control, false);
            SetControlsHelp(player, control, L10N::GetString(IDS_CONTROLS_BUTTON_ASSIGN_CONFLICT));
            int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
            gConsoleUIController.RequestAlertMessage(
                IDS_CONTROLS_WARNING_TITLE, IDS_CONTROLS_WARNING_TEXT, parameters, 2, m_playerId,
                ControlsWarningDialogReturned, (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
            return 1;
        }
    }
    return 0;
}

void UIScene_ControlsMenu::handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled)
{
#ifdef MINECRAFT_PC
    if (m_keyboard)
    {
        HandleKeyboardInput(player, action, repeat, pressed, released);
        return;
    }
#endif
    if (m_editingButton)
    {
        HandleButtonEdit(player, action, repeat, pressed, released);
        return;
    }
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !CheckControlsOnExit(player))
        {
            GameSettings::CheckGameSettingsChanged(true, player);
            GameSettings::ActionGameSettings(player, eGameSetting_ControlMappings);
            m_mappingsChanged = false;
            navigateBack();
        }
        break;
    case EControllerActions_MenuOk:
        if (pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        if (!repeat)
            sendInputToMovie(action, false, pressed, released);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        m_clearHelp = true;
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_ControlsMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == 0)
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestAlertMessage(IDS_DEFAULTS_CONTROLS_TITLE, IDS_DEFAULTS_CONTROLS_TEXT, parameters, 2,
                                                 m_playerId, ResetDefaultsDialogReturned,
                                                 (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
        return;
    }
#ifdef MINECRAFT_PC
    if (m_keyboard)
    {
        if (IsKeyRow(childId) && !m_editingButton)
        {
            m_controlsList.SetSecondaryLabel(childId, UIString(std::wstring(L"...")), true, false, false);
            m_lastEditAction = childId - KEY_ROW_FIRST;
            m_editingButton = true;
            PcInputBeginKeyCapture();
            SetControlsHelp(m_playerId, childId, L"Press a key or mouse button to bind it. Esc cancels.");
            updateTooltips();
        }
        return;
    }
#endif
    if (childId >= EControl_Use && childId <= EControl_LookBehind)
    {
        std::wstring ellipsis(L"...");
        m_controlsList.SetSecondaryLabel(childId, UIString(ellipsis), true, false, false);
        m_editingButton = true;
        m_showAssignHelp = true;
        updateTooltips();
    }
}

void UIScene_ControlsMenu::handleFocusChange(int focus, int childFocus)
{
    m_controlsList.updateChildFocus(childFocus);
    updateTooltips();
}

int UIScene_ControlsMenu::getSceneType()
{
    return EUIScene_ControlsMenu;
}

#ifdef MINECRAFT_PC
bool UIScene_ControlsMenu::KeyboardMode()
{
    return PcInputGlyphSet() == ePcGlyphSet_Keyboard && m_playerId == gProfile->GetPrimaryPad();
}

void UIScene_ControlsMenu::InitKeyboard(int player)
{
    m_controlsList.AddNewButton(L10N::GetString(IDS_RESET_TO_DEFAULTS), EControl_ResetToDefaults, true);
    int section = -1;
    for (int action = 0; action < PcKeyBindingCount(); ++action)
    {
        const PcKeyAction key = static_cast<PcKeyAction>(action);
        if (PcKeyBindingSection(key) != section)
        {
            section = PcKeyBindingSection(key);
            m_controlsList.AddNewLabel(L" ", EControl_Label);
            m_controlsList.AddNewLabel(PcKeySectionLabel(static_cast<PcKeySection>(section)), EControl_Label);
            if (section == ePcKeySection_Movement)
            {
                m_controlsList.AddNewCheckbox(L10N::GetString(IDS_INVERT_LOOK), EControl_InvertLook,
                                              GameSettings::GetGameSettings(m_playerId, eGameSetting_InvertLook) != 0);
                std::wstring look;
                PcKeyActionMarkup(EControllerActions_LookDown, KeyRowSize(), &look);
                m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOOK), look, true, EControl_Look);
            }
        }
        const int code = PcKeyBinding(key);
        m_controlsList.AddNewControllerButton(PcKeyBindingLabel(key), KeyRowText(code), code != kPcKeyNone,
                                              KEY_ROW_FIRST + action);
    }
    m_controlsList.EnableItem(EControl_Look, false, true);

    m_buttonCount = 0;
    m_firstButton = 0;
    m_editingButton = false;
    m_unusedEditFlag = false;
    m_updateControls = false;
    m_showAssignHelp = false;
    m_secondButton = 0;
    m_firstButtonMask = 0;
    m_clearHelp = false;
    m_mappingsChanged = false;
    m_lastEditAction = -1;
    m_checkMapping = false;

    SetControlsHelp(player, 0, L"");
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_ControlsMenu::UpdateKeyboard()
{
    m_controlsList.SetCheckboxValue(EControl_InvertLook,
                                    GameSettings::GetGameSettings(m_playerId, eGameSetting_InvertLook) != 0, false);
    for (int action = 0; action < PcKeyBindingCount(); ++action)
    {
        const int code = PcKeyBinding(static_cast<PcKeyAction>(action));
        m_controlsList.SetSecondaryLabel(KEY_ROW_FIRST + action, UIString(KeyRowText(code)), true,
                                         code != kPcKeyNone, false);
    }
}

void UIScene_ControlsMenu::FinishKeyCapture(int code)
{
    m_editingButton = false;
    const PcKeyAction action = static_cast<PcKeyAction>(m_lastEditAction);
    std::wstring help;
    if (code >= 0)
    {
        PcKeyBindingSet(action, code);
        help = SharedBindingHelp(action, code);
    }
    UpdateKeyboard();
    SetControlsHelp(m_playerId, KEY_ROW_FIRST + m_lastEditAction, help);
    updateTooltips();
}

void UIScene_ControlsMenu::HandleKeyboardInput(int player, int action, bool repeat, bool pressed, bool released)
{
    if (m_editingButton)
    {
        if (action == EControllerActions_MenuCancel && pressed && !repeat)
        {
            PcInputCancelKeyCapture();
            FinishKeyCapture(-1);
        }
        return;
    }
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
            navigateBack();
        break;
    case EControllerActions_MenuX:
        if (pressed && !repeat)
        {
            const int selection = m_controlsList.getCurrentSelection();
            if (IsKeyRow(selection)
                && PcKeyBinding(static_cast<PcKeyAction>(selection - KEY_ROW_FIRST)) != kPcKeyNone)
            {
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                PcKeyBindingSet(static_cast<PcKeyAction>(selection - KEY_ROW_FIRST), kPcKeyNone);
                UpdateKeyboard();
                SetControlsHelp(m_playerId, selection, L"");
                updateTooltips();
            }
        }
        break;
    case EControllerActions_MenuOk:
        if (pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        if (!repeat)
            sendInputToMovie(action, false, pressed, released);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        m_clearHelp = true;
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}
#endif
