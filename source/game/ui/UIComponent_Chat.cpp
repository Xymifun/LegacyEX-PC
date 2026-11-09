#include "game/ui/UIComponent_Chat.hpp"

#include <string>

#include "game/client/render/C4JRender.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"

UIComponent_Chat::UIComponent_Chat(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    for (int i = 0; i < 10; ++i)
    {
        m_labels[i].init(UIString(L""));
    }
    m_chatLabel.init(UIString(L""));
    addTimer(0, 100);
}

unsigned int UIComponent_Chat::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_labels[0].setupControl(this, currentNode, std::string("Label1"));
        m_controls.push_back(&m_labels[0]);
    }
    {
        m_labels[1].setupControl(this, currentNode, std::string("Label2"));
        m_controls.push_back(&m_labels[1]);
    }
    {
        m_labels[2].setupControl(this, currentNode, std::string("Label3"));
        m_controls.push_back(&m_labels[2]);
    }
    {
        m_labels[3].setupControl(this, currentNode, std::string("Label4"));
        m_controls.push_back(&m_labels[3]);
    }
    {
        m_labels[4].setupControl(this, currentNode, std::string("Label5"));
        m_controls.push_back(&m_labels[4]);
    }
    {
        m_labels[5].setupControl(this, currentNode, std::string("Label6"));
        m_controls.push_back(&m_labels[5]);
    }
    {
        m_labels[6].setupControl(this, currentNode, std::string("Label7"));
        m_controls.push_back(&m_labels[6]);
    }
    {
        m_labels[7].setupControl(this, currentNode, std::string("Label8"));
        m_controls.push_back(&m_labels[7]);
    }
    {
        m_labels[8].setupControl(this, currentNode, std::string("Label9"));
        m_controls.push_back(&m_labels[8]);
    }
    {
        m_labels[9].setupControl(this, currentNode, std::string("Label10"));
        m_controls.push_back(&m_labels[9]);
    }
    {
        m_backgrounds[0].setupControl(this, currentNode, std::string("Label1Background"));
        m_controls.push_back(&m_backgrounds[0]);
    }
    {
        m_backgrounds[1].setupControl(this, currentNode, std::string("Label2Background"));
        m_controls.push_back(&m_backgrounds[1]);
    }
    {
        m_backgrounds[2].setupControl(this, currentNode, std::string("Label3Background"));
        m_controls.push_back(&m_backgrounds[2]);
    }
    {
        m_backgrounds[3].setupControl(this, currentNode, std::string("Label4Background"));
        m_controls.push_back(&m_backgrounds[3]);
    }
    {
        m_backgrounds[4].setupControl(this, currentNode, std::string("Label5Background"));
        m_controls.push_back(&m_backgrounds[4]);
    }
    {
        m_backgrounds[5].setupControl(this, currentNode, std::string("Label6Background"));
        m_controls.push_back(&m_backgrounds[5]);
    }
    {
        m_backgrounds[6].setupControl(this, currentNode, std::string("Label7Background"));
        m_controls.push_back(&m_backgrounds[6]);
    }
    {
        m_backgrounds[7].setupControl(this, currentNode, std::string("Label8Background"));
        m_controls.push_back(&m_backgrounds[7]);
    }
    {
        m_backgrounds[8].setupControl(this, currentNode, std::string("Label9Background"));
        m_controls.push_back(&m_backgrounds[8]);
    }
    {
        m_backgrounds[9].setupControl(this, currentNode, std::string("Label10Background"));
        m_controls.push_back(&m_backgrounds[9]);
    }
    {
        m_chatLabel.setupControl(this, currentNode, std::string("Jukebox"));
        m_controls.push_back(&m_chatLabel);
    }
    return 1;
}

std::wstring UIComponent_Chat::getMoviePath()
{
    C4JRender::eViewportType viewport = m_layer->getViewport();
    switch (viewport)
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
        return L"ComponentChatSplit";
    default:
        m_splitScreen = false;
        return L"ComponentChat";
    }
}

void UIComponent_Chat::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!m_splitScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    switch (viewport)
    {
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_BottomLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f));
        break;
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f), 0);
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f),
                                                 static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f));
        break;
    default:
        gConsoleUIController.setupRenderPosition(0, 0);
        break;
    }

    int top = 0;
    switch (viewport)
    {
    case C4JRender::eViewportType_FullScreen:
        break;
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
        gFui->render(m_movie, 0.0f, m_stageHeight / 2, static_cast<int>(gConsoleUIController.getScreenWidth()), height);
        return;
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
        gFui->render(m_movie, 0.0f, 0, width, static_cast<int>(gConsoleUIController.getScreenHeight()));
        return;
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        top = m_stageHeight / 2;
        break;
    default:
        gFui->render(m_movie, 0.0f, 0, width, height);
        return;
    }
    gFui->render(m_movie, 0.0f, top, width, height);
}

void UIComponent_Chat::handleTimerComplete(int timerId)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId].get() != NULL)
    {
        Gui* gui = minecraft->mGui;
        bool visible = false;
        for (unsigned int i = 0; i < 10; ++i)
        {
            float messageOpacity = gui->getOpacity(m_playerId, i);
#ifdef MINECRAFT_PC
            if (!m_splitScreen)
                messageOpacity = 0.0f;
#endif
            if (messageOpacity > 0.0f)
            {
                m_backgrounds[i].setOpacity(messageOpacity);
                m_labels[i].setOpacity(messageOpacity);
                std::wstring message = gui->mMessages[m_playerId].at(i).message;
                m_labels[i].setLabel(UIString(message), false, false);
                visible = true;
            }
            else
            {
                m_backgrounds[i].setOpacity(0.0f);
                m_labels[i].setOpacity(0.0f);
                m_labels[i].setLabel(UIString(L""), false, false);
            }
        }
        if (gui->getJukeboxOpacity(m_playerId) > 0.0f)
            visible = true;
        m_chatLabel.setOpacity(gui->getJukeboxOpacity(m_playerId));
        {
            std::wstring nowPlaying = gui->mNowPlaying[m_playerId];
            m_chatLabel.setLabel(UIString(nowPlaying), false, false);
        }
        setVisible(visible);
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            m_backgrounds[i].setOpacity(0.0f);
            m_labels[i].setOpacity(0.0f);
            m_labels[i].setLabel(UIString(L""), false, false);
        }
        m_chatLabel.setOpacity(0.0f);
        setVisible(false);
    }
}

int UIComponent_Chat::getSceneType()
{
    return EUIScene_Chat;
}

bool UIComponent_Chat::stealsFocus()
{
    return false;
}

bool UIComponent_Chat::hasFocus(int sceneType)
{
    return false;
}

bool UIComponent_Chat::hidesLowerScenes()
{
    return false;
}
