#include "game/ui/UIGroup.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/UIComponent_PressStartToPlay.hpp"
#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "game/ui/UIScene_HUD.hpp"

namespace
{
const int DRC_SCREEN_WIDTH = 854;
const int DRC_SCREEN_HEIGHT = 480;
}

UIGroup::UIGroup(EUIGroup group, int player)
{
    m_group = group;
    m_player = player;
    m_containerMenuDisplayed = false;
    m_menuDisplayed = false;
    m_pauseMenuDisplayed = false;
    m_ignoreAutosaveMenuDisplayed = false;
    m_ignorePlayerJoinMenuDisplayed = false;
    m_updateFocusStateCountdown = 0;
    m_viewportType = 0;

    for (int i = 0; i < EUILayer_Count; ++i)
    {
        m_layers[i] = new UILayer(this);
        m_layers[i]->m_layerId = i;
    }

    if (m_group != EUIGroup_Fullscreen && m_group != EUIGroup_DRC)
        m_tooltips = static_cast<UIComponent_Tooltips*>(
            m_layers[EUILayer_Tooltips]->addComponent(m_player, EUIScene_Tooltips, NULL));
    else
        m_tooltips = static_cast<UIComponent_Tooltips*>(
            m_layers[EUILayer_Tooltips]->addComponent(0, EUIScene_Tooltips, NULL));
    m_hud = NULL;
    m_tutorialPopup = NULL;
    m_pressStartToPlay = NULL;
    m_drc = NULL;

    if (m_group == EUIGroup_DRC)
    {
        m_drc = static_cast<UIComponent_WiiUDRC*>(
            m_layers[EUILayer_Fullscreen]->addComponent(m_player, EUIScene_WiiUDRC, NULL));
    }
    else if (m_group != EUIGroup_Fullscreen)
    {
        m_tutorialPopup = static_cast<UIComponent_TutorialPopup*>(
            m_layers[EUILayer_Tutorial]->addComponent(m_player, EUIScene_TutorialPopup, NULL));
        m_hud = static_cast<UIScene_HUD*>(m_layers[EUILayer_HUD]->addComponent(m_player, EUIScene_HUD, NULL));
    }
    else
    {
        m_pressStartToPlay = static_cast<UIComponent_PressStartToPlay*>(
            m_layers[EUILayer_Tooltips]->addComponent(0, EUIScene_PressStartToPlay, NULL));
    }
    m_commandBufferList = MemoryTracker::genLists(1);
}

void UIGroup::getRenderDimensions(int& width, int& height)
{
    gConsoleUIController.getRenderDimensions(static_cast<C4JRender::eViewportType>(m_viewportType), width, height);
}

unsigned int UIGroup::GetLayerIndex(UILayer* layer)
{
    if (m_layers[0] != layer)
    {
        for (int i = 1; i < EUILayer_Count; ++i)
            if (m_layers[i] == layer)
                return static_cast<unsigned int>(i);
    }
    return 0;
}

unsigned int UIGroup::RequestFocus(UILayer* layer)
{
    unsigned int index = GetLayerIndex(layer);
    if (index == 0)
        return 1;
    for (int i = static_cast<int>(index) - 1; i >= 0; --i)
        if (m_layers[i]->m_hasFocus)
            return 0;
    return 1;
}

void UIGroup::UpdateFocusState()
{
    m_updateFocusStateCountdown = 10;
}

bool UIGroup::IsFullscreenGroup()
{
    return m_group == EUIGroup_Fullscreen;
}

unsigned int UIGroup::GetViewportType()
{
    return static_cast<unsigned int>(m_viewportType);
}

void UIGroup::DestroyAll()
{
    for (int i = 0; i < EUILayer_Count; ++i)
        m_layers[i]->DestroyAll();
}

void UIGroup::ReloadAll()
{
    int i;

    for (i = 0; i < EUILayer_Count; ++i)
    {
        if (m_layers[i]->hidesLowerScenes())
            break;
    }

    if (i < EUILayer_Fullscreen)
        i = EUILayer_Fullscreen;

    for (; i >= 0; --i)
    {
        if (i < EUILayer_Count)
            m_layers[i]->ReloadAll(i != EUILayer_Fullscreen);
    }
}

void UIGroup::_UpdateFocusState()
{
    for (int i = 0; i < EUILayer_Count; ++i)
        if (m_layers[i]->updateFocusState(true))
            return;
}

void UIGroup::tick()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
    {
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->tick();
        if (m_updateFocusStateCountdown > 0 && --m_updateFocusStateCountdown == 0)
            _UpdateFocusState();
    }
}
void UIGroup::render()
{
    int width;
    int height;

    if (m_player < 0 || gProfile->IsSignedIn(m_player))
    {
        if (m_group == EUIGroup_DRC)
        {
            width = DRC_SCREEN_WIDTH;
            height = DRC_SCREEN_HEIGHT;
        }
        else
        {
            height = 0;
            width = 0;
            gConsoleUIController.getRenderDimensions(static_cast<C4JRender::eViewportType>(m_viewportType), width,
                                                     height);
        }

        int i;
        for (i = 0; i < EUILayer_Count; ++i)
        {
            if (m_layers[i]->hidesLowerScenes())
                break;
        }

        for (; i >= 0; --i)
        {
            if (i < EUILayer_Count)
            {
                m_layers[i]->render(width, height, (C4JRender::eViewportType)m_viewportType);
            }
        }
    }
}
int UIGroup::hidesLowerScenes()
{
    if (m_player >= 0 && !gProfile->IsSignedIn(m_player))
        return 0;
    for (int i = EUILayer_HUD; i > 0; --i)
    {
        int hides = m_layers[i]->hidesLowerScenes();
        if (hides)
            return hides;
    }
    return m_layers[0]->hidesLowerScenes();
}
void UIGroup::updateStackStates()
{
    bool menuDisplayed = false;
    bool containerMenuDisplayed = false;
    bool ignoreAutosaveMenuDisplayed = false;
    bool ignorePlayerJoinMenuDisplayed = false;
    bool pauseMenuDisplayed = false;

    for (int i = 0; i < EUILayer_Count; ++i)
    {
        menuDisplayed = menuDisplayed || m_layers[i]->m_menuDisplayed;
        pauseMenuDisplayed = pauseMenuDisplayed || m_layers[i]->m_pauseMenuDisplayed;
        containerMenuDisplayed = containerMenuDisplayed || m_layers[i]->m_containerMenuDisplayed;
        ignoreAutosaveMenuDisplayed = ignoreAutosaveMenuDisplayed || m_layers[i]->m_ignoreAutosaveMenuDisplayed;
        ignorePlayerJoinMenuDisplayed = ignorePlayerJoinMenuDisplayed || m_layers[i]->m_ignorePlayerJoinMenuDisplayed;
    }

    m_pauseMenuDisplayed = pauseMenuDisplayed;
    m_ignorePlayerJoinMenuDisplayed = ignorePlayerJoinMenuDisplayed;
    m_containerMenuDisplayed = containerMenuDisplayed;
    m_ignoreAutosaveMenuDisplayed = ignoreAutosaveMenuDisplayed;
    m_menuDisplayed = menuDisplayed;
}

bool UIGroup::NavigateToScene(int player, EUIScene scene, void* data, EUILayer layer)
{
    bool result = m_layers[layer]->NavigateToScene(player, scene, data);
    updateStackStates();
    return result;
}

int UIGroup::NavigateBack(int player, EUIScene scene, EUILayer layer)
{
    int result = 0;
    for (int i = 0; i < EUILayer_Count; ++i)
    {
        if (layer < EUILayer_Count && i != static_cast<int>(layer))
            continue;
        result = m_layers[i]->NavigateBack(player, scene);
        if (result)
            break;
    }
    updateStackStates();
    return result;
}

void UIGroup::closeAllScenes()
{
    Minecraft* mc = Minecraft::GetInstance();
    if (m_player >= 0 && mc != 0 && mc->mGameModes[m_player] != 0)
    {
        MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[m_player];
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
            tutorial->showTutorialPopup(true);
    }

    for (unsigned int i = 0; i < EUILayer_Count; ++i)
        if (i != EUILayer_Alert)
            m_layers[i]->closeAllScenes();
    updateStackStates();
}
UIScene* UIGroup::GetTopScene(EUILayer layer)
{
    return m_layers[layer]->GetTopScene();
}

bool UIGroup::IsSceneInStack(EUIScene scene)
{
    for (int i = 0; i < EUILayer_Count - 1; ++i)
    {
        bool inStack = m_layers[i]->IsSceneInStack(scene);
        if (inStack)
            return inStack;
    }
    return m_layers[EUILayer_HUD]->IsSceneInStack(scene);
}

bool UIGroup::GetMenuDisplayed()
{
    return m_menuDisplayed;
}

void UIGroup::handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_player >= 0 && !gProfile->IsSignedIn(m_player))
        return;
    for (int i = 0; i < EUILayer_Count; ++i)
    {
        m_layers[i]->handleInput(player, action, repeat, pressed, released, handled);
        if (handled)
            return;
    }
}

UIScene* UIGroup::getCurrentScene(int player)
{
    for (int i = 0; i < EUILayer_Count; ++i)
    {
        UIScene* scene = m_layers[i]->getCurrentScene(player);
        if (scene != 0)
            return scene;
    }
    return 0;
}
void UIGroup::showComponent(int player, EUIScene scene, EUILayer layer, bool show)
{
    m_layers[layer]->showComponent(player, scene, show);
}
UIScene* UIGroup::addComponent(int player, EUIScene scene, EUILayer layer)
{
    return m_layers[layer]->addComponent(player, scene, NULL);
}

void UIGroup::removeComponent(EUIScene scene, EUILayer layer)
{
    m_layers[layer]->removeComponent(scene);
}

void UIGroup::ReloadAllLayers()
{
    for (int i = 0; i < EUILayer_Count; ++i)
        m_layers[i]->ReloadAll(true);
}

void UIGroup::SetViewportType(C4JRender::eViewportType viewport)
{
    if (m_viewportType != viewport)
    {
        m_viewportType = viewport;
        ReloadAllLayers();
    }
}
void UIGroup::HandleDLCMountingComplete()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->HandleDLCMountingComplete();
}

void UIGroup::HandleDLCInstalled()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->HandleDLCInstalled();
}

void UIGroup::HandleDLCLicenseChange()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->HandleDLCLicenseChange();
}

void UIGroup::HandleTMSBanFileRetrieved()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->HandleTMSBanFileRetrieved();
}

void UIGroup::HandleMessage(EUIMessage message, void* data)
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < EUILayer_Count; ++i)
            m_layers[i]->HandleMessage(message, data);
}

void UIGroup::HandleSaveDeviceRemoved(int device)
{
    for (int i = 0; i < EUILayer_Count; ++i)
        m_layers[i]->HandleSaveDeviceRemoved(device);
}

UIScene* UIGroup::FindScene(EUIScene scene)
{
    for (int i = 0; i < EUILayer_Count - 1; ++i)
    {
        UIScene* found = m_layers[i]->FindScene(scene);
        if (found != 0)
            return found;
    }
    return m_layers[EUILayer_HUD]->FindScene(scene);
}

void UIGroup::ClearTooltips()
{
    if (GetTopScene(EUILayer_Alert) == NULL)
    {
        m_tooltips->SetTooltips(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false);
    }
}
