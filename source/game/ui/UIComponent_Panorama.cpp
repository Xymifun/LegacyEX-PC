#include "game/ui/UIComponent_Panorama.hpp"

#include <string>

#include "game/client/render/C4JRender.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_Panorama.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#ifdef MINECRAFT_PC
#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/pc/PcDisplay.hpp"
#endif

UIComponent_Panorama::UIComponent_Panorama(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_panoramaDay = true;
    m_movieLoaded = true;
    m_currentFrame = 0;
    while (!m_hasTicked)
        tick();
}

void UIComponent_Panorama::setPanorama(bool day)
{
    if (day != m_panoramaDay)
    {
        m_panoramaDay = day;
        fuiRenderNode* root = m_movie->getRootNode();
        static_cast<FJ_Panorama*>(root->getFuiNode())->ShowPanoramaDay(day);
    }
}

void UIComponent_Panorama::tick()
{
    if (!hasMovie())
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    EnterCriticalSection(&minecraft->mLevelLock);
    if (minecraft->mLevel != NULL && minecraft->mLevel->mDimension->getType() == DimensionType::OVERWORLD)
    {
        long long dayTime = minecraft->mLevel->getLevelData()->getDayTime();
        if (dayTime % 24000 > 14000)
        {
            setPanorama(false);
            LeaveCriticalSection(&minecraft->mLevelLock);
            m_currentFrame = static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->GetCurrentFrame();
            UIScene::tick();
            return;
        }
    }
    setPanorama(true);
    LeaveCriticalSection(&minecraft->mLevelLock);
    m_currentFrame = static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->GetCurrentFrame();
    UIScene::tick();
}

std::wstring UIComponent_Panorama::getMoviePath()
{
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        m_splitScreen = true;
        return L"PanoramaSplit";
    default:
        m_splitScreen = false;
        return L"Panorama";
    }
}

void UIComponent_Panorama::handleReload()
{
    static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->SetCurrentFrame(m_currentFrame);
}

void UIComponent_Panorama::render(int width, int height, C4JRender::eViewportType viewport)
{
    bool isHalf = viewport >= C4JRender::eViewportType_TopHalf && viewport <= C4JRender::eViewportType_RightHalf;
    if (m_splitScreen && isHalf)
    {
        int x = 0;
        int y = 0;
        switch (viewport)
        {
        case C4JRender::eViewportType_BottomHalf:
            y = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        case C4JRender::eViewportType_RightHalf:
            x = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            break;
        }
        gConsoleUIController.setupRenderPosition(x, y);

        if (viewport == C4JRender::eViewportType_LeftHalf || viewport == C4JRender::eViewportType_RightHalf)
        {
            int screenHeight = static_cast<int>(gConsoleUIController.getScreenHeight());
            gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(screenHeight));
        }
        else
        {
            gFui->render(m_movie, 0.0f, 0.0f, gConsoleUIController.getScreenWidth(),
                         gConsoleUIController.getScreenHeight(), 0.5f, 0.5f);
        }
    }
    else
    {
#ifdef MINECRAFT_PC
        const bool cover = viewport == C4JRender::eViewportType_FullScreen && !gApp.GetGameStarted();
        PcSetCoverUiPass(cover);
#endif
        UIScene::render(width, height, viewport);
#ifdef MINECRAFT_PC
        PcSetCoverUiPass(false);
#endif
    }
}

int UIComponent_Panorama::getSceneType()
{
    return EUIScene_Panorama;
}

bool UIComponent_Panorama::stealsFocus()
{
    return false;
}

bool UIComponent_Panorama::hasFocus(int)
{
    return false;
}
