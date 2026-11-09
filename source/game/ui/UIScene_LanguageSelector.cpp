#include "game/ui/UIScene_LanguageSelector.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

static const unsigned int languageNames[] = {0x4A55D76A, 0x85CABB8D, 0xA7BED7AD, 0x927530AB, 0xB524803B,
                                             0x1C9D5CE9, 0xAF947AA3, 0xFD7B9B78, 0x8DC52CD3, 0xBEACA054,
                                             0x4F5B792B, 0xD1E5AF59, 0x21828946};

UIScene_LanguageSelector::UIScene_LanguageSelector(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_languageList.init(0);
    for (int i = 0; i < 13; ++i)
        m_languageList.addItem(UIString(languageNames[i]), i);
}

void UIScene_LanguageSelector::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_LanguageSelector::updateComponents()
{
    bool showSplitScreen = false;
    if (Minecraft::GetInstance()->mLevel == NULL)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showSplitScreen = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        if (gApp.GetLocalPlayerCount() == 1)
            showSplitScreen = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSplitScreen);
}

void UIScene_LanguageSelector::handleReload()
{
    for (int i = 0; i < 13; ++i)
        m_languageList.addItem(UIString(languageNames[i]), i);
}

std::wstring UIScene_LanguageSelector::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"LanguagesMenuSplit");
    return std::wstring(L"LanguagesMenu");
}

void UIScene_LanguageSelector::handleInput(int, int action, bool repeat, bool pressed, bool released, bool&)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
            navigateBack();
        break;
    }
}

void UIScene_LanguageSelector::handlePress(int controlId, int childId)
{
    static const unsigned int languages[] = {0, 1, 3, 5, 4, 6, 9, 2, 7, 8, 17, 16, 12};
    static const unsigned int locales[] = {0, 0, 0, 31, 0, 0, 27, 0, 0, 0, 0, 0, 0};

    if (controlId == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        unsigned int locale = locales[childId];
        GameSettings::SetMinecraftLanguage(m_playerId, static_cast<unsigned char>(languages[childId]));
        GameSettings::SetMinecraftLocale(m_playerId, static_cast<unsigned char>(locale));
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
}

unsigned int UIScene_LanguageSelector::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_languageList.setupControl(this, currentNode, std::string("HowToList"));
        m_controls.push_back(&m_languageList);
    }
    return 1;
}

int UIScene_LanguageSelector::getSceneType()
{
    return EUIScene_LanguageSelector;
}
