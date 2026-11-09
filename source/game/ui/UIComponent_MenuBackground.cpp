#include "game/ui/UIComponent_MenuBackground.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#ifdef MINECRAFT_PC
#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/pc/PcDisplay.hpp"
#endif

UIComponent_MenuBackground::UIComponent_MenuBackground(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer),
      m_splitScreen(false)
{
    initialiseMovie();
    m_movieLoaded = true;
}

std::wstring UIComponent_MenuBackground::getMoviePath()
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
        break;
    default:
        m_splitScreen = false;
        break;
    }
    return L"MenuBackground";
}

void UIComponent_MenuBackground::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (m_splitScreen)
    {
        int xPos = 0;
        int yPos = 0;
        switch (viewport)
        {
        case C4JRender::eViewportType_BottomHalf:
        case C4JRender::eViewportType_BottomLeftQuarter:
            yPos = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        case C4JRender::eViewportType_RightHalf:
        case C4JRender::eViewportType_TopRightQuarter:
            xPos = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            break;
        case C4JRender::eViewportType_BottomRightQuarter:
            xPos = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            yPos = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        default:
            break;
        }
        gConsoleUIController.setupRenderPosition(xPos, yPos);

        int movieY = 0;
        int movieWidth = width;
        int movieHeight = height;
        switch (viewport)
        {
        case C4JRender::eViewportType_LeftHalf:
        case C4JRender::eViewportType_RightHalf:
            movieHeight = static_cast<int>(gConsoleUIController.getScreenHeight());
            break;
        case C4JRender::eViewportType_TopHalf:
        case C4JRender::eViewportType_BottomHalf:
            movieWidth = static_cast<int>(gConsoleUIController.getScreenWidth());
            movieY = m_stageHeight / 2;
            break;
        case C4JRender::eViewportType_TopLeftQuarter:
        case C4JRender::eViewportType_TopRightQuarter:
        case C4JRender::eViewportType_BottomLeftQuarter:
        case C4JRender::eViewportType_BottomRightQuarter:
            movieY = m_stageHeight / 2;
            break;
        default:
            break;
        }
#ifdef MINECRAFT_PC
        PcSetWideWorldPass(true);
#endif
        gFui->render(m_movie, 0.0f, movieY, movieWidth, movieHeight);
#ifdef MINECRAFT_PC
        PcSetWideWorldPass(false);
#endif
    }
    else
    {
#ifdef MINECRAFT_PC
        PcSetWideWorldPass(true);
#endif
        UIScene::render(width, height, viewport);
#ifdef MINECRAFT_PC
        PcSetWideWorldPass(false);
#endif
    }
}

int UIComponent_MenuBackground::getSceneType()
{
    return EUIScene_MenuBackground;
}

bool UIComponent_MenuBackground::stealsFocus()
{
    return false;
}

bool UIComponent_MenuBackground::hasFocus(int)
{
    return false;
}

bool UIComponent_MenuBackground::hidesLowerScenes()
{
    return false;
}
