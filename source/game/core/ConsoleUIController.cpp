#include "game/core/ConsoleUIController.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <stdint.h>
#include <climits>
#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/CFontData.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/XLanguage.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerformanceTimer.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/FJ_PressStartToPlay.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIComponent_PressStartToPlay.hpp"
#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_TexturePackList.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#ifdef MINECRAFT_PC
#include "game/client/render/Font.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/Gui.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcTextKeyboard.hpp"
#endif
#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIControl_GridList.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_HUD.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UITTFFont.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmapFont.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "platform/wiiu/WiiU_FileSystem.hpp"
#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "vendor/_TutorialPopupInfo.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern "C" void OSReport(const char*, ...);

OSFastMutex g_uiCriticalSection;
ConsoleUIController gConsoleUIController;

static unsigned int s_trialTimerValue = 0;
static unsigned int s_trialTimerLimitSecs = 40 * 60;
static bool s_uiCriticalSectionInitialized = false;

void PopupToast(void* context, int playerId, int achievementId, int imageId, int value)
{
    (void)context;
    gConsoleUIController.SetAchievementUnlocked(playerId, achievementId, imageId, value);
}

ConsoleUIController::ConsoleUIController()
{
    m_pendingFont = EFont_None;
    m_oldFont = 0;
    m_homeMenuPending = false;
    m_font = 0;
    s_trialTimerValue = 0;
    m_skinThread = 0;
    m_currentFont = 0;
    m_bitmapFontAlt = 0;
    m_cleanupOnReload = false;
    m_autoCleanupOnReload = true;
    m_bitmapFont = 0;
    m_debugConsole = 0;
    InitializeCriticalSection(&m_fontMutex);
    m_initialized = false;
    m_screenHeight = 720.0f;
    m_fontChangePending = false;
    m_screenWidth = 1280.0f;
    for (int i = 0; i < 11; ++i)
        m_skinScenes[i] = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        m_menuDisplayed[pad] = false;
        m_menuDisplayDelay[pad] = 0;
        m_menuDisplayPending[pad] = false;
        for (int action = 0; action < 0x18; ++action)
            m_actionTimer[pad][action] = 0;
    }
    for (int i = 0; i < 6; ++i)
        m_playerFlags[i] = false;
    m_lastUISFXTime = 0;
    m_pressStartPlaying = 0;
    m_lastInputTime = 0;
    m_lastViewport = 0;
    m_viewportPositionDirty = false;
    m_winUserIndex = 0;
    InitializeCriticalSection(&m_renderMutex);
    InitializeCriticalSection(&m_messageBoxMutex);
    InitializeCriticalSection(&m_callbackMutex);
    InitializeCriticalSection(&m_skinThreadMutex);
    InitializeCriticalSection(&m_touchMutex);
    m_inputSuspended = false;
    m_touchHeld = false;
    m_rebuildingTouchBoxes = false;
    m_touchX = 0;
    m_touchY = 0;
    m_touchOffsetX = 0;
    m_touchOffsetY = 0;
    if (!s_uiCriticalSectionInitialized)
    {
        InitializeCriticalSection(&g_uiCriticalSection);
        s_uiCriticalSectionInitialized = true;
    }
    m_resolutionDisableFlags = 0;
}

EUIGroup ConsoleUIController::GetGroup(int playerId)
{
    if (playerId != 0xff && playerId >= 0)
        return static_cast<EUIGroup>(EUIGroup_Player1 + playerId);
    return EUIGroup_Fullscreen;
}

void ConsoleUIController::preInit(int width, int height)
{
    m_initialized = true;
    m_screenWidth = (float)width;
    m_screenHeight = (float)height;
}

void ConsoleUIController::postInit()
{
    gFui->setCustomTextureFn(TextureSubstitutionCreateCallbackFui, TextureSubstitutionDestroyCallbackFui, this);
    SetupFont();
    loadSkins();
    m_groups[EUIGroup_DRC] = new UIGroup(EUIGroup_DRC, -1);
    m_groups[EUIGroup_Fullscreen] = new UIGroup(EUIGroup_Fullscreen, -1);
    m_groups[EUIGroup_Player1] = new UIGroup(EUIGroup_Player1, 0);
    m_groups[EUIGroup_Player2] = new UIGroup(EUIGroup_Player2, 1);
    m_groups[EUIGroup_Player3] = new UIGroup(EUIGroup_Player3, 2);
    m_groups[EUIGroup_Player4] = new UIGroup(EUIGroup_Player4, 3);
    NavigateToScene(0, EUIScene_Intro, 0, EUILayer_Scene, EUIGroup_PlayerDefault);
}

void ConsoleUIController::init(int width, int height)
{
    gFui->setScreenSize((float)width, (float)height);
    preInit(width, height);
    postInit();
}

int ConsoleUIController::TextureSubstitutionCreateCallbackFui(void* context, const std::wstring& name,
                                                              unsigned int* width, unsigned int* height, bool* hasAlpha)
{
    ConsoleUIController* controller = static_cast<ConsoleUIController*>(context);
    boost::unordered_map<std::wstring, _TextureDetail>::iterator found = controller->m_substitutionTextures.find(name);
    if (found == controller->m_substitutionTextures.end())
        return -1;
    if (found->second.textureLoaded)
    {
        *width = 64;
        *height = 64;
        return found->second.texture;
    }
    if (found->second.data == 0)
    {
        *width = 64;
        *height = 64;
        *hasAlpha = true;
        return (int)found->second.size;
    }
    BufferedImage image(found->second.data, found->second.size);
    if (image.getData() == 0)
        return -1;
    image.preMultiplyAlpha();
    int texture = Minecraft::GetInstance()->getTextureManager()->getTexture(&image, C4JRender::eTextureFormat_RGBA8,
                                                                            false);
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(0, 1, 1);
    *width = 64;
    *height = 64;
    return texture;
}

void ConsoleUIController::TextureSubstitutionDestroyCallbackFui(int texture)
{
    Minecraft::GetInstance()->getTextureManager()->releaseTexture(texture);
}

ConsoleUIController::EFont ConsoleUIController::getFontForLanguage(int language)
{
    switch (language)
    {
    case XC_LANGUAGE_JAPANESE:
        return EFont_Japanese;
    case XC_LANGUAGE_ZH_HANS:
        return EFont_ChineseSimplified;
    case XC_LANGUAGE_TCHINESE:
        return EFont_ChineseTraditional;
    case XC_LANGUAGE_KOREAN:
        return EFont_Korean;
    default:
        return EFont_Bitmap;
    }
}

UITTFFont* ConsoleUIController::createFont(EFont font)
{
    if (font == EFont_Bitmap)
        return new UITTFFont("Mojangles_TTF_bitmap", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf", "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0);
    if (font == EFont_Japanese)
        return new UITTFFont("Mojangles_TTF_jaJP", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf", "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0x3f);
    if (font == EFont_ChineseSimplified)
        return new UITTFFont("Mojangled_TTF_cnCN", "Common/Media/font/CHS/MSYH.ttf", 0x3f);
    if (font == EFont_ChineseTraditional)
        return new UITTFFont("Mojangles_TTF_cnTD", "Common/Media/font/CHT/DFHeiMedium-B5.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf", 0x3f);
    if (font == EFont_Korean)
        return new UITTFFont("Mojangles_TTF_koKR", "Common/Media/font/KOR/BOKMSD.ttf",
                             "Common/Media/font/JPN/DFGMaruGothic-Md.ttf", "Common/Media/font/CHT/DFHeiMedium-B5.ttf",
                             0x3f);
    return 0;
}

void ConsoleUIController::updateCurrentFont()
{
    m_currentFont = m_pendingFont;
    if (m_oldFont != 0)
    {
        delete m_oldFont;
        m_oldFont = 0;
    }
}

void ConsoleUIController::SetupFont()
{
    EnterCriticalSection(&m_fontMutex);
    const int primaryPad = gProfile->GetPrimaryPad();
    if (m_fontChangePending)
    {
        if (m_currentFont != m_pendingFont)
        {
            gApp.SetAction(primaryPad, eXuiAction_ReloadSkin, NULL);
            m_fontChangePending = gApp.GetXuiAction(primaryPad) != eXuiAction_ReloadSkin;
            LeaveCriticalSection(&m_fontMutex);
            return;
        }
        m_fontChangePending = false;
    }
    if (m_currentFont == m_pendingFont && UIString::setCurrentLanguage() != 0)
    {
        m_pendingFont = getFontForLanguage(static_cast<int>(UIString::getCurrentLanguage()));
        gApp.mDLCManager.LanguageChanged();
        L10N::loadStringTable();
        if (m_pendingFont == m_currentFont)
        {
            if (gApp.GetGameStarted() != 0)
            {
                gApp.SetAction(primaryPad, eXuiAction_ReloadSkin, NULL);
                LeaveCriticalSection(&m_fontMutex);
                return;
            }
        }
        else
        {
            UITTFFont* replaced = m_font;
            if (replaced != 0)
            {
                m_font = 0;
                m_oldFont = replaced;
            }
            if (m_bitmapFont == 0)
            {
                m_bitmapFont = new fuiBitmapFont(g_mojangles7FontData, 0x3f);
                m_bitmapFont->registerFont();
            }
            if (m_bitmapFontAlt == 0)
            {
                m_bitmapFontAlt = new fuiBitmapFont(g_mojangles11FontData, 0x3f);
                m_bitmapFontAlt->registerFont();
            }
            if (m_pendingFont == EFont_Bitmap)
            {
                m_bitmapFont->registerFont();
                m_bitmapFontAlt->registerFont();
                m_font = createFont(m_pendingFont);
                gFui->setFontIndirection("Mojangles7", m_font->getFontName().c_str());
                gFui->setFontIndirection("Mojangles11", m_font->getFontName().c_str());
            }
            else if (m_pendingFont != EFont_None)
            {
                m_font = createFont(m_pendingFont);
                gFui->setFontIndirection("Mojangles7", m_font->getFontName().c_str());
                gFui->setFontIndirection("Mojangles11", m_font->getFontName().c_str());
            }
            if (m_currentFont != 0)
            {
                gApp.SetAction(primaryPad, eXuiAction_ReloadSkin, NULL);
                bool stillPending = true;
                if (!m_fontChangePending && gApp.GetXuiAction(primaryPad) == eXuiAction_ReloadSkin)
                    stillPending = false;
                m_fontChangePending = stillPending;
                LeaveCriticalSection(&m_fontMutex);
                return;
            }
            updateCurrentFont();
        }
    }
    LeaveCriticalSection(&m_fontMutex);
}

intptr_t ConsoleUIController::loadSkin(const std::wstring& name, const std::wstring& path, int fileIndex)
{
    (void)path;
    intptr_t scene = 0;
    if (name.size() != 0 && gApp.hasArchiveFile(name, false))
    {
        arrayWithLength<unsigned char> bytes = gApp.getArchiveFile(name, false);
        scene = gFui->load(bytes, false, fileIndex);
        delete[] bytes.data;
    }
    return scene;
}

void ConsoleUIController::loadSkins()
{
    std::wstring platformSkin = L"";
    platformSkin = L"skinWiiU.fui";
    if (m_screenHeight == 1080.0f || m_screenHeight > 1080.0f)
    {
        m_skinScenes[0] = loadSkin(platformSkin, L"platformskinHD.fui", 0);
    }
    else
    {
        m_skinScenes[0] = loadSkin(platformSkin, L"platformskin.fui", 0);
    }
    m_skinScenes[1] = loadSkin(L"skinGraphics.fui", L"skinGraphics.fui", 0);
    m_skinScenes[2] = loadSkin(L"skinGraphicsHud.fui", L"skinGraphicsHud.fui", 0);
    m_skinScenes[3] = loadSkin(L"skinGraphicsInGame.fui", L"skinGraphicsInGame.fui", 0);
    m_skinScenes[5] = loadSkin(L"skinGraphicsLabels.fui", L"skinGraphicsLabels.fui", 0);
    m_skinScenes[6] = loadSkin(L"skinLabels.fui", L"skinLabels.fui", 0);
    m_skinScenes[7] = loadSkin(L"skinInGame.fui", L"skinInGame.fui", 0);
    m_skinScenes[8] = loadSkin(L"skinHud.fui", L"skinHud.fui", 0);
    m_skinScenes[10] = loadSkin(L"skin.fui", L"skin.fui", 0);
}

void ConsoleUIController::renderScenes()
{
    PIXBeginNamedEvent(0.0f, "Rendering FUI scenes");
    if (!gApp.GetGameStarted())
    {
        PIXBeginNamedEvent(0.0f, "Rendering fullscreen scenes");
        m_groups[EUIGroup_Fullscreen]->render();
        PIXEndNamedEvent();
#ifdef MINECRAFT_PC
        PcTextKeyboardDraw();
#endif
        PIXBeginNamedEvent(0.0f, "Rendering DRC scenes");
        gRenderer->StartDRCFrame(GetDRCEnabled());
        gFui->setScreenSize(854.0f, 480.0f);
    }
    else
    {
        if (!m_groups[EUIGroup_Fullscreen]->hidesLowerScenes())
        {
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", EUIGroup_Player1);
            m_groups[EUIGroup_Player1]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", EUIGroup_Player2);
            m_groups[EUIGroup_Player2]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", EUIGroup_Player3);
            m_groups[EUIGroup_Player3]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", EUIGroup_Player4);
            m_groups[EUIGroup_Player4]->render();
            PIXEndNamedEvent();
        }
#ifdef MINECRAFT_PC
        bool worldHidden = m_groups[EUIGroup_Fullscreen]->hidesLowerScenes() != 0;
        for (int group = 0; group < EUIGroup_Count && !worldHidden; ++group)
            worldHidden = m_groups[group]->IsSceneInStack(EUIScene_FullscreenProgress)
                          || m_groups[group]->IsSceneInStack(EUIScene_ConnectingProgress);
        if (worldHidden)
            PcClearOutsideUiArea();
#endif
        PIXBeginNamedEvent(0.0f, "Rendering fullscreen scenes");
        m_groups[EUIGroup_Fullscreen]->render();
        PIXEndNamedEvent();
#ifdef MINECRAFT_PC
        PcTextKeyboardDraw();
#endif
        PIXBeginNamedEvent(0.0f, "Rendering DRC scenes");
        gRenderer->StartDRCFrame(GetDRCEnabled());
        gFui->setScreenSize(854.0f, 480.0f);
    }
    if (!GetDRCEnabled())
    {
        const float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        gRenderer->SetClearColour(clearColour);
        gRenderer->Clear(3);
    }
    m_groups[EUIGroup_DRC]->render();
    gFui->setScreenSize(1280.0f, 720.0f);
    PIXEndNamedEvent();
    PIXEndNamedEvent();
}

void ConsoleUIController::render()
{
    gRenderer->Clear(1);
    renderScenes();
}

bool ConsoleUIController::getCleanupOnReload()
{
    return m_cleanupOnReload;
}

void ConsoleUIController::setCleanupOnReload(bool cleanupOnReload)
{
    m_autoCleanupOnReload = cleanupOnReload;
    m_cleanupOnReload = true;
}

bool ConsoleUIController::PendingFontChange()
{
    return getFontForLanguage(XGetLanguage()) != m_currentFont;
}

bool ConsoleUIController::UsingBitmapFont()
{
    return m_currentFont == EFont_Bitmap;
}

void ConsoleUIController::registerSubstitutionTexture(const std::wstring& name, int texture)
{
    if (m_substitutionTextures.find(name) != m_substitutionTextures.end())
    {
        ++m_substitutionTextures[name].referenceCount;
        return;
    }
    m_substitutionTextures[name].referenceCount = 1;
    m_substitutionTextures[name].texture = texture;
    m_substitutionTextures[name].textureLoaded = true;
}

void ConsoleUIController::registerSubstitutionTexture(const std::wstring& name, unsigned char* data, unsigned int size,
                                                      bool deleteData)
{
    if (m_substitutionTextures.find(name) != m_substitutionTextures.end())
    {
        ++m_substitutionTextures[name].referenceCount;
        if (deleteData)
            delete[] data;
        return;
    }
    m_substitutionTextures[name].referenceCount = 1;
    _TextureDetail& detail = m_substitutionTextures[name];
    detail.data = data;
    detail.size = size;
    m_substitutionTextures[name].textureLoaded = false;
}

void ConsoleUIController::unregisterSubstitutionTexture(const std::wstring& name, bool deleteData)
{
    boost::unordered_map<std::wstring, _TextureDetail>::iterator found = m_substitutionTextures.find(name);
    if (found == m_substitutionTextures.end())
        return;
    --m_substitutionTextures[name].referenceCount;
    if (m_substitutionTextures[name].referenceCount < 1)
    {
        if (deleteData)
            delete[] found->second.data;
        m_substitutionTextures.erase(found);
    }
}

arrayWithLength<unsigned char> ConsoleUIController::getMovieData(const std::wstring& path)
{
    long long now = System::processTimeInMilliSecs();
    boost::unordered_map<std::wstring, _CachedMovieData>::iterator found = m_movieCache.find(path);
    if (found == m_movieCache.end())
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(path, false);
        m_movieCache[path] = _CachedMovieData(data, now + 60000);
        return data;
    }
    found->second.expiryTime = now + 60000;
    return found->second.data;
}

void ConsoleUIController::getRenderDimensions(C4JRender::eViewportType viewport, int& width, int& height)
{
    switch (viewport)
    {
    case C4JRender::eViewportType_FullScreen:
        width = (int)m_screenWidth;
        height = (int)m_screenHeight;
        break;
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        width = (int)(m_screenWidth * 0.5f);
        height = (int)(m_screenHeight * 0.5f);
        break;
    }
}

void ConsoleUIController::setupRenderPosition(int x, int y)
{
    m_viewportPositionDirty = true;
    m_renderOffsetX = x;
    m_renderOffsetY = y;
    gFui->setOrigin((float)x, (float)y);
}

void ConsoleUIController::updateViewportTouchOffset(int x, int y)
{
    m_touchOffsetX = x;
    m_touchOffsetY = y;
}

void ConsoleUIController::SetTutorialVisible(int playerId, bool visible)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    if (m_groups[groupId]->getTutorialPopup() != 0)
        m_groups[groupId]->getTutorialPopup()->SetVisible(visible);
}

void ConsoleUIController::SetTutorial(int playerId, Tutorial* tutorial)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    if (m_groups[groupId]->getTutorialPopup() != 0)
        m_groups[groupId]->getTutorialPopup()->m_tutorial = tutorial;
}

bool ConsoleUIController::IsPauseMenuDisplayed(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->m_pauseMenuDisplayed;
}

bool ConsoleUIController::GetMenuDisplayed(int playerId)
{
    return m_menuDisplayed[playerId];
}

void ConsoleUIController::SetTutorialDescription(int playerId, _TutorialPopupInfo* popupInfo)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    UIGroup* playerGroup = m_groups[groupId];
    UIComponent_TutorialPopup* popup = playerGroup->getTutorialPopup();
    if (popup != 0)
    {
        popup->m_containerMenuDisplayed = playerGroup->m_containerMenuDisplayed;
        popup->SetTutorialDescription(popupInfo);
    }
}

void ConsoleUIController::RemoveInteractSceneReference(int playerId, UIScene* scene)
{
    UIGroup* group = m_groups[GetGroup(playerId)];
    if (group->getTutorialPopup() != 0)
        group->getTutorialPopup()->RemoveInteractSceneReference(scene);
}

void ConsoleUIController::updateViewportTouchOffset(C4JRender::eViewportType viewport)
{
    int x = 0;
    int y = 0;
    switch ((int)viewport)
    {
    case 1:
        x = (int)(m_screenWidth * 0.25f);
        break;
    case 2:
        x = (int)(m_screenWidth * 0.25f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 3:
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 4:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 6:
        x = (int)(m_screenWidth * 0.5f);
        break;
    case 7:
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 8:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    }
    updateViewportTouchOffset(x, y);
}

void ConsoleUIController::setupRenderPosition(C4JRender::eViewportType viewport)
{
    if (!m_viewportPositionDirty && m_lastViewport == (int)viewport)
        return;
    m_lastViewport = (int)viewport;
    m_viewportPositionDirty = false;
    int x = 0;
    int y = 0;
    switch ((int)viewport)
    {
    case 1:
        x = (int)(m_screenWidth * 0.25f);
        break;
    case 2:
        x = (int)(m_screenWidth * 0.25f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 3:
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 4:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 6:
        x = (int)(m_screenWidth * 0.5f);
        break;
    case 7:
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 8:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    }
    m_renderOffsetX = x;
    m_renderOffsetY = y;
    gFui->setOrigin((float)x, (float)y);
}

void ConsoleUIController::setupCustomDrawGameState()
{
    m_customDrawLeft = INT_MAX;
    m_customDrawBottom = INT_MIN;
    m_customDrawTop = INT_MAX;
    m_customDrawRight = INT_MIN;
    gRenderer->StartFrame();
    gRenderer->Set_matrixDirty();
    PIXBeginNamedEvent(0.0f, "Final setup");
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(0.0, m_screenWidth, m_screenHeight, 0.0, 1000.0, 3000.0);
    GlStateManager::matrixMode(0);
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.1f);
    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);
    PIXEndNamedEvent();
}

void ConsoleUIController::endCustomDrawGameState()
{
    gRenderer->Clear(1);
    GlStateManager::depthMask(false);
    GlStateManager::disableAlphaTest();
}

void ConsoleUIController::SetMenuDisplayed(int playerId, bool displayed)
{
    if (!displayed)
    {
        if (playerId == 0xff)
        {
            for (int i = 0; i < 4; ++i)
            {
                m_menuDisplayPending[i] = true;
                m_menuDisplayDelay[i] = 10;
            }
            return;
        }
        m_menuDisplayPending[playerId] = true;
        m_menuDisplayDelay[playerId] = 10;
        return;
    }
    if (playerId == 0xff)
    {
        for (int i = 0; i < 4; ++i)
        {
            g_pInput->SetMenuDisplayed(i, true);
            m_menuDisplayed[i] = true;
            m_menuDisplayPending[i] = false;
        }
        return;
    }
    g_pInput->SetMenuDisplayed(playerId, true);
    m_menuDisplayed[playerId] = true;
    m_menuDisplayPending[playerId] = false;
}

void ConsoleUIController::setFullscreenMenuDisplayed(bool displayed)
{
    m_groups[EUIGroup_Fullscreen]->showComponent(gProfile->GetPrimaryPad(), EUIScene_Tooltips, EUILayer_Tooltips,
                                                 displayed);
    m_groups[EUIGroup_Player1]->showComponent(EUIGroup_Player1, EUIScene_Tooltips, EUILayer_Tooltips, !displayed);
    m_groups[EUIGroup_Player2]->showComponent(EUIGroup_Player2, EUIScene_Tooltips, EUILayer_Tooltips, !displayed);
    m_groups[EUIGroup_Player3]->showComponent(EUIGroup_Player3, EUIScene_Tooltips, EUILayer_Tooltips, !displayed);
    m_groups[EUIGroup_Player4]->showComponent(EUIGroup_Player4, EUIScene_Tooltips, EUILayer_Tooltips, !displayed);
}

bool ConsoleUIController::NavigateToScene(int playerId, EUIScene scene, void* sceneData, EUILayer layer,
                                          EUIGroup groupId)
{
    static bool updateMessageShown = false;
    if (scene == EUIScene_LoadCreateJoinMenu && !updateMessageShown
        && GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_DisplayUpdateMessage) != 0)
    {
        scene = EUIScene_NewUpdateMessage;
        updateMessageShown = true;
    }
    if (GetMenuDisplayed(playerId))
    {
        unsigned int sceneValue = (unsigned int)scene;
        if (sceneValue < 0x2e)
        {
            if (4 < sceneValue
                && (sceneValue < 0xd || (0x1c < sceneValue && (sceneValue < 0x1f || sceneValue == 0x2a))))
                return false;
        }
        else
        {
            if (sceneValue < 0x30)
                return false;
            if (0x31 < sceneValue)
            {
                if (sceneValue < 0x34)
                    return false;
                if (0x34 < sceneValue)
                {
                    if (sceneValue < 0x38)
                        return false;
                    if (0x3b < sceneValue && sceneValue < 0x3e)
                        return false;
                }
            }
        }
    }

    PerformanceTimer timer;
    int menuPlayer = 0xff;
    bool navigated;
    if ((int)scene == 4)
    {
        groupId = EUIGroup_Fullscreen;
        layer = EUILayer_Fullscreen;
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[EUIGroup_Fullscreen]->NavigateToScene(playerId, scene, sceneData, layer);
    }
    else if ((int)scene == 0x30)
    {
        groupId = EUIGroup_Fullscreen;
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[EUIGroup_Fullscreen]->NavigateToScene(playerId, scene, sceneData, EUILayer_Scene);
    }
    else
    {
        if ((int)scene == 0x1f)
            layer = EUILayer_Fullscreen;
        if (groupId == EUIGroup_PlayerDefault)
        {
            if (gApp.GetGameStarted())
            {
                if (playerId == 0xff || (unsigned int)playerId > 3)
                    groupId = EUIGroup_Fullscreen;
                else
                {
                    groupId = static_cast<EUIGroup>(EUIGroup_Player1 + playerId);
                    menuPlayer = playerId;
                }
            }
            else
            {
                groupId = EUIGroup_Fullscreen;
                layer = EUILayer_Fullscreen;
            }
        }
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[(int)groupId]->NavigateToScene(playerId, scene, sceneData, layer);
    }
    if (navigated && groupId == EUIGroup_Fullscreen)
        setFullscreenMenuDisplayed(true);
    LeaveCriticalSection(&m_renderMutex);
    timer.PrintElapsedTime(L"Navigate to scene");
    return navigated;
}

bool ConsoleUIController::NavigateBack(int playerId, bool closeCurrent, EUIScene scene, EUILayer layer)
{
    bool result = false;
    bool gameStarted = gApp.GetGameStarted();
    bool navigated = m_groups[EUIGroup_Fullscreen]->NavigateBack(playerId, scene, layer);
    EUIGroup groupId = static_cast<EUIGroup>(playerId + EUIGroup_Player1);
    if (!gameStarted)
    {
        result = navigated;
        if (!m_groups[EUIGroup_Fullscreen]->GetMenuDisplayed())
            SetMenuDisplayed(0xff, false);
    }
    else if (!navigated && playerId != 0xff && playerId >= 0)
    {
        m_groups[groupId]->NavigateBack(playerId, scene, layer);
        if (!m_groups[groupId]->GetMenuDisplayed())
        {
            Tooltips::ForceTooltipReload(playerId);
            SetMenuDisplayed(playerId, false);
        }
    }
    else if (!m_groups[EUIGroup_Fullscreen]->GetMenuDisplayed())
    {
        setFullscreenMenuDisplayed(false);
        SetMenuDisplayed(0, m_groups[EUIGroup_Player1]->GetMenuDisplayed());
        SetMenuDisplayed(1, m_groups[EUIGroup_Player2]->GetMenuDisplayed());
        SetMenuDisplayed(2, m_groups[EUIGroup_Player3]->GetMenuDisplayed());
        SetMenuDisplayed(3, m_groups[EUIGroup_Player4]->GetMenuDisplayed());
    }
    return result;
}

void ConsoleUIController::FixupMenuDisplayedAfterNavigateBack()
{
    gConsoleUIController.setFullscreenMenuDisplayed(false);
    for (int player = 0; player < 4; ++player)
        gConsoleUIController.SetMenuDisplayed(player, m_groups[player + 2]->GetMenuDisplayed());
}

UIScene* ConsoleUIController::GetTopScene(int playerId, EUILayer layer, EUIGroup groupId)
{
    if (groupId == EUIGroup_PlayerDefault)
    {
        if (gApp.GetGameStarted())
        {
            groupId = GetGroup(playerId);
            return m_groups[(int)groupId]->GetTopScene(layer);
        }
        groupId = EUIGroup_Fullscreen;
        layer = EUILayer_Fullscreen;
    }
    return m_groups[(int)groupId]->GetTopScene(layer);
}

unsigned int ConsoleUIController::RegisterForCallbackId(UIScene* scene)
{
    EnterCriticalSection(&m_callbackMutex);
    unsigned int tickCount = GetTickCount();
    unsigned int callbackId = (tickCount & 0x00ffffff) | ((unsigned int)scene->getSceneType() << 24);
    m_callbackScenes[callbackId] = scene;
    LeaveCriticalSection(&m_callbackMutex);
    return callbackId;
}

void ConsoleUIController::UnregisterCallbackId(unsigned int callbackId)
{
    EnterCriticalSection(&m_callbackMutex);
    boost::unordered_map<unsigned int, UIScene*>::iterator found = m_callbackScenes.find(callbackId);
    if (found != m_callbackScenes.end())
        m_callbackScenes.erase(found);
    LeaveCriticalSection(&m_callbackMutex);
}

UIScene* ConsoleUIController::GetSceneFromCallbackId(unsigned int callbackId)
{
    UIScene* scene = 0;
    boost::unordered_map<unsigned int, UIScene*>::iterator found = m_callbackScenes.find(callbackId);
    if (found != m_callbackScenes.end())
        scene = found->second;
    return scene;
}

void ConsoleUIController::EnterCallbackIdCriticalSection()
{
    EnterCriticalSection(&m_callbackMutex);
}

void ConsoleUIController::LeaveCallbackIdCriticalSection()
{
    LeaveCriticalSection(&m_callbackMutex);
}

void ConsoleUIController::CloseUIScenes(int playerId, bool immediate)
{
    EUIGroup groupId;
    UIGroup* playerGroup;
    if (!gApp.GetGameStarted() && !immediate)
    {
        groupId = EUIGroup_Fullscreen;
        EnterCriticalSection(&g_uiCriticalSection);
        playerGroup = m_groups[EUIGroup_Fullscreen];
    }
    else
    {
        groupId = GetGroup(playerId);
        EnterCriticalSection(&g_uiCriticalSection);
        playerGroup = m_groups[groupId];
    }
    playerGroup->closeAllScenes();
    playerGroup->ClearTooltips();
    Tooltips::ForceTooltipReload(playerId);
    if (playerGroup->getTutorialPopup() != 0)
    {
        _TutorialPopupInfo popupInfo;
        playerGroup->getTutorialPopup()->SetTutorialDescription(&popupInfo);
    }
    if (groupId == EUIGroup_Fullscreen)
        setFullscreenMenuDisplayed(false);
    bool sharedDisplayed = m_groups[EUIGroup_Fullscreen]->GetMenuDisplayed();
    SetMenuDisplayed(0, sharedDisplayed || m_groups[EUIGroup_Player1]->GetMenuDisplayed());
    SetMenuDisplayed(1, sharedDisplayed || m_groups[EUIGroup_Player2]->GetMenuDisplayed());
    SetMenuDisplayed(2, sharedDisplayed || m_groups[EUIGroup_Player3]->GetMenuDisplayed());
    SetMenuDisplayed(3, sharedDisplayed || m_groups[EUIGroup_Player4]->GetMenuDisplayed());
    LeaveCriticalSection(&g_uiCriticalSection);
}

void ConsoleUIController::CloseAllPlayersScenes(bool immediate)
{
    EnterCriticalSection(&g_uiCriticalSection);
    m_groups[EUIGroup_Fullscreen]->ClearTooltips();
    m_groups[EUIGroup_DRC]->closeAllScenes();
    m_groups[EUIGroup_DRC]->ClearTooltips();
    m_groups[EUIGroup_Fullscreen]->closeAllScenes();
    m_groups[EUIGroup_Fullscreen]->ClearTooltips();
    m_groups[EUIGroup_Player1]->closeAllScenes();
    m_groups[EUIGroup_Player1]->ClearTooltips();
    m_groups[EUIGroup_Player2]->closeAllScenes();
    m_groups[EUIGroup_Player2]->ClearTooltips();
    m_groups[EUIGroup_Player3]->closeAllScenes();
    m_groups[EUIGroup_Player3]->ClearTooltips();
    m_groups[EUIGroup_Player4]->closeAllScenes();
    m_groups[EUIGroup_Player4]->ClearTooltips();
    if (!m_groups[EUIGroup_Fullscreen]->GetMenuDisplayed())
    {
        SetMenuDisplayed(0, false);
        SetMenuDisplayed(1, false);
        SetMenuDisplayed(2, false);
        SetMenuDisplayed(3, false);
    }
    setFullscreenMenuDisplayed(false);
    LeaveCriticalSection(&g_uiCriticalSection);
}

bool ConsoleUIController::IsSceneInStack(int playerId, EUIScene scene)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->IsSceneInStack(scene);
}

bool ConsoleUIController::IsContainerMenuDisplayed(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->m_containerMenuDisplayed;
}

bool ConsoleUIController::IsIgnorePlayerJoinMenuDisplayed(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->m_ignorePlayerJoinMenuDisplayed;
}

bool ConsoleUIController::IsIgnoreAutosaveMenuDisplayed(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[EUIGroup_Fullscreen]->m_ignoreAutosaveMenuDisplayed
           || (groupId != EUIGroup_Fullscreen && m_groups[groupId]->m_ignoreAutosaveMenuDisplayed);
}

void ConsoleUIController::SetIgnoreAutosaveMenuDisplayed(int playerId, bool ignore)
{
}

void ConsoleUIController::CheckMenuDisplayed()
{
    for (int player = 0; player < 4; ++player)
    {
        if (!m_menuDisplayPending[player])
            continue;
        if (m_menuDisplayDelay[player] != 0)
        {
            --m_menuDisplayDelay[player];
            continue;
        }
        m_menuDisplayPending[player] = false;
        m_menuDisplayed[player] = false;
        g_pInput->SetMenuDisplayed(player, false);
    }
}

void ConsoleUIController::AnimateKeyPress(int playerId, int action, bool repeat, bool pressed, bool released)
{
    if (!pressed)
        return;
    EUIGroup groupId = static_cast<EUIGroup>(playerId + EUIGroup_Player1);
    UIGroup* group = (gApp.GetGameStarted() && playerId != 0xff && (unsigned int)playerId < 4)
                         ? m_groups[groupId]
                         : m_groups[EUIGroup_Fullscreen];
    if (group->getTooltips() != 0)
    {
        bool handled = false;
        group->getTooltips()->handleInput(playerId, action, repeat, pressed, released, handled);
    }
}

void ConsoleUIController::OverrideSFX(int playerId, int action, bool enabled)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        m_groups[groupId]->getTooltips()->overrideSFX(playerId, action, enabled);
}

void ConsoleUIController::SetTooltipText(unsigned int playerId, unsigned int tooltipIndex, int textId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        m_groups[groupId]->getTooltips()->SetTooltipText(tooltipIndex, textId);
}

void ConsoleUIController::SetEnableTooltips(unsigned int playerId, bool enable)
{
    int groupId = 1;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        m_groups[groupId]->getTooltips()->SetEnableTooltips(enable);
}

void ConsoleUIController::ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool show)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        m_groups[groupId]->getTooltips()->ShowTooltip(tooltipIndex, show);
}

void ConsoleUIController::SetTooltips(unsigned int playerId, int* tooltips, bool show, bool useGlobalGroup)
{
    if (gConsoleUIController.IsReloadingSkin())
        return;
    UIGroup* tooltipGroup;
    if (gApp.GetGameStarted() && !useGlobalGroup)
        tooltipGroup = m_groups[GetGroup((int)playerId)];
    else
        tooltipGroup = m_groups[EUIGroup_Fullscreen];
    if (tooltipGroup->getTooltips() != 0)
        tooltipGroup->getTooltips()->SetTooltips((int)playerId, tooltips, show);
}

void ConsoleUIController::SetTooltips(unsigned int playerId, int tooltip0, int tooltip1, int tooltip2, int tooltip3,
                                      int tooltip4, int tooltip5, int tooltip6, int tooltip7, int tooltip8,
                                      int tooltip9, int tooltip10, int tooltip11, int tooltip12, int tooltip13,
                                      int tooltip14, int tooltip15, bool show, bool useGlobalGroup)
{
    if (gConsoleUIController.IsReloadingSkin())
        return;
    if (tooltip2 == IDS_TOOLTIPS_SELECTDEVICE || tooltip2 == IDS_TOOLTIPS_CHANGEDEVICE)
        tooltip2 = -1;
    if (tooltip3 == IDS_TOOLTIPS_VIEW_GAMERCARD || tooltip3 == IDS_TOOLTIPS_VIEW_GAMERPROFILE)
        tooltip3 = -1;
    UIGroup* tooltipGroup = gApp.GetGameStarted() && !useGlobalGroup ? m_groups[GetGroup((int)playerId)]
                                                                     : m_groups[EUIGroup_Fullscreen];
    if (tooltipGroup->getTooltips() != 0)
        tooltipGroup->getTooltips()->SetTooltips((int)playerId, tooltip0, tooltip1, tooltip2, tooltip3, tooltip4,
                                                 tooltip5, tooltip6, tooltip7, tooltip8, tooltip9, tooltip10, tooltip11,
                                                 tooltip12, tooltip13, tooltip14, tooltip15, show);
}

void ConsoleUIController::EnableTooltip(unsigned int playerId, unsigned int tooltipIndex, bool enable)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        m_groups[groupId]->getTooltips()->EnableTooltip(tooltipIndex, enable);
}

bool ConsoleUIController::IsToolTipDynamic(unsigned int playerId, unsigned int tooltipIndex, EControllerActions& action)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getTooltips() != 0)
        return m_groups[groupId]->getTooltips()->IsToolTipDynamic(tooltipIndex, action);
    return false;
}

void ConsoleUIController::RefreshTooltips(unsigned int playerId)
{
}

void ConsoleUIController::DisplayGamertag(unsigned int playerId, bool show)
{
    if (!GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames))
        show = false;
    EUIGroup groupId = static_cast<EUIGroup>(playerId + EUIGroup_Player1);
    if (m_groups[groupId]->getHUD() != 0)
        m_groups[groupId]->getHUD()->ShowDisplayName(show);
    if (gApp.GetLocalPlayerCount() > 1 && m_groups[groupId]->getTutorialPopup() != 0
        && !m_groups[groupId]->m_containerMenuDisplayed)
        m_groups[groupId]->getTutorialPopup()->UpdateTutorialPopup();
}

void ConsoleUIController::SetSelectedItem(unsigned int playerId, const std::wstring& itemName,
                                          const std::wstring& itemDescription)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    if (m_groups[groupId]->getHUD() != 0)
        m_groups[groupId]->getHUD()->SetSelectedLabel(itemName);
}

void ConsoleUIController::ResetSelectedItem()
{
    for (int i = 0; i < 6; ++i)
    {
        if (m_groups[i]->getHUD() != 0)
            m_groups[i]->getHUD()->SetSelectedLabel(L"");
    }
}

void ConsoleUIController::UpdateSelectedItemPos(unsigned int playerId)
{
}

int ConsoleUIController::GetHotBarX(unsigned int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    int value = 0;
    if (m_groups[groupId]->getHUD() != 0)
        value = m_groups[groupId]->getHUD()->GetHotBarX();
    return value;
}

int ConsoleUIController::GetHotBarY(unsigned int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    int value = 0;
    if (m_groups[groupId]->getHUD() != 0)
        value = m_groups[groupId]->getHUD()->GetHotBarY();
    return value;
}

int ConsoleUIController::GetHotBarWidth(unsigned int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    int value = 0;
    if (m_groups[groupId]->getHUD() != 0)
        value = m_groups[groupId]->getHUD()->GetHotBarWidth();
    return value;
}

int ConsoleUIController::GetHotBarHeight(unsigned int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    int value = 0;
    if (m_groups[groupId]->getHUD() != 0)
        value = m_groups[groupId]->getHUD()->GetHotBarHeight();
    return value;
}

bool ConsoleUIController::GetQuickSelectVisible(unsigned int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    bool visible = false;
    if (m_groups[groupId]->getHUD() != 0)
        visible = m_groups[groupId]->getHUD()->m_quickSelectVisible;
    return visible;
}

bool ConsoleUIController::GetDRCEnabled()
{
    if (m_groups[EUIGroup_DRC]->getDRC() != 0)
        return m_groups[EUIGroup_DRC]->getDRC()->m_drcOutputEnabled;
    return false;
}

void ConsoleUIController::HandleDLCMountingComplete()
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCMountingComplete();
}

void ConsoleUIController::HandleDLCInstalled(int packId)
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCInstalled();
}

void ConsoleUIController::HandleDLCLicenseChange()
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCLicenseChange();
}

void ConsoleUIController::HandleTMSDLCFileRetrieved(int fileId)
{
}

void ConsoleUIController::HandleTMSBanFileRetrieved(int fileId)
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleTMSBanFileRetrieved();
}

void ConsoleUIController::HandleInventoryUpdated(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted() && playerId != 0xff && playerId >= 0)
        groupId = EUIGroup_Player1 + playerId;
    m_groups[groupId]->HandleMessage(EUIMessage_Unknown, 0);
}

void ConsoleUIController::HandleGameTick()
{
    tickInput();
    if (!IsExpectingOrReloadingSkin())
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_groups[i]->getHUD() != 0)
                m_groups[i]->getHUD()->handleGameTick();
        }
    }
}

void ConsoleUIController::HandleSaveDeviceRemoved(int deviceIndex)
{
    m_groups[EUIGroup_Fullscreen]->HandleSaveDeviceRemoved(deviceIndex);
}

bool ConsoleUIController::IsTutorialVisible(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    bool visible = false;
    if (m_groups[groupId]->getTutorialPopup() != 0)
        visible = m_groups[groupId]->getTutorialPopup()->IsVisible();
    return visible;
}

void ConsoleUIController::UpdatePlayerBasePositions()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    for (int i = 0; i < 4; ++i)
    {
        if (minecraft->mLocalPlayers[i] != 0)
        {
            if (minecraft->mLocalPlayers[i]->mViewport != C4JRender::eViewportType_FullScreen)
                DisplayGamertag(i, true);
            else
                DisplayGamertag(i, false);
            m_groups[EUIGroup_Player1 + i]->SetViewportType(
                static_cast<C4JRender::eViewportType>(minecraft->mLocalPlayers[i]->mViewport));
        }
        else
        {
            m_groups[EUIGroup_Player1 + i]->SetViewportType(C4JRender::eViewportType_FullScreen);
            DisplayGamertag(i, false);
        }
    }
}

void ConsoleUIController::SetEmptyQuadrantLogo(int quadrant)
{
}

void ConsoleUIController::ResetPressStartToPlayUIComponents()
{
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->ResetUIComponents();
}

void ConsoleUIController::HideAllGameUIElements()
{
    ResetPressStartToPlayUIComponents();
}

void ConsoleUIController::ShowOtherPlayersBaseScene(unsigned int playerId, bool show)
{
}

void ConsoleUIController::ShowTrialTimer(bool show)
{
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showTrialTimer(show);
}

void ConsoleUIController::SetTrialTimerLimitSecs(unsigned int limitSecs)
{
    s_trialTimerLimitSecs = limitSecs;
}

void ConsoleUIController::UpdateTrialTimer(unsigned int playerId)
{
    unsigned int elapsed = (unsigned int)gApp.getTrialTimer();
    if (elapsed > s_trialTimerLimitSecs)
        elapsed = s_trialTimerLimitSecs;
    unsigned int remaining = s_trialTimerLimitSecs - elapsed;
    if (remaining < 180)
    {
        wchar_t timer[20];
        swprintf(timer, 20, L"%d:%02d", remaining / 60, remaining % 60);
        if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
            m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->setTrialTimer(timer);
    }
    else
    {
        if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() == 0)
            return;
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->setTrialTimer(L"");
    }
    if (remaining == 0 && !gConsoleUIController.GetMenuDisplayed((int)playerId))
    {
        gConsoleUIController.NavigateToScene((int)playerId, EUIScene_PauseMenu, 0, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        gApp.SetAction((int)playerId, eXuiAction_TrialOver, 0);
    }
}

void ConsoleUIController::ReduceTrialTimerValue()
{
    unsigned int elapsed = (unsigned int)(int)gApp.getTrialTimer();
    if (elapsed > s_trialTimerLimitSecs)
        elapsed = s_trialTimerLimitSecs;
    s_trialTimerLimitSecs -= elapsed;
}

void ConsoleUIController::ShowAutosaveCountdownTimer(bool show)
{
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showTrialTimer(show);
}

void ConsoleUIController::UpdateAutosaveCountdownTimer(unsigned int remainingSecs)
{
    wchar_t text[100];
    swprintf(text, 100, L10N::GetString(IDS_AUTOSAVE_COUNTDOWN), remainingSecs);
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->setTrialTimer(text);
}

void ConsoleUIController::ShowSavingMessage(unsigned int playerId, C4JStorage::ESavingMessage message)
{
    static const bool visible[3] = {false, true, true};
    bool show = false;
    if ((unsigned int)message <= 2)
        show = visible[(unsigned int)message];
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showSaveIcon(show);
}

void ConsoleUIController::ShowChestRefillAnimation(bool show)
{
    if (!m_groups[EUIGroup_Fullscreen]->IsSceneInStack(EUIScene_MessageBox)
        && m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
    {
        int yOffset = 0;
        if (!show)
            yOffset = m_groups[EUIGroup_Player1]->getHUD()->GetStatusEffectsYOffset();
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->ShowChestRefillAnimation(show, yOffset);
    }
}

void ConsoleUIController::HideChestRefillAnimation()
{
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->HideChestRefillAnimation();
}

void ConsoleUIController::ShowTimedSplash(unsigned int playerId, const std::wstring& text, unsigned int durationMs)
{
    if (m_groups[GetGroup((int)playerId)]->getHUD() != 0)
        m_groups[GetGroup((int)playerId)]->getHUD()->showTimedSplash(text, durationMs, 1);
}

void ConsoleUIController::ShowPlayerDisplayname(bool show)
{
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPlayerDisplayName(show);
}

bool ConsoleUIController::PressStartPlaying(unsigned int playerId)
{
    return (m_pressStartPlaying >> playerId) & 1;
}

void ConsoleUIController::ShowPressStart(unsigned int playerId)
{
    unsigned int mask = 1 << playerId;
    if ((m_pressStartBlocked & mask) == 0 && gStorage->GetSaveState() != C4JStorage::ESaveGame_SaveIncompleteWaiting)
    {
        m_pressStartPlaying |= mask;
        if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
            m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPressStart((int)playerId, true);
    }
}

void ConsoleUIController::ClearPressStart()
{
    m_pressStartPlaying = 0;
}

void ConsoleUIController::HidePressStart()
{
    ClearPressStart();
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
    {
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPressStart(0, false);
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPressStart(1, false);
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPressStart(2, false);
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->showPressStart(3, false);
    }
}

void ConsoleUIController::SetAchievementUnlocked(int playerId, int achievementId, int imageId, int value)
{
    unsigned int mask = 1 << playerId;
    if ((m_pressStartBlocked & mask) != 0)
        return;
    m_pressStartPlaying |= mask;
    if (m_groups[EUIGroup_Fullscreen]->getPressStartToPlay() != 0)
        m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->SetAchievementUnlocked(playerId, achievementId);
}

bool ConsoleUIController::GetAchievementToastShowing(int playerId)
{
    return m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->GetAchievementToastShowing(playerId);
}

bool ConsoleUIController::GetHoldToShowAchievementsActive(bool includeCancelled)
{
    return m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->GetHoldToShowAchievementsActive(includeCancelled);
}

bool ConsoleUIController::GetHoldActionCancelled(int playerId)
{
    return m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->GetHoldActionCancelled(playerId);
}

struct ProfileSignInMenuData
{
    int titleId;
    int messageId;
    wchar_t** messageParameters;
    unsigned int parameterCount;
    unsigned int userIndex;
    int (*callback)(void*, int, int);
    void* callbackData;
    wchar_t* overrideText;
    unsigned int controllerIndex;
};

int ConsoleUIController::RequestProfileSignInMenu(int titleId, int messageId, wchar_t** messageParameters,
                                                  unsigned int parameterCount, unsigned int userIndex,
                                                  int (*callback)(void*, int, int), void* callbackData,
                                                  wchar_t* overrideText, unsigned int controllerIndex)
{
    ProfileSignInMenuData data = {titleId,  messageId,    messageParameters, parameterCount, userIndex,
                                  callback, callbackData, overrideText,      controllerIndex};
    return gConsoleUIController.NavigateToScene((int)userIndex, EUIScene_AvatarSelectMenu, &data, EUILayer_Popup,
                                                EUIGroup_Fullscreen)
               ? 2
               : 1;
}

void ConsoleUIController::RequestAlertMessage(int titleId, int messageId, int* messageParameters,
                                              unsigned int parameterCount, unsigned int userIndex,
                                              int (*callback)(void*, int, C4JStorage::EMessageResult),
                                              void* callbackData, wchar_t* overrideText, bool blocking)
{
    RequestMessageBox(titleId, messageId, messageParameters, parameterCount, userIndex, callback, callbackData,
                      overrideText, 0, false, blocking);
}

int ConsoleUIController::RequestErrorMessage(int titleId, int messageId, int* messageParameters,
                                             unsigned int parameterCount, unsigned int userIndex,
                                             int (*callback)(void*, int, C4JStorage::EMessageResult),
                                             void* callbackData, wchar_t* overrideText, bool blocking)
{
    gApp.setIgnoreMainMenuInputDuringInvite(false);
    return RequestMessageBox(titleId, messageId, messageParameters, parameterCount, userIndex, callback, callbackData,
                             overrideText, 0, false, blocking);
}

int ConsoleUIController::RequestMessageBox(int titleId, int messageId, int* messageParameters,
                                           unsigned int parameterCount, unsigned int userIndex,
                                           int (*callback)(void*, int, C4JStorage::EMessageResult), void* callbackData,
                                           wchar_t* overrideText, unsigned int buttonFlags, bool alert, bool blocking)
{
    EUILayer layer = alert ? EUILayer_Alert : EUILayer_Popup;
    _MessageBoxData data;
    data.titleId = titleId;
    data.messageId = messageId;
    data.messageParameters = messageParameters;
    data.parameterCount = parameterCount;
    data.userIndex = userIndex;
    data.callback = callback;
    data.callbackData = callbackData;
    data.overrideText = overrideText;
    data.buttonFlags = buttonFlags;
    data.blocking = blocking;
    if (!gConsoleUIController.IsReloadingSkin())
    {
        bool navigated = (!gApp.GetGameStarted() || layer == EUILayer_Alert)
                             ? gConsoleUIController.NavigateToScene((int)userIndex, EUIScene_MessageBox, &data, layer,
                                                                    EUIGroup_Fullscreen)
                             : gConsoleUIController.NavigateToScene((int)userIndex, EUIScene_MessageBox, &data, layer,
                                                                    EUIGroup_PlayerDefault);
        if (navigated)
            return 2;
    }
    else
    {
        _QueuedMessageBoxData* queued = new _QueuedMessageBoxData();
        queued->info = data;
        int* parameters = new int[data.parameterCount];
        queued->info.messageParameters = parameters;
        std::memcpy(parameters, data.messageParameters, data.parameterCount * sizeof(int));
        queued->layer = EUILayer_Alert;
        queued->queuedUserIndex = userIndex;
        EnterCriticalSection(&m_messageBoxMutex);
        m_queuedMessageBoxes.push_back(queued);
        LeaveCriticalSection(&m_messageBoxMutex);
    }
    return 1;
}

void ConsoleUIController::RequestUGCMessageBox(int titleId, int messageId, int buttonId,
                                               int (*callback)(void*, int, C4JStorage::EMessageResult),
                                               void* callbackData)
{
    if (titleId == -1)
        titleId = IDS_FAILED_TO_CREATE_GAME_TITLE;
    if (messageId == -1)
        messageId = (int)0xda0b9859;
    if (buttonId == -1)
        buttonId = gProfile->GetPrimaryPad();
    int messageParameter = (int)IDS_CONFIRM_OK;
    gConsoleUIController.RequestAlertMessage(titleId, messageId, &messageParameter, 1, buttonId, callback, callbackData,
                                             0, true);
}

bool ConsoleUIController::IsShowingErrorMessage(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->GetTopScene(EUILayer_Alert) != 0;
}

bool ConsoleUIController::IsShowingAlertMessage(int playerId)
{
    int groupId = EUIGroup_Fullscreen;
    if (gApp.GetGameStarted())
        groupId = GetGroup(playerId);
    return m_groups[groupId]->GetTopScene(EUILayer_Popup) != 0;
}

void ConsoleUIController::SetWinUserIndex(unsigned int userIndex)
{
    m_winUserIndex = userIndex;
}

unsigned int ConsoleUIController::GetWinUserIndex()
{
    return m_winUserIndex;
}

void ConsoleUIController::ShowUIDebugConsole(bool show)
{
    if (show)
        m_debugConsole = m_groups[EUIGroup_Fullscreen]->addComponent(0, EUIScene_DebugUIConsole, EUILayer_Debug);
    else
    {
        m_groups[EUIGroup_Fullscreen]->removeComponent(EUIScene_DebugUIConsole, EUILayer_Debug);
        m_debugConsole = 0;
    }
}

void ConsoleUIController::ShowUIDebugMarketingGuide(bool show)
{
}

bool ConsoleUIController::requestResolutionChange(int resolution)
{
    if (m_resolutionDisableFlags != 0)
        return false;
    if (!TryEnterCriticalSection(&g_uiCriticalSection))
        return false;
    gFui->setResolution(resolution);
    if (resolution == 0)
    {
        m_screenWidth = 1280.0f;
        m_screenHeight = 720.0f;
    }
    else
    {
        m_screenWidth = 1920.0f;
        m_screenHeight = 1080.0f;
    }
    m_groups[EUIGroup_Player1]->ReloadAllLayers();
    m_groups[EUIGroup_Player2]->ReloadAllLayers();
    m_groups[EUIGroup_Player3]->ReloadAllLayers();
    m_groups[EUIGroup_Player4]->ReloadAllLayers();
    m_groups[EUIGroup_Fullscreen]->ReloadAllLayers();
    LeaveCriticalSection(&g_uiCriticalSection);
    return true;
}

void ConsoleUIController::setResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag)
{
    m_resolutionDisableFlags |= 1u << (unsigned int)flag;
}

void ConsoleUIController::clearResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag)
{
    m_resolutionDisableFlags &= ~(1u << (unsigned int)flag);
}

int ConsoleUIController::reloadSkinThreadProc(void* context)
{
    ConsoleUIController* controller = static_cast<ConsoleUIController*>(context);
    EnterCriticalSection(&g_uiCriticalSection);
    controller->loadSkins();
    controller->m_groups[EUIGroup_Player1]->ReloadAll();
    controller->m_groups[EUIGroup_Player2]->ReloadAll();
    controller->m_groups[EUIGroup_Player3]->ReloadAll();
    controller->m_groups[EUIGroup_Player4]->ReloadAll();
    controller->m_groups[EUIGroup_Fullscreen]->ReloadAll();
    controller->m_groups[EUIGroup_DRC]->ReloadAll();
    controller->NavigateBack(0, false, EUIScene_None, EUILayer_Tooltips);
    LeaveCriticalSection(&g_uiCriticalSection);
    return 0;
}

void ConsoleUIController::ReloadSkin()
{
    m_groups[EUIGroup_DRC]->DestroyAll();
    m_groups[EUIGroup_Fullscreen]->DestroyAll();
    m_groups[EUIGroup_Player1]->DestroyAll();
    m_groups[EUIGroup_Player2]->DestroyAll();
    m_groups[EUIGroup_Player3]->DestroyAll();
    m_groups[EUIGroup_Player4]->DestroyAll();
    for (int i = 10; i >= 0; --i)
    {
        if (m_skinScenes[i] != 0)
        {
            gFui->unloadScene((fuiFile*)m_skinScenes[i]);
            m_skinScenes[i] = 0;
        }
    }
    m_skinThread = new C4JThreadImpl(reloadSkinThreadProc, this, "Reload skin thread", 0);
    m_skinThread->SetProcessor(0);
    gConsoleUIController.NavigateToScene(0, EUIScene_Timer, (void*)1, EUILayer_Tooltips, EUIGroup_Fullscreen);
}

void ConsoleUIController::StartReloadSkinThread()
{
    if (m_skinThread != 0)
        m_skinThread->Run();
}

bool ConsoleUIController::IsReloadingSkin()
{
    EnterCriticalSection(&m_skinThreadMutex);
    bool reloading = m_skinThread != 0 && (!m_skinThread->hasStarted() || m_skinThread->isRunning());
    LeaveCriticalSection(&m_skinThreadMutex);
    return reloading;
}

bool ConsoleUIController::IsExpectingOrReloadingSkin()
{
    bool expecting = Minecraft::GetInstance()->mTexturePackRepository->getSelected()->isLoadingData()
                     || Minecraft::GetInstance()->mTexturePackRepository->needsUIUpdate() || IsReloadingSkin()
                     || PendingFontChange();
    return expecting;
}

void ConsoleUIController::CleanUpSkinReload()
{
    EnterCriticalSection(&m_skinThreadMutex);
    delete m_skinThread;
    m_skinThread = 0;
    LeaveCriticalSection(&m_skinThreadMutex);

    if (Minecraft::GetInstance()->mTexturePackRepository->getSelected() != TexturePackRepository::defaultTexturePack)
        Minecraft::GetInstance()->mTexturePackRepository->getSelected()->hasAudio();

    EnterCriticalSection(&m_messageBoxMutex);
    for (std::vector<_QueuedMessageBoxData*>::iterator it = m_queuedMessageBoxes.begin();
         it != m_queuedMessageBoxes.end(); ++it)
    {
        _QueuedMessageBoxData* queued = *it;
        gConsoleUIController.NavigateToScene(queued->queuedUserIndex, EUIScene_MessageBox, queued, queued->layer,
                                             EUIGroup_Fullscreen);
        delete[] queued->info.messageParameters;
        delete queued;
    }
    m_queuedMessageBoxes.clear();
    LeaveCriticalSection(&m_messageBoxMutex);
}

void ConsoleUIController::tickInput()
{
    if (!m_inputSuspended)
    {
        handleInput();
        ++m_lastInputTime;
    }
}

void ConsoleUIController::handleKeyPress(unsigned int key, unsigned int character)
{
    bool touchPressed = false;
    bool touchReleased = false;
    bool repeatFired = false;
    if (character == 0x14)
    {
        bool newTouch = false;
        bool touchDataValid = false;
        int touchX = 0;
        int touchY = 0;
        if (m_touchHeld && key > 4 && g_pInput->ButtonPressed((int)key, 0x14, 0) != 0)
            m_touchHeld = false;
        unsigned short* touchData = g_pInput->GetTouchPadData((int)key, C_4JInput::ETouchType_0, true);
        if (touchData != 0 && touchData[2] == 1 && touchData[3] == 0)
        {
            touchX = touchData[0];
            touchY = touchData[1];
            touchDataValid = true;
        }
        if (!gConsoleUIController.IsReloadingSkin() && !m_rebuildingTouchBoxes && touchData != 0 && GetDRCEnabled())
        {
            if (touchDataValid)
            {
                if (!m_touchHeld)
                {
                    m_touchPressElement = 0;
                    m_touchCurrentElement = 0;
                    UIScene* touchScene = m_groups[EUIGroup_Fullscreen]->getCurrentScene((int)key);
                    UIComponent_Tooltips* touchTooltips = m_groups[EUIGroup_Fullscreen]->getTooltips();
                    if (touchScene == 0)
                    {
                        if (key < 4)
                        {
                            touchScene = m_groups[static_cast<EUIGroup>(key + EUIGroup_Player1)]->getCurrentScene(
                                (int)key);
                            touchTooltips = m_groups[2 + key]->getTooltips();
                        }
                        else
                        {
                            touchScene = m_groups[EUIGroup_Player1]->getCurrentScene((int)key);
                            touchTooltips = m_groups[EUIGroup_Player1]->getTooltips();
                        }
                    }
                    bool hitFound = false;
                    if (touchScene != 0)
                        hitFound = TouchBoxHit(touchScene, touchX, touchY)
                                   || TouchBoxHit(touchTooltips, touchX, touchY);
                    else if (touchTooltips != 0)
                        hitFound = TouchBoxHit(touchTooltips, touchX, touchY);
                    if (hitFound)
                    {
                        touchPressed = true;
                        m_touchHoldPad = (int)key;
                        newTouch = true;
                        m_touchHeld = true;
                        m_actionTimer[key][0x14] = GetTickCount() + 300;
                    }
                }
                else if (m_touchHoldPad == (int)key)
                {
                    UIScene* touchScene = m_groups[EUIGroup_Fullscreen]->getCurrentScene((int)key);
                    UIComponent_Tooltips* touchTooltips = m_groups[EUIGroup_Fullscreen]->getTooltips();
                    if (touchScene == 0)
                    {
                        if (key < 4)
                        {
                            touchScene = m_groups[static_cast<EUIGroup>(key + EUIGroup_Player1)]->getCurrentScene(
                                (int)key);
                            touchTooltips = m_groups[2 + key]->getTooltips();
                        }
                        else
                        {
                            touchScene = m_groups[EUIGroup_Player1]->getCurrentScene((int)key);
                            touchTooltips = m_groups[EUIGroup_Player1]->getTooltips();
                        }
                    }
                    bool hitFound = false;
                    if (touchScene != 0)
                        hitFound = TouchBoxHit(touchScene, touchX, touchY)
                                   || TouchBoxHit(touchTooltips, touchX, touchY);
                    else if (touchTooltips != 0)
                        hitFound = TouchBoxHit(touchTooltips, touchX, touchY);
                    if (hitFound)
                    {
                        newTouch = true;
                        unsigned int now = GetTickCount();
                        if (m_actionTimer[key][0x14] != 0 && (unsigned int)m_actionTimer[key][0x14] < now)
                        {
                            repeatFired = true;
                            touchPressed = true;
                            m_actionTimer[key][0x14] = now + 100;
                        }
                    }
                }
            }
            else if (m_touchHeld && m_touchHoldPad == (int)key)
            {
                newTouch = true;
                m_touchHeld = false;
                touchReleased = true;
                m_actionTimer[key][0x14] = 0;
            }
            HandleTouchInput(key, 0x14, touchPressed, repeatFired, touchReleased);
            if (newTouch)
                return;
        }
    }

    unsigned int actionByte = character & 0xff;
    bool buttonDown = g_pInput->ButtonDown((int)key, actionByte) != 0;
    bool buttonPressed = g_pInput->ButtonPressed((int)key, actionByte, 0) != 0;
    bool buttonReleased = g_pInput->ButtonReleased((int)key, actionByte) != 0;

    if (buttonPressed)
    {
        m_actionTimer[key][character] = GetTickCount() + 300;
    }
    else if (buttonReleased)
    {
        m_actionTimer[key][character] = 0;
    }
    else if (buttonDown)
    {
        unsigned int now = GetTickCount();
        if (m_actionTimer[key][character] != 0 && (unsigned int)m_actionTimer[key][character] < now)
        {
            m_actionTimer[key][character] = now + 100;
            repeatFired = true;
            buttonPressed = true;
        }
    }

    if (!repeatFired && !buttonPressed && !buttonReleased)
        return;

    bool handled = false;
    UIGroup* fullscreenGroup = m_groups[EUIGroup_Fullscreen];
    unsigned int toastShowing = fullscreenGroup->getPressStartToPlay()->GetAchievementToastShowing((int)key);
    if ((toastShowing != 0 || fullscreenGroup->getPressStartToPlay()->GetHoldToShowAchievementsActive(false))
        && !gConsoleUIController.IsSceneInStack((int)key, EUIScene_PauseMenu)
        && !m_groups[EUIGroup_Fullscreen]->getPressStartToPlay()->handleHold((int)key, (int)character, repeatFired,
                                                                             buttonPressed, buttonReleased, handled))
    {
        buttonPressed = true;
    }

    if (!handled)
    {
        m_groups[EUIGroup_Fullscreen]->handleInput((int)key, (int)character, repeatFired, buttonPressed, buttonReleased,
                                                   handled);
        if (!handled && key < 4)
            m_groups[2 + key]->handleInput((int)key, (int)character, repeatFired, buttonPressed, buttonReleased,
                                           handled);
    }
}

bool ConsoleUIController::handleInput()
{
    for (int pad = 0; pad < 4; ++pad)
    {
        if (g_pInput->IsPadConnected(pad) != 0 && g_pInput->IsPadLocked(pad) != 0)
        {
            for (unsigned int action = 0; action <= 0x17; ++action)
                handleKeyPress((unsigned int)pad, action);
        }
    }
    for (int pad = 4; pad < 9; ++pad)
    {
        if (g_pInput->IsPadConnected(pad) != 0 && g_pInput->IsPadDRCController(pad) != 0)
        {
            if (m_groups[EUIGroup_DRC]->getDRC() != 0)
                m_groups[EUIGroup_DRC]->getDRC()->handleDRCTouchInput((unsigned int)pad);
        }
        if (!gApp.GetGameStarted() || gApp.m_signInResult != 0)
        {
            if (g_pInput->IsPadLocked(pad) == 0 && g_pInput->IsPadConnected(pad) != 0)
            {
                for (unsigned int action = 0; action <= 0x17; ++action)
                    handleKeyPress((unsigned int)pad, action);
            }
        }
    }
    return false;
}

void ConsoleUIController::tick()
{
    SetupFont();
    if ((m_homeMenuPending || m_cleanupOnReload) && !gConsoleUIController.IsReloadingSkin())
    {
        gConsoleUIController.CleanUpSkinReload();
        if (!m_homeMenuPending && CGameNetworkManager::sInstance.IsInSession())
        {
            if (m_autoCleanupOnReload)
                gConsoleUIController.CloseAllPlayersScenes(false);
        }
        else if (g_introRunning != 0)
        {
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_MainMenu, 0, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
        }
        updateCurrentFont();
        m_autoCleanupOnReload = true;
        m_homeMenuPending = false;
        m_cleanupOnReload = false;
    }
    for (int i = 0; i < 6; ++i)
    {
        if (m_playerFlags[i])
        {
            m_groups[i]->closeAllScenes();
            m_groups[i]->ClearTooltips();
            m_playerFlags[i] = false;
        }
    }
    if (m_lastInputTime == 0)
        tickInput();
    m_lastInputTime = 0;
    gFui->dispatchEnterFrameEvent();
    for (int i = 0; i < 6; ++i)
        m_groups[i]->tick();
    if (!gConsoleUIController.IsReloadingSkin())
    {
        const long long now = System::processTimeInMilliSecs();
        for (boost::unordered_map<std::wstring, _CachedMovieData>::iterator it = m_movieCache.begin();
             it != m_movieCache.end();)
        {
            if (it->second.expiryTime < now)
            {
                delete[] it->second.data.data;
                it = m_movieCache.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    gFui->tick();
}

void ConsoleUIController::SendTouchInput(unsigned int playerId, unsigned int action, bool pressed, bool repeat,
                                         bool released)
{
    bool handled = false;
    m_groups[EUIGroup_Fullscreen]->handleInput((int)playerId, (int)action, repeat, pressed, released, handled);
    if (!handled && playerId < 4)
        m_groups[EUIGroup_Player1 + playerId]->handleInput((int)playerId, (int)action, repeat, pressed, released,
                                                           handled);
}

#ifdef MINECRAFT_PC
void ConsoleUIController::PcHoverPointer(unsigned int playerId, int x, int y)
{
    static UIControl* s_hovered = 0;
    if (m_touchHeld || m_rebuildingTouchBoxes || IsReloadingSkin() || !GetDRCEnabled())
        return;
    UIScene* scene = m_groups[EUIGroup_Fullscreen]->getCurrentScene((int)playerId);
    if (scene == 0)
        scene = m_groups[static_cast<EUIGroup>(playerId < 4 ? playerId + EUIGroup_Player1 : EUIGroup_Player1)]
                    ->getCurrentScene((int)playerId);
    if (scene == 0 || !TouchBoxHit(scene, x, y) || m_touchCurrentElement == 0)
    {
        s_hovered = 0;
        return;
    }
    UIControl* control = m_touchCurrentElement->control;
    switch (control->m_controlType)
    {
    case UIControl::eUIControlType_Button:
    case UIControl::eUIControlType_CheckBox:
    case UIControl::eUIControlType_Slider:
    case UIControl::eUIControlType_TextInput:
    case UIControl::eUIControlType_TexturePackList:
        if (control != s_hovered)
            control->m_scene->SetFocusToElement(control->m_controlId);
        break;
    case UIControl::eUIControlType_ButtonList:
    case UIControl::eUIControlType_CheckboxButtonList:
    case UIControl::eUIControlType_LayerIconButtonList:
    case UIControl::eUIControlType_LeftIconButtonList:
    case UIControl::eUIControlType_MultiList:
    {
        UIControl* panel = control->m_scene->GetMainPanel();
        int listY = panel == 0 ? y : y - (int)panel->getYPos();
        static_cast<UIControl_List*>(control)->PcHoverItem(listY);
        break;
    }
    default:
        break;
    }
    s_hovered = control;
}
#endif

void ConsoleUIController::HandleTouchInput(unsigned int playerId, unsigned int action, bool pressed, bool repeat,
                                           bool released)
{
    if (m_rebuildingTouchBoxes)
        return;

    if (!pressed && !repeat && !released)
    {
        if (!m_touchHeld || m_touchPressElement == 0)
            return;
        UIControl::eUIControlType type = m_touchPressElement->control->m_controlType;
        if (type != UIControl::eUIControlType_Slider && type != UIControl::eUIControlType_ButtonList
            && type != UIControl::eUIControlType_LeftIconButtonList
            && type != UIControl::eUIControlType_LayerIconButtonList
            && type != UIControl::eUIControlType_CheckboxButtonList && type != UIControl::eUIControlType_MultiList
            && type != UIControl::eUIControlType_TexturePackList && type != UIControl::eUIControlType_DynamicLabel
            && type != UIControl::eUIControlType_HTMLLabel && type != UIControl::eUIControlType_LeaderboardList
            && type != UIControl::eUIControlType_AchievementsList && type != UIControl::eUIControlType_Touch)
            return;
        repeat = true;
    }

    unsigned short* touch = g_pInput->GetTouchPadData(playerId, C_4JInput::ETouchType_0, true);
    int x = touch[0];
    int y = touch[1];
    if (released)
    {
        x = m_touchX;
        y = m_touchY;
    }
    else
    {
        m_touchX = x;
        m_touchY = y;
    }

    if (!repeat)
    {
        if (pressed && !released)
        {
            UIELEMENT* element = m_touchPressElement;
            UIControl* control = element->control;
            switch (control->m_controlType)
            {
            case UIControl::eUIControlType_Button:
            case UIControl::eUIControlType_CheckBox:
            case UIControl::eUIControlType_TextInput:
                control->m_scene->SetFocusToElement(control->m_controlId);
                return;
            case UIControl::eUIControlType_DynamicLabel:
                static_cast<UIControl_DynamicLabel*>(control)->TouchScroll(y, true);
                return;
            case UIControl::eUIControlType_HTMLLabel:
                static_cast<UIControl_HTMLLabel*>(control)->TouchScroll(y, true);
                return;
            case UIControl::eUIControlType_LeaderboardList:
                static_cast<UIControl_LeaderboardList*>(control)->SetTouchFocus(x, y, false);
                return;
            case UIControl::eUIControlType_Slider:
                control->m_scene->SetFocusToElement(control->m_controlId);
                SendTouchInput(playerId, action, pressed, repeat, false);
                return;
            case UIControl::eUIControlType_TexturePackList:
                control->m_scene->SetFocusToElement(control->m_controlId);
                static_cast<UIControl_TexturePackList*>(control)->SetTouchFocus(x - m_touchPressElement->left,
                                                                                y - m_touchPressElement->top, false);
                return;
            case UIControl::eUIControlType_Touch:
                control->m_scene->handleTouchInput(playerId, x, y, control->m_controlId, pressed, false, false);
                return;
            case UIControl::eUIControlType_ButtonList:
            case UIControl::eUIControlType_CheckboxButtonList:
            case UIControl::eUIControlType_LayerIconButtonList:
            case UIControl::eUIControlType_LeftIconButtonList:
            case UIControl::eUIControlType_MultiList:
            {
                float panelX;
                UIControl* panel = control->m_scene->GetMainPanel();
                if (panel == 0)
                {
                    panelX = control->getXPos();
                }
                else
                {
                    panelX = control->m_scene->GetMainPanel()->getXPos();
                    y -= (int)control->m_scene->GetMainPanel()->getYPos();
                }
                UIControl_List* list = static_cast<UIControl_List*>(control);
                if (m_touchPressElement->control->m_controlType != UIControl::eUIControlType_MultiList
                    || list->CanTouchFocus(x - (int)panelX, y))
                {
                    list->SetTouchFocus(x - (int)panelX, y, false);
#ifdef MINECRAFT_PC
                    list->PcPressSlider(x - (int)panelX, y);
#endif
                }
                return;
            }
            case UIControl::eUIControlType_AchievementsList:
            {
                UIControl* panel = control->m_scene->GetMainPanel();
                if (panel == 0)
                {
                    x -= (int)control->getXPos();
                }
                else
                {
                    x -= (int)control->m_scene->GetMainPanel()->getXPos();
                    y -= (int)control->m_scene->GetMainPanel()->getYPos();
                }
                UIControl_GridList* list = static_cast<UIControl_GridList*>(control);
                if (list->CanTouchFocus(x, y))
                    list->SetTouchFocus(x, y, false);
                return;
            }
            default:
                SendTouchInput(playerId, action, pressed, repeat, released);
                return;
            }
        }
    }
    else
    {
        UIELEMENT* element = m_touchPressElement;
        UIControl* control = element->control;
        switch (control->m_controlType)
        {
        case UIControl::eUIControlType_CheckBox:
        case UIControl::eUIControlType_TextInput:
            repeat = false;
            pressed = false;
            break;
        case UIControl::eUIControlType_DynamicLabel:
            static_cast<UIControl_DynamicLabel*>(control)->TouchScroll(y, true);
            pressed = false;
            repeat = false;
            break;
        case UIControl::eUIControlType_HTMLLabel:
            static_cast<UIControl_HTMLLabel*>(control)->TouchScroll(y, true);
            pressed = false;
            repeat = false;
            break;
        case UIControl::eUIControlType_LeaderboardList:
            static_cast<UIControl_LeaderboardList*>(control)->SetTouchFocus(x, y, true);
            break;
        case UIControl::eUIControlType_Slider:
        {
            UIControl_Slider* slider = static_cast<UIControl_Slider*>(control);
            slider->SetSliderTouchPos(((float)x - element->left) / slider->GetRealWidth());
            break;
        }
        case UIControl::eUIControlType_TexturePackList:
            static_cast<UIControl_TexturePackList*>(control)->SetTouchFocus(x - element->left, y - element->top, true);
            break;
        case UIControl::eUIControlType_Touch:
            pressed = false;
            control->m_scene->handleTouchInput(playerId, x, y, control->m_controlId, pressed, repeat, released);
            repeat = false;
            break;
        case UIControl::eUIControlType_ButtonList:
        case UIControl::eUIControlType_CheckboxButtonList:
        case UIControl::eUIControlType_LayerIconButtonList:
        case UIControl::eUIControlType_LeftIconButtonList:
        case UIControl::eUIControlType_MultiList:
        {
            float panelX;
            UIControl* panel = control->m_scene->GetMainPanel();
            if (panel == 0)
            {
                panelX = control->getXPos();
            }
            else
            {
                panelX = control->m_scene->GetMainPanel()->getXPos();
                y -= (int)control->m_scene->GetMainPanel()->getYPos();
            }
            x -= (int)panelX;
            UIControl_List* list = static_cast<UIControl_List*>(control);
            if (m_touchPressElement->control->m_controlType != UIControl::eUIControlType_MultiList
                || list->CanTouchFocus(x, y))
                list->SetTouchFocus(x, y, true);
            break;
        }
        case UIControl::eUIControlType_AchievementsList:
        {
            UIControl* panel = control->m_scene->GetMainPanel();
            if (panel == 0)
            {
                x -= (int)control->getXPos();
            }
            else
            {
                x -= (int)control->m_scene->GetMainPanel()->getXPos();
                y -= (int)control->m_scene->GetMainPanel()->getYPos();
            }
            UIControl_GridList* list = static_cast<UIControl_GridList*>(control);
            if (list->CanTouchFocus(x, y))
                list->SetTouchFocus(x, y, true);
            break;
        }
        default:
            break;
        }
    }

    if (released)
    {
        std::string controlName = m_touchPressElement->control->m_name;
        UIELEMENT* element = m_touchPressElement;
        UIControl* control = element->control;
        switch (control->m_controlType)
        {
        case UIControl::eUIControlType_Button:
        case UIControl::eUIControlType_TextInput:
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control)
                pressed = true;
            break;
        case UIControl::eUIControlType_CheckBox:
        {
            UIControl_CheckBox* checkBox = static_cast<UIControl_CheckBox*>(control);
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control && checkBox->IsEnabled())
                checkBox->setChecked(!checkBox->IsChecked());
            released = false;
            break;
        }
        case UIControl::eUIControlType_DynamicLabel:
            static_cast<UIControl_DynamicLabel*>(control)->TouchScroll(y, false);
            break;
        case UIControl::eUIControlType_HTMLLabel:
            static_cast<UIControl_HTMLLabel*>(control)->TouchScroll(y, false);
            break;
        case UIControl::eUIControlType_TexturePackList:
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control
                && static_cast<UIControl_TexturePackList*>(control)->CanTouchTrigger(x - element->left,
                                                                                     y - element->top))
                pressed = true;
            break;
        case UIControl::eUIControlType_Touch:
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control)
                control->m_scene->handleTouchInput(playerId, x, y, control->m_controlId, pressed, repeat, released);
            released = false;
            break;
        case UIControl::eUIControlType_ButtonList:
        case UIControl::eUIControlType_CheckboxButtonList:
        case UIControl::eUIControlType_LayerIconButtonList:
        case UIControl::eUIControlType_LeftIconButtonList:
        case UIControl::eUIControlType_MultiList:
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control)
            {
                UIControl_List* list = static_cast<UIControl_List*>(control);
                bool triggered;
                if (control->m_scene->GetMainPanel() == 0)
                {
                    triggered = list->CanTouchTrigger(x - (int)control->getXPos(), y);
                }
                else
                {
                    float panelX = control->m_scene->GetMainPanel()->getXPos();
                    float panelY = control->m_scene->GetMainPanel()->getYPos();
                    triggered = list->CanTouchTrigger(x - (int)panelX, y - (int)panelY);
                }
                if (triggered)
                    pressed = true;
            }
            break;
        case UIControl::eUIControlType_AchievementsList:
            if (m_touchCurrentElement != 0 && control == m_touchCurrentElement->control)
            {
                UIControl_GridList* list = static_cast<UIControl_GridList*>(control);
                bool triggered;
                if (control->m_scene->GetMainPanel() != 0)
                {
                    float panelX = control->m_scene->GetMainPanel()->getXPos();
                    float panelY = control->m_scene->GetMainPanel()->getYPos();
                    triggered = list->CanTouchTrigger(x - (int)panelX, y - (int)panelY);
                }
                else
                {
                    triggered = list->CanTouchTrigger(x - (int)control->getXPos(), y);
                }
                if (triggered)
                    pressed = true;
            }
            break;
        default:
            break;
        }
    }

    if (pressed || repeat || released)
        SendTouchInput(playerId, action, pressed, repeat, released);
}

void ConsoleUIController::TouchBoxAdd(UIControl* control, UIScene* scene)
{
    EUILayer layer = (EUILayer)scene->m_layer->m_layerId;
    EUIGroup group = scene->m_layer->m_group->m_group;
    EUIScene sceneType = (EUIScene)scene->getSceneType();
    TouchBoxAdd(control, group, layer, sceneType, scene->GetMainPanel());
}

void ConsoleUIController::TouchBoxAdd(UIControl* control, EUIGroup groupId, EUILayer layer, EUIScene scene,
                                      UIControl* parent)
{
    UIELEMENT* element = new UIELEMENT;
    element->control = control;
    float width = control->getWidth();
    float height = control->getHeight();
    float parentX = 0.0f;
    float parentY = 0.0f;
    if (parent != 0)
    {
        parentX = parent->getXPos();
        parentY = parent->getYPos();
    }
    if (control->m_controlType == UIControl::eUIControlType_Slider)
    {
        width = static_cast<UIControl_Slider*>(control)->GetRealWidth();
    }
    else if (control->m_controlType == UIControl::eUIControlType_TexturePackList)
    {
        height = static_cast<UIControl_TexturePackList*>(control)->GetRealHeight();
    }
    else if (control->m_controlType == UIControl::eUIControlType_DynamicLabel)
    {
        width = static_cast<UIControl_DynamicLabel*>(control)->GetRealWidth();
        height = static_cast<UIControl_DynamicLabel*>(control)->GetRealHeight();
    }
    else if (control->m_controlType == UIControl::eUIControlType_HTMLLabel)
    {
        width = static_cast<UIControl_HTMLLabel*>(control)->GetRealWidth();
        height = static_cast<UIControl_HTMLLabel*>(control)->GetRealHeight();
    }
    control->m_scene->updateViewportTouchOffset();
    element->left = (int)(control->getXPos() + parentX + m_touchOffsetX);
    element->top = (int)(control->getYPos() + parentY + m_touchOffsetY);
    element->right = (int)(control->getXPos() + width + parentX + m_touchOffsetX);
    element->bottom = (int)(control->getYPos() + height + parentY + m_touchOffsetY);
    if (element->left != element->right && element->top != element->bottom)
    {
        EnterCriticalSection(&m_touchMutex);
        m_touchBoxes[groupId][layer][scene].push_back(element);
        LeaveCriticalSection(&m_touchMutex);
    }
}

void ConsoleUIController::PlayUISFX(const SoundEvent* soundEvent)
{
    unsigned long long now = System::processTimeInMilliSecs();
    if (!(now - m_lastUISFXTime < 10))
    {
        m_lastUISFXTime = now;
        if (soundEvent == SoundEvents::UI_PRESS)
            Minecraft::GetInstance()->mSoundEngine->playUI(soundEvent, 0.5f, 1.0f);
        else
            Minecraft::GetInstance()->mSoundEngine->playUI(soundEvent, 1.0f, 1.0f);
    }
}

bool ConsoleUIController::TouchBoxHit(UIScene* scene, int x, int y)
{
    EUIGroup group = scene->m_layer->m_group->m_group;
    int layer = scene->m_layer->m_layerId;
    EUIScene sceneType = (EUIScene)scene->getSceneType();
    if (x == 12 && y == 708)
    {
        x = m_touchX;
        y = m_touchY;
    }
    else
    {
        m_touchX = x;
        m_touchY = y;
    }
    EnterCriticalSection(&m_touchMutex);
    if (m_touchBoxes[group][layer][sceneType].size() > 0)
    {
        std::vector<UIELEMENT*>::iterator itEnd = m_touchBoxes[group][layer][sceneType].end();
        for (std::vector<UIELEMENT*>::iterator it = m_touchBoxes[group][layer][sceneType].begin(); it != itEnd; ++it)
        {
            UIELEMENT* element = *it;
            if (element != 0 && element->control != 0 && !element->control->m_removed && element->control->getVisible()
                && x >= element->left && x <= element->right && y >= element->top && y <= element->bottom)
            {
                if (!m_touchHeld)
                    m_touchPressElement = element;
                m_touchCurrentElement = element;
                LeaveCriticalSection(&m_touchMutex);
                return true;
            }
        }
    }
    LeaveCriticalSection(&m_touchMutex);
    m_touchCurrentElement = 0;
    return false;
}

void ConsoleUIController::TouchBoxesClear(UIScene* scene)
{
    EnterCriticalSection(&m_touchMutex);
    m_touchHeld = false;
    EUIGroup group = scene->m_layer->m_group->m_group;
    int layer = scene->m_layer->m_layerId;
    std::vector<UIELEMENT*>& touchBoxes = m_touchBoxes[group][layer][scene->getSceneType()];
    std::vector<UIELEMENT*>::iterator itEnd = touchBoxes.end();
    for (std::vector<UIELEMENT*>::iterator it = touchBoxes.begin(); it != itEnd; ++it)
        delete *it;
    touchBoxes.clear();
    LeaveCriticalSection(&m_touchMutex);
}

void ConsoleUIController::TouchBoxRebuild(UIScene* scene)
{
    EUIGroup group = scene->m_layer->m_group->m_group;
    int layer = scene->m_layer->m_layerId;
    EUIScene sceneType = (EUIScene)scene->getSceneType();
    bool touchHeld = false;
    if (m_touchHeld && m_touchPressElement != 0 && m_touchPressElement->control->m_scene->m_layer->m_layerId != layer)
        touchHeld = m_touchHeld;
    m_rebuildingTouchBoxes = true;
    m_touchHeld = false;
    gConsoleUIController.TouchBoxesClear(scene);
    std::vector<UIControl*>::iterator itEnd = scene->m_controls.end();
    for (std::vector<UIControl*>::iterator it = scene->m_controls.begin(); it != itEnd; ++it)
    {
        UIControl* control = *it;
        switch (control->m_controlType)
        {
        case UIControl::eUIControlType_Button:
        case UIControl::eUIControlType_CheckBox:
        case UIControl::eUIControlType_DynamicLabel:
        case UIControl::eUIControlType_HTMLLabel:
        case UIControl::eUIControlType_LeaderboardList:
        case UIControl::eUIControlType_Slider:
        case UIControl::eUIControlType_TextInput:
        case UIControl::eUIControlType_TexturePackList:
        case UIControl::eUIControlType_Touch:
        case UIControl::eUIControlType_ButtonList:
        case UIControl::eUIControlType_CheckboxButtonList:
        case UIControl::eUIControlType_LayerIconButtonList:
        case UIControl::eUIControlType_LeftIconButtonList:
        case UIControl::eUIControlType_MultiList:
        case UIControl::eUIControlType_AchievementsList:
            if (control->getVisible())
            {
                if (control->m_controlType == UIControl::eUIControlType_MultiList)
                {
                    (*it)->UpdateControl();
                    gConsoleUIController.TouchBoxAdd(control, group, (EUILayer)layer, sceneType, scene->GetMainPanel());
                }
                else
                {
                    control->UpdateControl();
                    gConsoleUIController.TouchBoxAdd(control, group, (EUILayer)layer, sceneType, scene->GetMainPanel());
                }
            }
            break;
        default:
            break;
        }
    }
    m_rebuildingTouchBoxes = false;
    m_touchHeld = touchHeld;
}

void ConsoleUIController::NavigateToHomeMenu(bool immediate)
{
    gConsoleUIController.CloseAllPlayersScenes(false);
    gApp.SetLiveLinkRequired(false);
    Minecraft* minecraft = Minecraft::GetInstance();
    gApp.setIgnoreMainMenuInputDuringInvite(false);
    TexturePack* previousPack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    previousPack->hasAudio();
    minecraft->mTexturePackRepository->selectTexturePackById(0, false);
    if (!immediate)
    {
        minecraft->mSoundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f,
                                                    0x20, 0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
        minecraft->mSoundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                                               ConsoleSoundEngine::eAudioDelayType_Random);
    }
    previousPack->hasAudio();
    gStorage->UnmountInstalledDLC("TPACK");
    CGameNetworkManager::sInstance.ForceFriendsSessionRefresh();
    if (minecraft->mTexturePackRepository->needsUIUpdate())
    {
        m_homeMenuPending = true;
        return;
    }
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_MainMenu, 0, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

UIScene* ConsoleUIController::FindScene(EUIScene scene)
{
    for (int i = 0; i < 5; ++i)
    {
        UIScene* found = m_groups[i]->FindScene(scene);
        if (found != 0)
            return found;
    }
    return m_groups[5]->FindScene(scene);
}
