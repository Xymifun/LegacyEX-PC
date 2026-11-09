#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"

extern CConsoleMinecraftApp gApp;
extern ConsoleUIController gConsoleUIController;

UIComponent_WiiUDRC::UIComponent_WiiUDRC(int sceneType, void* param, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_iconY = -1;
    m_drcOutputEnabled = true;
    m_iconHeight = -1;
    m_touchY = 0;
    m_drcIconEnabled = false;
    m_iconWidth = -1;
    m_touchX = 0;
    m_iconX = -1;
    SetEnableBackground(false);
    SetDRCIcon(false, false);
    GetIconWidth();
    GetIconHeight();
    GetIconX();
    GetIconY();
}

int UIComponent_WiiUDRC::GetIconX()
{
    if (m_iconX < 1)
        m_iconX = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconX();
    return m_iconX;
}

int UIComponent_WiiUDRC::GetIconWidth()
{
    if (m_iconWidth < 1)
        m_iconWidth = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconWidth();
    return m_iconWidth;
}

int UIComponent_WiiUDRC::GetIconY()
{
    if (m_iconY < 1)
        m_iconY = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconY();
    return m_iconY;
}

int UIComponent_WiiUDRC::GetIconHeight()
{
    if (m_iconHeight < 1)
        m_iconHeight = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconHeight();
    return m_iconHeight;
}

void UIComponent_WiiUDRC::handleDRCTouchInput(unsigned int playerIndex)
{
    bool pressed = false;
    int x;
    int y;
    unsigned short* touchData = g_pInput->GetTouchPadData(playerIndex, C_4JInput::ETouchType_0, true);
    if (touchData != NULL && touchData[2] == 1 && touchData[3] == 0)
    {
        x = touchData[0];
        y = touchData[1];
        m_touchX = x;
        pressed = true;
        m_touchY = y;
    }
    else if (m_touchingIcon)
    {
        x = m_touchX;
        y = m_touchY;
    }
    if (pressed || m_touchingIcon)
    {
        if (!gApp.GetDRCIconEnabled())
            return;
        if (x > GetIconX() && x < GetIconX() + GetIconWidth() && y > GetIconY() && y < GetIconY() + GetIconHeight())
        {
            if (pressed)
            {
                if (!m_touchingIcon)
                    m_touchPressed = true;
            }
            else if (m_touchingIcon && m_touchPressed)
                gApp.SetDRCOuputEnabled(!gApp.GetDRCOutputEnabled());
        }
        m_touchingIcon = pressed;
    }
    else
    {
        m_touchingIcon = false;
        m_touchPressed = false;
    }
}

std::wstring UIComponent_WiiUDRC::getMoviePath()
{
    return std::wstring(L"WiiUDRC");
}

void UIComponent_WiiUDRC::SetEnableBackground(bool enable)
{
    static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->ShowDRCBackground(enable);
}

void UIComponent_WiiUDRC::SetDRCIcon(bool enable, bool connected)
{
    static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->ShowDRCIcon(enable, connected);
}

void UIComponent_WiiUDRC::tick()
{
    UIScene::tick();
    if (!gConsoleUIController.IsReloadingSkin())
    {
        bool iconEnabled = gApp.GetDRCIconEnabled();
        bool outputEnabled = gApp.GetDRCOutputEnabled();
        if (iconEnabled != m_drcIconEnabled || outputEnabled != m_drcOutputEnabled)
        {
            SetDRCIcon(iconEnabled, !outputEnabled);
            if (outputEnabled != m_drcOutputEnabled)
                SetEnableBackground(!outputEnabled);
            m_drcIconEnabled = iconEnabled;
            m_drcOutputEnabled = outputEnabled;
        }
    }
}

void UIComponent_WiiUDRC::handleReload()
{
    SetDRCIcon(m_drcIconEnabled, !m_drcOutputEnabled);
    SetEnableBackground(!m_drcOutputEnabled);
}

int UIComponent_WiiUDRC::getSceneType()
{
    return EUIScene_WiiUDRC;
}

bool UIComponent_WiiUDRC::stealsFocus()
{
    return false;
}

bool UIComponent_WiiUDRC::hasFocus(int playerIndex)
{
    return false;
}

bool UIComponent_WiiUDRC::hidesLowerScenes()
{
    return false;
}
