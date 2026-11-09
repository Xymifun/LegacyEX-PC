#include "game/ui/UIScene.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_Document.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HowToPlay.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/PIXNamedEvents.hpp"

template <typename T>
std::wstring _toString(T value);

UIScene::UIScene(int playerId, UILayer* layer)
{
    m_opacity = 1.0f;
    m_opacityInitialised = false;
    m_visible = true;
    m_tickedLastFrame = false;
    m_backScene = 0;
    m_callbackId = 0;
    m_reloading = false;
    m_hasGainedFocus = false;
    m_layer = layer;
    m_hasFocus = false;
    m_playerId = playerId;
    m_hasTicked = false;
    m_slotDrawPending = false;
    m_updatePending = true;
    m_slotDrawCount = 0;
    m_movie = 0;
    m_itemRenderer = 0;
    m_controlFocus = -1;
    m_controlChildFocus = 0;
    m_moviePath = L"";
    m_movieLoaded = false;
}

UIScene::~UIScene()
{
    for (boost::unordered_map<std::wstring, bool>::iterator it = m_substitutionTextures.begin();
         it != m_substitutionTextures.end(); ++it)
    {
        gConsoleUIController.unregisterSubstitutionTexture(it->first, it->second);
    }

    if (m_callbackId != 0)
        gConsoleUIController.UnregisterCallbackId(m_callbackId);

    if (m_movie != 0)
        m_movie->getRootNode()->getFuiNode()->setCallbackScene(0);
    gFui->unloadScene(m_movie);

    delete m_itemRenderer;
}

void UIScene::reloadMovie(bool force)
{
    if (!force && stealsFocus() && getSceneType() != EUIScene_FullscreenProgress && !m_hasFocus)
        return;

    m_reloading = true;
    if (m_movie != 0)
    {
        handlePreUnloadForReload();
        gFui->unloadScene(m_movie);
        m_movie = 0;
        m_controls.clear();
    }

    initialiseMovie();
    handlePreReload();
    for (std::vector<UIControl*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        (*it)->ReInit();
    updateComponents();
    handleReload();

    if (m_movie != 0)
    {
        if (m_movie->getRootNode()->getFuiNode()->isFJ_Document())
            m_movie->getRootNode()->getFuiNode()->asFJ_Document()->SetFocus(m_controlFocus);
    }

    m_reloading = false;
    m_updatePending = true;
}

bool UIScene::needsReloaded()
{
    return m_movie == 0 && (!stealsFocus() || m_hasFocus);
}

bool UIScene::hasMovie()
{
    return m_movie != 0;
}

void UIScene::updateSafeZone()
{
    switch (m_layer->getViewport())
    {
    default:
    case C4JRender::eViewportType_FullScreen:
        setSafeZone(getSafeZoneHalfHeight(), getSafeZoneHalfHeight(), getSafeZoneHalfWidth(), getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_TopHalf:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, 0.0, 0.0);
        break;
    case C4JRender::eViewportType_BottomHalf:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, 0.0);
        break;
    case C4JRender::eViewportType_LeftHalf:
        setSafeZone(0.0, 0.0, getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_RightHalf:
        setSafeZone(0.0, 0.0, 0.0, getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, 0.0, getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        setSafeZone(0.0, getSafeZoneHalfHeight(), getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth());
        break;
    }
}

void UIScene::updateViewportTouchOffset()
{
    gConsoleUIController.updateViewportTouchOffset(m_layer->getViewport());
}

double UIScene::getSafeZoneHalfHeight()
{
    return gConsoleUIController.getScreenHeight() * 0.05f;
}

double UIScene::getSafeZoneHalfWidth()
{
    return gConsoleUIController.getScreenWidth() * 0.05f;
}

unsigned int UIScene::mapElementsAndNames()
{
    return 1;
}

unsigned int UIScene::getSubSceneType() const
{
    return 0;
}

void UIScene::tick()
{
    if (m_reloading)
        return;

    if (m_hasTicked)
        m_tickedLastFrame = true;

    tickTimers();
    for (std::vector<UIControl*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        (*it)->tick();
    m_hasTicked = true;
}

void UIScene::SetFocusToElement(int focus)
{
    if (m_movie->getRootNode()->getFuiNode()->asFJ_Document() != 0)
        m_movie->getRootNode()->getFuiNode()->asFJ_Document()->SetFocus(focus);
    _handleFocusChange(focus, 0);
}

void UIScene::handleTimerComplete(int)
{
}

bool UIScene::stealsFocus()
{
    return true;
}

bool UIScene::hasFocus(int playerId)
{
    return m_hasFocus && playerId == m_playerId;
}

void UIScene::updateTooltips()
{
    if (!gConsoleUIController.IsReloadingSkin())
    {
        gConsoleUIController.SetTooltips(m_playerId, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         false, false);
    }
}

void UIScene::updateComponents()
{
}

void UIScene::handleGainFocus(bool)
{
}

void UIScene::handleLoseFocus()
{
}

bool UIScene::hidesLowerScenes()
{
    return m_hasTicked;
}

bool UIScene::blocksInput()
{
    return false;
}

UIControl* UIScene::GetMainPanel()
{
    return 0;
}

void UIScene::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!m_reloading && m_hasTicked && m_movie != 0)
    {
        gConsoleUIController.setupRenderPosition(viewport);
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    }
}

void UIScene::customDraw(const char*, fuiRect*)
{
}

bool UIScene::allowRepeat(int action)
{
    unsigned int unsignedAction = action;
    return unsignedAction > EControllerActions_MenuY
           && (unsignedAction < EControllerActions_MenuOk || unsignedAction > EControllerActions_MenuCancel);
}

void UIScene::handleInput(int, int, bool, bool, bool, bool&)
{
}

void UIScene::handleDestroy()
{
}

void UIScene::handlePreUnloadForReload()
{
}

void UIScene::handlePreReload()
{
}

void UIScene::handleReload()
{
}

void UIScene::handlePress(int, int)
{
}

void UIScene::handleFocusChange(int, int)
{
}

void UIScene::handleInitFocus(int, int)
{
}

void UIScene::handleCheckboxToggled(int id, bool checked)
{
    for (std::vector<UIControl*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
    {
        UIControl* control = *it;
        if (control->m_controlType == 2 && control->m_controlId == id)
        {
            static_cast<UIControl_CheckBox*>(control)->updateCheckedStatus(checked);
            return;
        }
    }
}

void UIScene::handleCheckboxElementToggled(int, int, bool)
{
}

void UIScene::handleSliderMove(int, int)
{
}

void UIScene::handleSliderElementMove(int, int, int)
{
}

void UIScene::handleAnimationEnd()
{
}

void UIScene::handleSelectionChanged(int)
{
}

void UIScene::handleRequestMoreData(int, bool)
{
}

void UIScene::handleTouchBoxRebuild()
{
}

void UIScene::HandleDLCMountingComplete()
{
}

void UIScene::HandleDLCInstalled()
{
}

void UIScene::HandleDLCLicenseChange()
{
}

void UIScene::HandleTMSBanFileRetrieved()
{
}

void UIScene::HandleMessage(EUIMessage, void*)
{
}

void UIScene::HandleSaveDeviceRemoved(int)
{
}

void UIScene::handleUnlockFullVersion()
{
}

void UIScene::handleTouchInput(unsigned int, int, int, int, bool, bool, bool)
{
}

bool UIScene::isReadyToDelete()
{
    return true;
}

void UIScene::customDrawFui(void* context, const char* name, fuiRect* rect)
{
    static_cast<UIScene*>(context)->customDraw(name, rect);
}

void UIScene::tickTimers()
{
    const int now = static_cast<int>(System::processTimeInMilliSecs());
    boost::unordered_map<int, _TimerInfo>::iterator it = m_timers.begin();
    while (it != m_timers.end())
    {
        if (!it->second.active)
        {
            it = m_timers.erase(it);
            continue;
        }

        if (it->second.triggerTime < now)
        {
            handleTimerComplete(it->first);
            it->second.triggerTime = now + it->second.duration;
        }
        ++it;
    }
}

void UIScene::navigateBack()
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
    gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
}

bool UIScene::hasRegisteredSubstitutionTexture(const std::wstring& name)
{
    return m_substitutionTextures.find(name) != m_substitutionTextures.end();
}

void UIScene::unregisterSubstitutionTexture(const std::wstring& name, bool deleteData)
{
    boost::unordered_map<std::wstring, bool>::iterator found = m_substitutionTextures.find(name);
    m_substitutionTextures[name] = deleteData;
    if (found != m_substitutionTextures.end())
    {
        gConsoleUIController.unregisterSubstitutionTexture(found->first, found->second);
        m_substitutionTextures.erase(name);
    }
}

void UIScene::registerSubstitutionTexture(const std::wstring& name, unsigned char* data, unsigned int size,
                                          bool deleteData)
{
    boost::unordered_map<std::wstring, bool>::iterator found = m_substitutionTextures.find(name);
    if (found != m_substitutionTextures.end())
        gConsoleUIController.unregisterSubstitutionTexture(found->first, found->second);
    m_substitutionTextures[name] = deleteData;
    gConsoleUIController.registerSubstitutionTexture(name, data, size, deleteData);
}

void UIScene::loadMovie()
{
    EnterCriticalSection(&g_uiCriticalSection);
    std::wstring path = getMoviePath();
    if (gRenderer->IsWidescreen())
    {
        path.append(L"720.fui");
        m_movieResolution = 1;
    }
    else
    {
        path.append(L"480.fui");
        m_movieResolution = 2;
    }

    arrayWithLength<unsigned char> movieData = gConsoleUIController.getMovieData(path.c_str());
    m_movie = (fuiFile*)gFui->load(movieData, true, gFui->getResolution());
    m_movie->getRootNode()->getFuiNode()->setCallbackScene(this);
    m_movie->setCustomDrawCallback(customDrawFui, this);
    m_stageWidth = static_cast<int>(m_movie->getStageWidth());
    m_stageHeight = static_cast<int>(m_movie->getStageHeight());
    m_originalStageWidth = m_stageWidth;
    m_originalStageHeight = m_stageHeight;
    LeaveCriticalSection(&g_uiCriticalSection);
}

void UIScene::initialiseMovie()
{
    loadMovie();
    mapElementsAndNames();
    updateSafeZone();
    m_opacityInitialised = true;
}

void UIScene::addTimer(int timerId, int duration)
{
    _TimerInfo info;
    info.duration = duration;
    info.triggerTime = static_cast<int>(System::processTimeInMilliSecs()) + duration;
    info.active = true;
    m_timers[timerId] = info;
}

void UIScene::killTimer(int timerId)
{
    boost::unordered_map<int, _TimerInfo>::iterator found = m_timers.find(timerId);
    if (found != m_timers.end())
        found->second.active = false;
}

unsigned int UIScene::convertGameActionToFuiKeycode(int action)
{
    static const unsigned char firstActions[] = {0, 7, 8, 9, 3, 4};
    static const unsigned char secondActions[] = {5, 1, 2};
    static const unsigned char thirdActions[] = {12, 17, 18};

    if (action >= EControllerActions_MenuA && action <= EControllerActions_MenuDown)
        return firstActions[action];
    if (action >= EControllerActions_MenuLeft && action <= EControllerActions_MenuPageDown)
        return secondActions[action - 7];
    if (action == EControllerActions_MenuStickPress)
        return 100;
    if (action >= EControllerActions_MenuOtherStickPress && action <= EControllerActions_MenuOtherStickDown)
        return thirdActions[action - 13];
    if (action == EControllerActions_MenuOk)
        return 0;
    if (action == EControllerActions_MenuCancel)
        return 7;
    if (action >= EControllerActions_MenuStickPress)
        return 100;
    return action;
}

void UIScene::sendInputToMovie(int action, bool, bool pressed, bool)
{
    if (m_movie != 0)
        gFui->dispatchKeyboardEvent(m_movie, !pressed, convertGameActionToFuiKeycode(action));
}

unsigned int UIScene::GetCallbackUniqueId()
{
    if (m_callbackId == 0)
        m_callbackId = gConsoleUIController.RegisterForCallbackId(this);
    return m_callbackId;
}

bool UIScene::controlHasFocus(int focus)
{
    return m_controlFocus == focus;
}

unsigned int UIScene::getControlChildFocus()
{
    return m_controlChildFocus;
}

void UIScene::customDrawSlotControlFui(fuiRect* rect, int, not_null_ptr<ItemInstance> item, float partialTick,
                                       bool flat, bool drawDecorations, bool drawOverlay)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const float x = rect->xMin;
    const float y = rect->yMin;
    const float scaleX = (rect->xMax - rect->xMin) * 0.0625f;
    const float scaleY = (rect->yMax - rect->yMin) * 0.0625f;

    GlStateManager::enableRescaleNormal();
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(120.0f, 1.0f, 0.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::popMatrix();

    if (item->getPopTime() > 0)
    {
        const float translateX = 8.0f * scaleX + x;
        const float translateY = 12.0f * scaleY + y;
        const float popScale = item->getPopTime() / 5.0f + 1.0f;
        GlStateManager::pushMatrix();
        GlStateManager::translatef(translateX, translateY, 0.0f);
        GlStateManager::scalef(1.0f / popScale, (popScale + 1.0f) * 0.5f, 1.0f);
        GlStateManager::translatef(-translateX, -translateY, 0.0f);
    }

    Minecraft* rendererMinecraft = Minecraft::GetInstance();
    PIXBeginNamedEvent(0.0f, "Render and decorate");
    if (m_itemRenderer == 0)
    {
        m_itemRenderer = new ItemRenderer(rendererMinecraft->getEntityRenderDispatcher(),
                                          rendererMinecraft->getTextureManager(),
                                          Minecraft::GetInstance()->getItemColors());
    }

    gRenderer->StateSetBlendEnable(true);
    gRenderer->StateSetBlendFunc(4, 5);
    gRenderer->StateSetBlendFactor(0xFFFFFFFF);

    std::wstring countText = L"";
    int countColour = -1;
    TextureAtlasSprite* countIcon = 0;
    if (getSceneType() == EUIScene_HUD)
    {
        int overrideCount = item->getItem()->GetOverrideCount();
        if (overrideCount > 0)
            countText = _toString(overrideCount);
        countColour = item->getItem()->GetOverrideCountColour();
        countIcon = item->getItem()->GetOverrideCountIcon(item);
    }

    m_itemRenderer->renderAndDecorateItem(item, x, y, scaleX, scaleY, partialTick, flat, true, !drawOverlay, countIcon);
    PIXEndNamedEvent();

    if (item->getPopTime() > 0)
        GlStateManager::popMatrix();

    if (drawDecorations)
    {
        if (scaleX != 1.0f || scaleY != 1.0f)
        {
            GlStateManager::pushMatrix();
            GlStateManager::scalef(scaleX, scaleY, 1.0f);
            gRenderer->StateSetBlendEnable(true);
            gRenderer->StateSetBlendFunc(4, 5);
            gRenderer->StateSetBlendFactor(0xFFFFFFFF);
            m_itemRenderer->renderGuiItemDecorations(minecraft->mFont, item, static_cast<int>(x / scaleX + 0.5f),
                                                     static_cast<int>(y / scaleY + 0.5f), countText, countColour,
                                                     partialTick);
            GlStateManager::popMatrix();
        }
        else
        {
            m_itemRenderer->renderGuiItemDecorations(minecraft->mFont, item, static_cast<int>(x), static_cast<int>(y),
                                                     countText, countColour, partialTick);
        }
    }

    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}

void UIScene::destroyMovie()
{
    if (m_movie != 0)
    {
        gFui->unloadScene(m_movie);
        m_movie = 0;
    }
    m_controls.clear();
}

void UIScene::UpdateSceneControls()
{
    std::vector<UIControl*>::iterator end = m_controls.end();
    for (std::vector<UIControl*>::iterator it = m_controls.begin(); it != end; ++it)
        (*it)->UpdateControl();
}

void UIScene::removeControl(UIControl_Base* control, bool destroy)
{
    std::string name = control->m_name;
    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->RemoveObject(name, destroy);
    control->removeUINodeRef();
    UpdateSceneControls();
    control->m_removed = true;
    gConsoleUIController.TouchBoxRebuild(control->m_scene);
}

void UIScene::doHorizontalResizeCheck()
{
    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->DoHorizontalResizeCheck();
}

UIScene* UIScene::getBackScene()
{
    return m_backScene;
}

void UIScene::gainFocus()
{
    if (!m_hasFocus)
    {
        if (stealsFocus())
        {
            m_hasFocus = true;
            if (needsReloaded())
                reloadMovie(false);
            updateTooltips();
            updateComponents();

            bool hadFocus = m_hasGainedFocus;
            if (!hadFocus)
            {
                if (m_movie->getRootNode()->getFuiNode()->isFJ_Document())
                    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->SetFocus(-1);
                hadFocus = m_hasGainedFocus;
            }

            handleGainFocus(hadFocus);
            if (!m_hasFocus)
                return;
            m_hasGainedFocus = true;
            return;
        }
        if (!m_hasFocus)
            return;
    }

    if (stealsFocus())
        updateTooltips();
}

void UIScene::loseFocus()
{
    if (m_hasFocus)
    {
        m_hasFocus = false;
        handleLoseFocus();
    }
}

void UIScene::setBackScene(UIScene* scene)
{
    m_backScene = scene;
}

void UIScene::setSafeZone(double top, double bottom, double left, double right)
{
    if (m_movie->getRootNode()->getFuiNode()->getType() == 0x59)
    {
        static_cast<FJ_Hud*>(m_movie->getRootNode()->getFuiNode())
            ->SetSafeZone(static_cast<int>(top), static_cast<int>(bottom), static_cast<int>(left),
                          static_cast<int>(right));
        return;
    }
    if (m_movie->getRootNode()->getFuiNode()->getType() == 0x58)
    {
        static_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode())
            ->SetSafeZone(static_cast<int>(top), static_cast<int>(bottom), static_cast<int>(left),
                          static_cast<int>(right));
        return;
    }
    if (m_movie->getRootNode()->getFuiNode()->getType() == 0x5a)
    {
        static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode())
            ->SetSafeZone(static_cast<int>(top), static_cast<int>(bottom), static_cast<int>(left),
                          static_cast<int>(right));
        return;
    }
    if (m_movie->getRootNode()->getFuiNode()->getType() != 0)
    {
        static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())
            ->SetSafeZone(static_cast<int>(top), static_cast<int>(bottom), static_cast<int>(left),
                          static_cast<int>(right));
    }
}

void UIScene::resetHorizontalResizeCheck()
{
    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->ResetHorizontalResizeCheck();
}

void UIScene::_handleFocusChange(int focus, int childFocus)
{
    m_controlChildFocus = childFocus;
    m_controlFocus = focus;
    handleFocusChange(focus, childFocus);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void UIScene::_handleInitFocus(int focus, int childFocus)
{
    m_controlChildFocus = childFocus;
    m_controlFocus = focus;
    handleFocusChange(focus, childFocus);
}

bool UIScene::controlHasFocus(UIControl_Base* control)
{
    return controlHasFocus(control->m_controlId);
}

int UIScene::getControlFocus()
{
    return m_controlFocus;
}

void UIScene::setOpacity(float opacity)
{
    if (opacity == m_opacity && (!m_opacityInitialised || m_movie == 0))
        return;

    m_opacity = opacity;
    if (m_opacityInitialised)
        m_opacityInitialised = false;
    if (m_movie->getRootNode()->getFuiNode()->isFJ_Hud())
        static_cast<FJ_Hud*>(m_movie->getRootNode()->getFuiNode())->SetAlpha(opacity);
    else
        m_movie->getRootNode()->getFuiNode()->setAlpha(opacity);
}

void UIScene::setVisible(bool visible)
{
    m_visible = visible;
}

bool UIScene::hasTimer(int timerId)
{
    boost::unordered_map<int, _TimerInfo>::iterator found = m_timers.find(timerId);
    if (found != m_timers.end())
        return found->second.active;
    return false;
}

void UIScene::slideLeft()
{
    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->SlideLeft();
}

void UIScene::slideRight()
{
    static_cast<FJ_Document*>(m_movie->getRootNode()->getFuiNode())->SlideRight();
}
