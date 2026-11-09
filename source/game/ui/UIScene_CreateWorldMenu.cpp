#include "game/ui/UIScene_CreateWorldMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_CreateWorldMenu::UIScene_CreateWorldMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_StartGame(sceneType, data, layer, false),
      m_worldNameLabel(),
      m_moreOptionsButton(),
      m_playButton(),
      m_worldNameInput(),
      m_worldIcon()
{
    initialiseMovie();
    m_worldName.assign(L10N::GetString(IDS_DEFAULT_WORLD_NAME));
    m_playerId = sceneType;
    m_worldNameLabel.init(UIString(L10N::GetString(IDS_WORLD_NAME)));
    m_worldNameInput.init(UIString(m_worldName), 0);
    m_worldNameInput.SetCharLimit(0x19);
    m_gameModeSlider.init(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), 1, 0, GameType::CREATIVE->getId(),
                          GameType::SURVIVAL->getId(), false, 10, 1);
    m_moreOptionsButton.init(UIString(L10N::GetString(IDS_MORE_OPTIONS)), 3);
    m_playButton.init(UIString(L10N::GetString(IDS_CREATE_NEW_WORLD)), 6);
    m_texturePackList.init(std::wstring(L10N::GetString(IDS_DLC_MENU_TEXTUREPACKS)), 4);
    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    m_difficulty = GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty);
    wchar_t difficultyLabel[256];
    std::swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                  L10N::GetString(
                      Difficulty::byId(GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty))->getKey()));
    m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3,
                            GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty), false, 10, 1);
    m_launchData.mNewWorld = true;
    bool initialChecked = m_launchData.mOnlineGame;
    m_onlineGameCheckBox.SetEnable(true);
    if (!gProfile->IsSignedInLive(m_playerId))
        m_onlineGameCheckBox.SetEnable(false);
    if (m_launchData.mOnlineSettingChangedBySystem)
    {
        m_onlineGameCheckBox.SetEnable(false);
        initialChecked = false;
    }
    m_onlineGameCheckBox.init(UIString(L10N::GetString(IDS_ONLINE_GAME)), 5, initialChecked);
    _init();
    if (data != NULL)
        ::operator delete(data);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_CreateWorldMenu::~UIScene_CreateWorldMenu()
{
}

unsigned int UIScene_CreateWorldMenu::mapElementsAndNames()
{
    IUIScene_StartGame::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_worldNameLabel.setupControl(this, currentNode, std::string("WorldName"));
            m_controls.push_back(&m_worldNameLabel);
        }
        {
            m_worldNameInput.setupControl(this, currentNode, std::string("EditWorldName"));
            m_controls.push_back(&m_worldNameInput);
        }
        {
            m_texturePackList.setupControl(this, currentNode, std::string("TexturePackSelector"));
            m_controls.push_back(&m_texturePackList);
        }
        {
            m_gameModeSlider.setupControl(this, currentNode, std::string("GameModeToggle"));
            m_controls.push_back(&m_gameModeSlider);
        }
        {
            m_onlineGameCheckBox.setupControl(this, currentNode, std::string("CheckboxOnline"));
            m_controls.push_back(&m_onlineGameCheckBox);
        }
        {
            m_moreOptionsButton.setupControl(this, currentNode, std::string("MoreOptions"));
            m_controls.push_back(&m_moreOptionsButton);
        }
        {
            m_playButton.setupControl(this, currentNode, std::string("NewWorld"));
            m_controls.push_back(&m_playButton);
        }
        {
            m_difficultySlider.setupControl(this, currentNode, std::string("Difficulty"));
            m_controls.push_back(&m_difficultySlider);
        }
        currentNode = parentNode;
    }
    return 1;
}

void UIScene_CreateWorldMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, false, false);
}

void UIScene_CreateWorldMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_CreateWorldMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

void UIScene_CreateWorldMenu::tick()
{
    IUIScene_StartGame::tick();
}

UIControl* UIScene_CreateWorldMenu::GetMainPanel()
{
    return &m_mainPanel;
}

std::wstring UIScene_CreateWorldMenu::getMoviePath()
{
    return std::wstring(L"CreateWorldMenu");
}

void UIScene_CreateWorldMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        m_onlineGameCheckBox.setChecked(m_launchData.mOnlineGame);
}

void UIScene_CreateWorldMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                          bool& handled)
{
    if (m_actionPending)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        {
            bool checked = m_onlineGameCheckBox.IsChecked();
            if (m_launchData.mOnlineGame != checked)
            {
                m_launchData.mOnlineGame = checked;
                if (!checked)
                {
                    m_launchData.mInviteOnly = false;
                    m_launchData.mAllowFriendsOfFriends = false;
                }
            }
        }
        handled = true;
        break;
    }
}

int UIScene_CreateWorldMenu::KeyboardCompleteWorldNameCallback(void* data, bool accepted)
{
    UIScene_CreateWorldMenu* scene = static_cast<UIScene_CreateWorldMenu*>(data);
    scene->m_actionPending = false;
    if (accepted)
    {
        wchar_t text[128];
        std::memset(text, 0, sizeof(text));
        g_pInput->GetText(text, 127);
        if (text[0] != L'\0')
        {
            scene->m_worldNameInput.setLabel(UIString(text), false, false);
            scene->m_worldName = text;
        }
        scene->m_playButton.setEnable(scene->m_worldName.size() != 0, true);
    }
    return 0;
}

void UIScene_CreateWorldMenu::handlePress(int controlId, int childId)
{
    if (!m_actionPending)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        switch (controlId)
        {
        case 0:
            m_actionPending = true;
            g_pInput->RequestKeyboard(L10N::GetString(IDS_CREATE_NEW_WORLD), m_worldNameInput.getLabel(), 0, 0x19,
                                      KeyboardCompleteWorldNameCallback, this, C_4JInput::EKeyboardMode_0);
            break;
        case 3:
            gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LaunchMoreOptionsMenu, &m_launchData,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        case 4:
            UpdateCurrentTexturePack(childId);
            break;
        case 6:
            StartSharedLaunchFlow();
            break;
        }
    }
}

void UIScene_CreateWorldMenu::handleCheckboxToggled(int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_CreateWorldMenu::checkStateAndStartGame()
{
    LaunchGame();
}

void UIScene_CreateWorldMenu::SetEnablePlayGameButton(bool enable)
{
    m_playButton.setEnable(enable, true);
}

int UIScene_CreateWorldMenu::getSceneType()
{
    return EUIScene_CreateWorldMenu;
}
