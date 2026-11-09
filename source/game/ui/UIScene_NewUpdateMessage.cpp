#include "game/ui/UIScene_NewUpdateMessage.hpp"
#include "game/core/StringIds.hpp"

#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"

UIScene_NewUpdateMessage::UIScene_NewUpdateMessage(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    layer->addComponent(sceneType, EUIScene_Panorama, NULL);
    layer->addComponent(sceneType, EUIScene_Logo, NULL);
    m_acceptButton.init(UIString(L10N::GetString(static_cast<int>(IDS_TOOLTIPS_ACCEPT))), 0);

    std::wstring text(L10N::GetString(static_cast<int>(0x31bf39b5)));
    text += L"\r\n";
    text = gApp.FormatHTMLString(m_playerId, text, -1);

    std::vector<std::wstring> lines;
    std::wstring::size_type start = 0;
    std::wstring::size_type separator = text.find(L"\r\n", start, 2);
    while (separator != std::wstring::npos)
    {
        lines.push_back(text.substr(start, separator - start) + L" ");
        start = separator + 2;
        separator = text.find(L"\r\n", start, 2);
    }
    lines.push_back(text.substr(start, text.size() - start));
    for (unsigned int i = 0; i < lines.size(); ++i)
        m_eulaText.addText(lines[i], i == lines.size() - 1);

    m_ignoreInput = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_NewUpdateMessage::~UIScene_NewUpdateMessage()
{
    m_layer->removeComponent(EUIScene_Panorama);
    m_layer->removeComponent(EUIScene_Logo);
}

unsigned int UIScene_NewUpdateMessage::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_acceptButton.setupControl(this, root, std::string("AcceptButton"));
        m_controls.push_back(&m_acceptButton);
    }
    {
        m_eulaText.setupControl(this, root, std::string("EULAtext"));
        m_controls.push_back(&m_eulaText);
    }
    return 1;
}

void UIScene_NewUpdateMessage::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

std::wstring UIScene_NewUpdateMessage::getMoviePath()
{
    return L"EULA";
}

void UIScene_NewUpdateMessage::handleInput(int, int action, bool repeat, bool pressed, bool released, bool&)
{
    if (m_ignoreInput)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuB:
    {
        int value = GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayUpdateMessage);
        if (value > 0)
            --value;
        GameSettings::SetGameSettings(m_playerId, eGameSetting_DisplayUpdateMessage, static_cast<unsigned char>(value));
        GameSettings::SetGameSettings(m_playerId, eGameSetting_UpdateMessageSeen, 1);
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
        break;
    }
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_NewUpdateMessage::handlePress(int controlId, int)
{
    if (controlId == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        int value = GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayUpdateMessage);
        if (value > 0)
            --value;
        GameSettings::SetGameSettings(m_playerId, eGameSetting_DisplayUpdateMessage, static_cast<unsigned char>(value));
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
    }
}

int UIScene_NewUpdateMessage::getSceneType()
{
    return EUIScene_EULA;
}

bool UIScene_NewUpdateMessage::hasFocus(int)
{
    return m_hasFocus;
}

unsigned long long UIScene_NewUpdateMessage::getDefaultGtcButtons()
{
    return 0;
}
