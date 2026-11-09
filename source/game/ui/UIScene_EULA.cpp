#include "game/ui/UIScene_EULA.hpp"
#include "game/core/StringIds.hpp"

#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/UIString.hpp"

UIScene_EULA::UIScene_EULA(int sceneType, void* callback, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    layer->addComponent(sceneType, EUIScene_Panorama, 0);
    layer->addComponent(sceneType, EUIScene_Logo, 0);
    m_acceptButton.init(UIString(L10N::GetString(IDS_TOOLTIPS_ACCEPT)), 0);

    std::wstring text(L10N::GetString(IDS_EULA));
    text.append(L"\r\n");
    std::vector<std::wstring> lines;
    std::wstring::size_type start = 0;
    std::wstring::size_type separator = text.find(L"\r\n", start, 2);
    while (separator != std::wstring::npos)
    {
        lines.push_back(text.substr(start, separator - start) + L" ");
        start = separator + 2;
        separator = text.find(L"\r\n", start, 2);
    }
    lines.push_back(text.substr(start, text.length() - start));

    unsigned int i;
    for (i = 0; i < lines.size(); ++i)
        m_eulaText.addText(lines[i], i == lines.size() - 1);

    m_ignoreInput = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_EULA::~UIScene_EULA()
{
    m_layer->removeComponent(EUIScene_Panorama);
    m_layer->removeComponent(EUIScene_Logo);
}

unsigned int UIScene_EULA::mapElementsAndNames()
{
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

void UIScene_EULA::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

std::wstring UIScene_EULA::getMoviePath()
{
    return std::wstring(L"EULA");
}

void UIScene_EULA::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_ignoreInput)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
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

void UIScene_EULA::handlePress(int controlId, int childId)
{
    if (controlId == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        GameSettings::SetGameSettings(0, eGameSetting_EULAAccepted, 1);
        gConsoleUIController.NavigateToScene(0, EUIScene_SaveMessage, 0, EUILayer_Scene, EUIGroup_PlayerDefault);
    }
}

int UIScene_EULA::getSceneType()
{
    return EUIScene_EULA;
}

bool UIScene_EULA::hasFocus(int controller)
{
    return m_hasFocus;
}

unsigned long long UIScene_EULA::getDefaultGtcButtons()
{
    return 0;
}
