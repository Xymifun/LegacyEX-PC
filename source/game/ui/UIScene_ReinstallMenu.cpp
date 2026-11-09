#include "game/ui/UIScene_ReinstallMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"

UIScene_ReinstallMenu::UIScene_ReinstallMenu(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
}

unsigned int UIScene_ReinstallMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_buttons[0].setupControl(this, currentNode, std::string("Button1"));
        m_controls.push_back(&m_buttons[0]);
    }
    {
        m_buttons[1].setupControl(this, currentNode, std::string("Button2"));
        m_controls.push_back(&m_buttons[1]);
    }
    {
        m_buttons[2].setupControl(this, currentNode, std::string("Button3"));
        m_controls.push_back(&m_buttons[2]);
    }
    {
        m_buttons[3].setupControl(this, currentNode, std::string("Button4"));
        m_controls.push_back(&m_buttons[3]);
    }
    {
        m_buttons[4].setupControl(this, currentNode, std::string("Button5"));
        m_controls.push_back(&m_buttons[4]);
    }
    {
        m_buttons[5].setupControl(this, currentNode, std::string("Button6"));
        m_controls.push_back(&m_buttons[5]);
    }
    return 1;
}

void UIScene_ReinstallMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_SELECTDEVICE,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_ReinstallMenu::updateComponents()
{
    bool noLevel = Minecraft::GetInstance()->mLevel == NULL;
    if (noLevel)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, true);
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        m_layer->showComponent(m_playerId, EUIScene_Logo, false);
    }
}

std::wstring UIScene_ReinstallMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"ReinstallSplit");
    return std::wstring(L"ReinstallMenu");
}

void UIScene_ReinstallMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
            navigateBack();
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_ReinstallMenu::handlePress(int, int)
{
}

int UIScene_ReinstallMenu::getSceneType()
{
    return EUIScene_ReinstallMenu;
}
