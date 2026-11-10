#include "game/ui/UILayer.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModHooks.hpp"
#endif

#include <algorithm>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/ui/UIComponent_Chat.hpp"
#include "game/ui/UIComponent_DebugUIConsole.hpp"
#include "game/ui/UIComponent_DebugUIMarketingGuide.hpp"
#include "game/ui/UIComponent_Logo.hpp"
#include "game/ui/UIComponent_MenuBackground.hpp"
#include "game/ui/UIComponent_Panorama.hpp"
#include "game/ui/UIComponent_PressStartToPlay.hpp"
#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_AchievementsMenu.hpp"
#include "game/ui/UIScene_AnvilMenu.hpp"
#include "game/ui/UIScene_AvatarSelectMenu.hpp"
#include "game/ui/UIScene_BeaconMenu.hpp"
#include "game/ui/UIScene_BiomePreviewMenu.hpp"
#include "game/ui/UIScene_BrewingStandMenu.hpp"
#include "game/ui/UIScene_ClassicCraftingMenu.hpp"
#include "game/ui/UIScene_ConnectingProgress.hpp"
#include "game/ui/UIScene_ContainerMenu.hpp"
#include "game/ui/UIScene_ControlsMenu.hpp"
#include "game/ui/UIScene_CraftingMenu.hpp"
#include "game/ui/UIScene_CreateWorldMenu.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/ui/UIScene_Credits.hpp"
#include "game/ui/UIScene_DLCMainMenu.hpp"
#include "game/ui/UIScene_DLCOffersMenu.hpp"
#include "game/ui/UIScene_DeathMenu.hpp"
#include "game/ui/UIScene_DispenserMenu.hpp"
#include "game/ui/UIScene_EULA.hpp"
#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/ui/UIScene_EndPoem.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/ui/UIScene_FurnaceMenu.hpp"
#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"
#include "game/ui/UIScene_HUD.hpp"
#include "game/ui/UIScene_HelpAndOptionsMenu.hpp"
#include "game/ui/UIScene_HopperMenu.hpp"
#include "game/ui/UIScene_HorseInventoryMenu.hpp"
#include "game/ui/UIScene_HowToPlay.hpp"
#include "game/ui/UIScene_HowToPlayMenu.hpp"
#include "game/ui/UIScene_InGameHostOptionsMenu.hpp"
#include "game/ui/UIScene_InGameInfoMenu.hpp"
#include "game/ui/UIScene_InGamePlayerOptionsMenu.hpp"
#include "game/ui/UIScene_InGameSaveManagementMenu.hpp"
#include "game/ui/UIScene_InGameSettingsListMenu.hpp"
#include "game/ui/UIScene_Intro.hpp"
#include "game/ui/UIScene_InventoryMenu.hpp"
#include "game/ui/UIScene_JoinMenu.hpp"
#include "game/ui/UIScene_Keyboard.hpp"
#include "game/ui/UIScene_LanguageSelector.hpp"
#include "game/ui/UIScene_LaunchMoreOptionsMenu.hpp"
#include "game/ui/UIScene_LeaderboardsMenu.hpp"
#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"
#include "game/ui/UIScene_LoadMenu.hpp"
#include "game/ui/UIScene_MainMenu.hpp"
#include "game/ui/UIScene_MapSelectMenu.hpp"
#include "game/ui/UIScene_MapVoteMenu.hpp"
#include "game/ui/UIScene_MessageBox.hpp"
#include "game/ui/UIScene_NewUpdateMessage.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/ui/UIScene_ReinstallMenu.hpp"
#include "game/ui/UIScene_SaveMessage.hpp"
#include "game/ui/UIScene_SettingsListMenu.hpp"
#include "game/ui/UIScene_SettingsMenu.hpp"
#include "game/ui/UIScene_SignEntryMenu.hpp"
#include "game/ui/UIScene_DebugSetCamera.hpp"
#include "game/ui/UIScene_DebugFillArea.hpp"
#include "game/ui/UIScene_DebugCreateSchematic.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGlideSections.hpp"
#endif
#include "game/ui/UIScene_SkinSelectMenu.hpp"
#include "game/ui/UIScene_SuperFlatEditLayer.hpp"
#include "game/ui/UIScene_SuperFlatMenu.hpp"
#include "game/ui/UIScene_SuperFlatPresets.hpp"
#include "game/ui/UIScene_TeleportMenu.hpp"
#include "game/ui/UIScene_Timer.hpp"
#include "game/ui/UIScene_TradingMenu.hpp"
#include "game/ui/UIScene_TrialExitUpsell.hpp"
#include "game/ui/UIScene_WritableBookMenu.hpp"
#include "platform/wiiu/UIComponent_WiiUDRC.hpp"

UILayer::UILayer(UIGroup* group)
    : m_hasFocus(false),
      m_menuDisplayed(false),
      m_pauseMenuDisplayed(false),
      m_containerMenuDisplayed(false),
      m_ignoreAutosaveMenuDisplayed(false),
      m_ignorePlayerJoinMenuDisplayed(false),
      m_group(group)
{
}

void UILayer::getRenderDimensions(int& width, int& height)
{
    m_group->getRenderDimensions(width, height);
}

UIScene* UILayer::addComponent(int player, EUIScene scene, void* data)
{
    boost::unordered_map<EUIScene, std::pair<int, bool> >::iterator existing = m_componentStates.find(scene);
    if (existing != m_componentStates.end())
    {
        ++existing->second.first;
        for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
            if ((*it)->getSceneType() == static_cast<int>(scene))
                return *it;
        return NULL;
    }

    UIScene* created = NULL;
    switch (static_cast<int>(scene))
    {
    case EUIScene_TutorialPopup:
        created = new UIComponent_TutorialPopup(player, data, this);
        break;
    case EUIScene_Chat:
        created = new UIComponent_Chat(player, data, this);
        break;
    case EUIScene_HUD:
        created = new UIScene_HUD(player, data, this);
        break;
    case EUIScene_Panorama:
        created = new UIComponent_Panorama(player, data, this);
        break;
    case EUIScene_Logo:
        created = new UIComponent_Logo(player, data, this);
        break;
    case EUIScene_DebugUIConsole:
        created = new UIComponent_DebugUIConsole(player, data, this);
        break;
    case EUIScene_DebugUIMarketingGuide:
        created = new UIComponent_DebugUIMarketingGuide(player, data, this);
        break;
    case EUIScene_Tooltips:
        created = new UIComponent_Tooltips(player, data, this);
        break;
    case EUIScene_PressStartToPlay:
        created = new UIComponent_PressStartToPlay(player, data, this);
        break;
    case EUIScene_MenuBackground:
        created = new UIComponent_MenuBackground(player, data, this);
        break;
    case EUIScene_WiiUDRC:
        created = new UIComponent_WiiUDRC(player, data, this);
        break;
    default:
        return NULL;
    }

    const bool visible = scene != EUIScene_TutorialPopup && scene != EUIScene_HUD;
    m_componentStates[scene] = std::make_pair(1, visible);

    if (created != NULL)
    {
        m_components.push_back(created);
        return created;
    }
    return NULL;
}

void UILayer::showComponent(int player, EUIScene scene, bool visible)
{
    boost::unordered_map<EUIScene, std::pair<int, bool> >::iterator it = m_componentStates.find(scene);
    if (it != m_componentStates.end())
        it->second.second = visible;
    else if (visible)
        addComponent(player, scene, NULL);
}

void UILayer::tick()
{
    if (gConsoleUIController.IsReloadingSkin())
        return;

    std::vector<UIScene*> pendingDestroy;
    for (std::vector<UIScene*>::iterator it = m_pendingDestroy.begin(); it != m_pendingDestroy.end(); ++it)
        pendingDestroy.push_back(*it);
    m_pendingDestroy.clear();

    for (std::vector<UIScene*>::iterator it = pendingDestroy.begin(); it != pendingDestroy.end(); ++it)
    {
        UIScene* scene = *it;
        if (!scene->isReadyToDelete())
        {
            m_pendingDestroy.push_back(scene);
        }
        else
        {
            for (std::vector<UIScene*>::iterator reload = m_pendingReload.begin(); reload != m_pendingReload.end();)
            {
                if (*reload == scene)
                    reload = m_pendingReload.erase(reload);
                else
                    ++reload;
            }
            if (scene != NULL)
                delete scene;
        }
    }

    while (!m_pendingReload.empty())
    {
        UIScene* scene = m_pendingReload.back();
        m_pendingReload.pop_back();
        scene->destroyMovie();
    }
    m_pendingReload.clear();

    for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->tick();

    for (unsigned int i = m_scenes.size() - 1; static_cast<int>(i) >= 0 && i < m_scenes.size(); --i)
        m_scenes[i]->tick();
}

void UILayer::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!gConsoleUIController.IsExpectingOrReloadingSkin())
    {
        for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
        {
            EUIScene type = (EUIScene)(*it)->getSceneType();
            boost::unordered_map<EUIScene, std::pair<int, bool> >::iterator state = m_componentStates.find(type);
            if (state != m_componentStates.end() && state->second.second && (*it)->isVisible())
            {
                PIXBeginNamedEvent(0.0f, "Rendering component %d", (*it)->getSceneType());
                (*it)->render(width, height, viewport);
                PIXEndNamedEvent();
            }
        }
    }

    if (!m_scenes.empty())
    {
        unsigned int first = 0;
        for (int i = (int)m_scenes.size() - 1; i >= 0; --i)
            if (m_scenes[i]->hidesLowerScenes())
            {
                first = (unsigned int)i;
                break;
            }
        for (unsigned int i = first; i < m_scenes.size(); ++i)
        {
            UIScene* scene = m_scenes[i];
            if (scene->isVisible()
                && (!gConsoleUIController.IsExpectingOrReloadingSkin() || scene->getSceneType() == EUIScene_Timer))
            {
                PIXBeginNamedEvent(0.0f, "Rendering scene %d", scene->getSceneType());
                scene->render(width, height, viewport);
                PIXEndNamedEvent();
            }
        }
    }
}

void UILayer::DestroyAll()
{
    for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->destroyMovie();
    for (std::vector<UIScene*>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it)
        (*it)->destroyMovie();
}

void UILayer::ReloadAll(bool force)
{
    for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->reloadMovie(force);
    if (!m_scenes.empty())
        for (unsigned int i = 0; i < m_scenes.size(); ++i)
            m_scenes[i]->reloadMovie(force);
}

bool UILayer::IsSceneInStack(EUIScene scene)
{
    bool found = false;
    for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
        if (m_scenes[i]->getSceneType() == static_cast<int>(scene))
        {
            found = true;
            break;
        }
    return found;
}

bool UILayer::updateFocusState(bool requestFocus)
{
    if (!requestFocus)
        requestFocus = m_group->RequestFocus(this);

    m_pauseMenuDisplayed = false;
    m_ignoreAutosaveMenuDisplayed = false;
    m_containerMenuDisplayed = false;
    m_menuDisplayed = false;
    m_ignorePlayerJoinMenuDisplayed = false;
    bool granted = false;

    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
    {
        UIScene* scene = *it;

        if (!granted && requestFocus && scene->stealsFocus())
        {
            scene->gainFocus();
            granted = true;
        }
        else
        {
            scene->loseFocus();
            if (scene->getSceneType() == EUIScene_SettingsListMenu)
            {
                scene->loseFocus();
                m_pendingReload.push_back(scene);
            }
        }

        m_menuDisplayed = true;
        int sceneType = scene->getSceneType();

        switch (sceneType)
        {
        case EUIScene_FullscreenProgress:
        case EUIScene_DeathMenu:
        case EUIScene_SignEntryMenu:
        case EUIScene_EndPoem:
        case EUIScene_WritableBookMenu:
            m_ignoreAutosaveMenuDisplayed = true;
            break;
        case EUIScene_PauseMenu:
#ifdef MINECRAFT_PC
        case EUIScene_PcGlideSections:
#endif
            m_pauseMenuDisplayed = true;
            break;
        case EUIScene_CraftingMenu2x2:
        case EUIScene_CraftingMenu:
        case EUIScene_FurnaceMenu:
        case EUIScene_ContainerMenu:
        case EUIScene_ContainerMenuLarge:
        case EUIScene_InventoryMenu:
        case EUIScene_DispenserMenu:
        case EUIScene_CreativeMenu:
        case EUIScene_EnchantingMenu:
        case EUIScene_BrewingStandMenu:
        case EUIScene_TradingMenu:
        case EUIScene_AnvilMenu:
        case EUIScene_HopperMenu:
        case EUIScene_BeaconMenu:
        case EUIScene_HorseInventoryMenu:
        case EUIScene_ClassicCraftingMenu:
            m_containerMenuDisplayed = true;
            m_ignoreAutosaveMenuDisplayed = true;
            break;
        default:
#ifdef MINECRAFT_PC
        {
            bool container = false;
            if (PcModHooksIsScreen(sceneType, &container) && container)
            {
                m_containerMenuDisplayed = true;
                m_ignoreAutosaveMenuDisplayed = true;
            }
        }
#endif
            break;
        }

        if (sceneType == EUIScene_FullscreenProgress || sceneType == EUIScene_PauseMenu
            || sceneType == EUIScene_LeaderboardsMenu || sceneType == EUIScene_Credits || sceneType == EUIScene_EndPoem
            || sceneType == EUIScene_MessageBox)
            m_ignorePlayerJoinMenuDisplayed = true;
    }

    m_hasFocus = granted;
    return granted;
}

void UILayer::closeAllScenes()
{
    std::vector<UIScene*> closing(m_scenes.begin(), m_scenes.end());
    m_scenes.clear();

    for (std::vector<UIScene*>::iterator it = closing.begin(); it != closing.end(); ++it)
    {
        gConsoleUIController.TouchBoxesClear(*it);
        m_pendingDestroy.push_back(*it);
        (*it)->handleDestroy();
    }

    updateFocusState(false);
    if (m_hasFocus)
    {
        m_hasFocus = false;
        m_group->UpdateFocusState();
    }
}

bool UILayer::NavigateToScene(int player, EUIScene scene, void* data)
{
    UIScene* created = NULL;
    switch (static_cast<int>(scene))
    {
    case EUIScene_InventoryMenu:
        created = new UIScene_InventoryMenu(player, data, this);
        break;
    case EUIScene_CreativeMenu:
        created = new UIScene_CreativeMenu(player, data, this);
        break;
    case EUIScene_ContainerMenu:
    case EUIScene_ContainerMenuLarge:
        created = new UIScene_ContainerMenu(player, data, this);
        break;
    case EUIScene_BrewingStandMenu:
        created = new UIScene_BrewingStandMenu(player, data, this);
        break;
    case EUIScene_DispenserMenu:
        created = new UIScene_DispenserMenu(player, data, this);
        break;
    case EUIScene_EnchantingMenu:
        created = new UIScene_EnchantingMenu(player, data, this);
        break;
    case EUIScene_FurnaceMenu:
        created = new UIScene_FurnaceMenu(player, data, this);
        break;
    case EUIScene_CraftingMenu2x2:
    case EUIScene_CraftingMenu:
        created = new UIScene_CraftingMenu(player, data, this);
        break;
    case EUIScene_TradingMenu:
        created = new UIScene_TradingMenu(player, data, this);
        break;
    case EUIScene_AnvilMenu:
        created = new UIScene_AnvilMenu(player, data, this);
        break;
    case EUIScene_HopperMenu:
        created = new UIScene_HopperMenu(player, data, this);
        break;
    case EUIScene_BeaconMenu:
        created = new UIScene_BeaconMenu(player, data, this);
        break;
    case EUIScene_HorseInventoryMenu:
        created = new UIScene_HorseInventoryMenu(player, data, this);
        break;
    case EUIScene_ClassicCraftingMenu:
        created = new UIScene_ClassicCraftingMenu(player, data, this);
        break;
    case EUIScene_HelpAndOptionsMenu:
        created = new UIScene_HelpAndOptionsMenu(player, data, this);
        break;
    case EUIScene_SettingsMenu:
        created = new UIScene_SettingsMenu(player, data, this);
        break;
    case EUIScene_SettingsListMenu:
        created = new UIScene_SettingsListMenu(player, data, this);
        break;
    case EUIScene_SkinSelectMenu:
        created = new UIScene_SkinSelectMenu(player, data, this);
        break;
    case EUIScene_HowToPlayMenu:
        created = new UIScene_HowToPlayMenu(player, data, this);
        break;
    case EUIScene_LanguageSelector:
        created = new UIScene_LanguageSelector(player, data, this);
        break;
    case EUIScene_HowToPlay:
        created = new UIScene_HowToPlay(player, data, this);
        break;
    case EUIScene_ControlsMenu:
        created = new UIScene_ControlsMenu(player, data, this);
        break;
    case EUIScene_ReinstallMenu:
        created = new UIScene_ReinstallMenu(player, data, this);
        break;
    case EUIScene_Credits:
        created = new UIScene_Credits(player, data, this);
        break;
    case EUIScene_PauseMenu:
        created = new UIScene_PauseMenu(player, data, this);
        break;
    case EUIScene_InGameSettingsListMenu:
        created = new UIScene_InGameSettingsListMenu(player, data, this);
        break;
    case EUIScene_DeathMenu:
        created = new UIScene_DeathMenu(player, data, this);
        break;
    case EUIScene_ConnectingProgress:
        created = new UIScene_ConnectingProgress(player, data, this);
        break;
    case EUIScene_SignEntryMenu:
        created = new UIScene_SignEntryMenu(player, data, this);
        break;
    case EUIScene_DebugSetCamera:
        created = new UIScene_DebugSetCamera(player, data, this);
        break;
#ifdef MINECRAFT_PC
    case EUIScene_PcGlideSections:
        created = new UIScene_PcGlideSections(player, data, this);
        break;
#endif
    case EUIScene_DebugFillArea:
        created = new UIScene_DebugFillArea(player, data, this);
        break;
    case EUIScene_DebugCreateSchematic:
        created = new UIScene_DebugCreateSchematic(player, data, this);
        break;
    case EUIScene_WritableBookMenu:
        created = new UIScene_WritableBookMenu(player, data, this);
        break;
    case EUIScene_InGameInfoMenu:
        created = new UIScene_InGameInfoMenu(player, data, this);
        break;
    case EUIScene_GameModeScoreboardMenu:
        created = new UIScene_GameModeScoreboardMenu(player, data, this);
        break;
    case EUIScene_InGameHostOptionsMenu:
        created = new UIScene_InGameHostOptionsMenu(player, data, this);
        break;
    case EUIScene_InGamePlayerOptionsMenu:
        created = new UIScene_InGamePlayerOptionsMenu(player, data, this);
        break;
    case EUIScene_InGameSaveManagementMenu:
        created = new UIScene_InGameSaveManagementMenu(player, data, this);
        break;
    case EUIScene_TeleportMenu:
        created = new UIScene_TeleportMenu(player, data, this);
        break;
    case EUIScene_EndPoem:
        if (IsSceneInStack(EUIScene_EndPoem))
            return false;
        created = new UIScene_EndPoem(player, data, this);
        break;
    case EUIScene_MapVoteMenu:
        created = new UIScene_MapVoteMenu(player, data, this);
        break;
    case EUIScene_TrialExitUpsell:
        created = new UIScene_TrialExitUpsell(player, data, this);
        break;
    case EUIScene_Intro:
        created = new UIScene_Intro(player, data, this);
        break;
    case EUIScene_SaveMessage:
        created = new UIScene_SaveMessage(player, data, this);
        break;
    case EUIScene_MainMenu:
        created = new UIScene_MainMenu(player, data, this);
        break;
    case EUIScene_LoadCreateJoinMenu:
        created = new UIScene_LoadCreateJoinMenu(player, data, this);
        break;
    case EUIScene_LoadMenu:
        created = new UIScene_LoadMenu(player, data, this);
        break;
    case EUIScene_JoinMenu:
        created = new UIScene_JoinMenu(player, data, this);
        break;
    case EUIScene_CreateWorldMenu:
        created = new UIScene_CreateWorldMenu(player, data, this);
        break;
    case EUIScene_LaunchMoreOptionsMenu:
        created = new UIScene_LaunchMoreOptionsMenu(player, data, this);
        break;
    case EUIScene_FullscreenProgress:
        created = new UIScene_FullscreenProgress(player, data, this);
        break;
    case EUIScene_LeaderboardsMenu:
        created = new UIScene_LeaderboardsMenu(player, data, this);
        break;
    case EUIScene_DLCMainMenu:
        created = new UIScene_DLCMainMenu(player, data, this);
        break;
    case EUIScene_DLCOffersMenu:
        created = new UIScene_DLCOffersMenu(player, data, this);
        break;
    case EUIScene_EULA:
        created = new UIScene_EULA(player, data, this);
        break;
    case EUIScene_NewUpdateMessage:
        created = new UIScene_NewUpdateMessage(player, data, this);
        break;
    case EUIScene_SuperFlatMenu:
        created = new UIScene_SuperFlatMenu(player, data, this);
        break;
    case EUIScene_SuperFlatEditLayer:
        created = new UIScene_SuperFlatEditLayer(player, data, this);
        break;
    case EUIScene_SuperFlatPresets:
        created = new UIScene_SuperFlatPresets(player, data, this);
        break;
    case EUIScene_MapSelectMenu:
        created = new UIScene_MapSelectMenu(player, data, this);
        break;
    case EUIScene_BiomePreviewMenu:
        created = new UIScene_BiomePreviewMenu(player, data, this);
        break;
    case EUIScene_Keyboard:
        created = new UIScene_Keyboard(player, data, this);
        break;
    case EUIScene_QuadrantSignin:
        created = new UIScene_QuadrantSignin(player, data, this);
        break;
    case EUIScene_MessageBox:
        if (IsSceneInStack(EUIScene_MessageBox))
            return false;
        created = new UIScene_MessageBox(player, data, this);
        break;
    case EUIScene_Timer:
        created = new UIScene_Timer(player, data, this);
        break;
    case EUIScene_AvatarSelectMenu:
        created = new UIScene_AvatarSelectMenu(player, data, this);
        break;
    case EUIScene_AchievementsMenu:
        created = new UIScene_AchievementsMenu(player, data, this);
        break;
    default:
#ifdef MINECRAFT_PC
        if (PcModHooksIsScreen((int)scene, 0))
        {
            created = PcModHooksMakeScreen((int)scene, player, data, this);
            break;
        }
#endif
        return false;
    }

    if (created == NULL)
        return false;

    telemetryManagerInstance->RecordMenuShown(player, scene, created->getSubSceneType());

    if (!m_scenes.empty())
    {
        UIScene*& top = m_scenes[m_scenes.size() - 1];
        created->setBackScene(top);
    }
    m_scenes.push_back(created);
    updateFocusState(false);
    created->tick();
    return true;
}

void UILayer::removeScene(UIScene* scene)
{
    gConsoleUIController.TouchBoxesClear(scene);

    m_scenes.erase(std::remove(m_scenes.begin(), m_scenes.end(), scene), m_scenes.end());

    m_pendingDestroy.push_back(scene);
    scene->handleDestroy();

    bool focusChangedBefore = m_hasFocus;
    updateFocusState(false);
    bool focusChangedAfter = m_hasFocus;

    if (focusChangedAfter || focusChangedBefore)
    {
        m_hasFocus = false;
        m_group->UpdateFocusState();
    }
}

bool UILayer::NavigateBack(int player, EUIScene scene)
{
    if (m_scenes.empty())
        return false;

    bool navigated;
    if (static_cast<int>(scene) < EUIScene_None)
    {
        while (true)
        {
            UIScene* top = m_scenes.back();
            if (top->getSceneType() == static_cast<int>(scene))
            {
                navigated = true;
                break;
            }
            if (!top->hasFocus(player))
            {
                navigated = false;
                break;
            }
            removeScene(top);
            if (m_scenes.empty())
            {
                navigated = false;
                break;
            }
        }
    }
    else
    {
        UIScene* top = m_scenes.back();
        if (top->hasFocus(player))
        {
            removeScene(top);
            navigated = true;
        }
        else
        {
            navigated = false;
        }
    }
    return navigated;
}

UIScene* UILayer::GetTopScene()
{
    return m_scenes.empty() ? NULL : m_scenes[m_scenes.size() - 1];
}

bool UILayer::HasFocus(int controllerId)
{
    bool result = false;
    if (m_hasFocus)
    {
        for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
        {
            if (m_scenes[i]->stealsFocus())
            {
                if (m_scenes[i]->hasFocus(controllerId))
                    result = true;
                break;
            }
        }
    }
    return result;
}

bool UILayer::hidesLowerScenes()
{
    bool hides = false;
    for (std::vector<UIScene*>::iterator it = m_components.begin(); it != m_components.end(); ++it)
        if ((*it)->hidesLowerScenes())
        {
            hides = true;
            break;
        }
    if (!hides && !m_scenes.empty())
        for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
            if (m_scenes[i]->hidesLowerScenes())
            {
                hides = true;
                break;
            }
    return hides;
}

bool UILayer::isComponentVisible(EUIScene scene)
{
    boost::unordered_map<EUIScene, std::pair<int, bool> >::iterator it = m_componentStates.find(scene);
    return it == m_componentStates.end() ? false : it->second.second;
}

void UILayer::removeComponent(EUIScene scene)
{
    boost::unordered_map<EUIScene, std::pair<int, bool> >::iterator state = m_componentStates.find(scene);
    if (state == m_componentStates.end())
        return;

    if (--state->second.first > 0)
        return;

    m_componentStates.erase(state);

    std::vector<UIScene*>::iterator it = m_components.begin();
    while (it != m_components.end())
    {
        if ((*it)->getSceneType() != static_cast<int>(scene))
        {
            ++it;
            continue;
        }
        gConsoleUIController.TouchBoxesClear(*it);
        m_pendingDestroy.push_back(*it);
        (*it)->handleDestroy();
        it = m_components.erase(it);
    }
}

void UILayer::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
    {
        UIScene* scene = *it;
        if (scene->hasFocus(playerId) && scene->m_tickedLastFrame)
        {
            if (repeat && !scene->allowRepeat(action))
                return;
            scene->handleInput(playerId, action, repeat, pressed, released, handled);
        }

        handled = handled || scene->hidesLowerScenes() || scene->blocksInput();
        if (handled)
            break;
    }
}

UIScene* UILayer::getCurrentScene(int controllerId)
{
    UIScene* current = NULL;
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
    {
        UIScene* scene = *it;
        if (scene->hasFocus(controllerId) && scene->m_tickedLastFrame)
        {
            current = scene;
            break;
        }
    }
    return current;
}

bool UILayer::IsFullscreenGroup()
{
    return m_group->IsFullscreenGroup();
}

C4JRender::eViewportType UILayer::getViewport()
{
    return static_cast<C4JRender::eViewportType>(m_group->GetViewportType());
}

void UILayer::HandleDLCMountingComplete()
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleDLCMountingComplete();
}

void UILayer::HandleDLCInstalled()
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleDLCInstalled();
}

void UILayer::HandleDLCLicenseChange()
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleDLCLicenseChange();
}

void UILayer::HandleTMSBanFileRetrieved()
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleTMSBanFileRetrieved();
}

void UILayer::HandleMessage(EUIMessage message, void* data)
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleMessage(message, data);
}

void UILayer::HandleSaveDeviceRemoved(int device)
{
    for (std::vector<UIScene*>::reverse_iterator it = m_scenes.rbegin(); it != m_scenes.rend(); ++it)
        (*it)->HandleSaveDeviceRemoved(device);
}

void UILayer::handleUnlockFullVersion()
{
    for (std::vector<UIScene*>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it)
        (*it)->handleUnlockFullVersion();
}

UIScene* UILayer::FindScene(EUIScene scene)
{
    for (unsigned int i = 0; i < m_scenes.size(); ++i)
        if (m_scenes[i]->getSceneType() == static_cast<int>(scene))
            return m_scenes[i];
    return NULL;
}

void UILayer::DestroyAllScenes()
{
    closeAllScenes();
}

bool UILayer::GetMenuDisplayed()
{
    return m_menuDisplayed;
}
