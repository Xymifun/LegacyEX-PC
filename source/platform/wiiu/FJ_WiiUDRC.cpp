#include "platform/wiiu/FJ_WiiUDRC.hpp"

#include <cmath>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/util/Math.hpp"
#include "platform/wiiu/CInput.hpp"

extern CConsoleMinecraftApp gApp;

static const float s_drcDesignWidth = 1280.0f;
static const float s_drcDesignHeight = 720.0f;
static const int s_drcPad = 4;

static unsigned char s_drcConnected = 1;

void HandleDRCStatusChanges()
{
    unsigned char drcConnected = g_pInput->IsPadDRCController(s_drcPad);
    if (drcConnected != s_drcConnected)
    {
        if (drcConnected != 0)
        {
            if (gApp.GetDRCIconEnabled())
                gApp.SetDRCOuputEnabled(true);
        }
        else
            gApp.SetDRCOuputEnabled(false);
        s_drcConnected = drcConnected;
    }
}

FJ_WiiUDRC::FJ_WiiUDRC(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode),
      m_tileCount(0),
      m_tilesAcross(0),
      m_tilesDown(0),
      m_backgroundVisible(false)
{
    ShowDRCBackground(false);
    ShowDRCIcon(true, true);
    SetSafeZone(25, 25, 25, 25);
}

void FJ_WiiUDRC::UpdateTileBackground(FJ_Event*)
{
    if (m_tileCount == 0)
    {
        FJ_FuiNode* tile = FJ_Factory::createObject("DirtTile", m_pRenderNode, true, NULL);
        m_tilesAcross = (int)ceilf(getStage()->getStageWidth() / tile->getWidth());
        int tilesDown = (int)ceilf(getStage()->getStageHeight() / tile->getHeight()) + 1;
        int tileCount = tilesDown * m_tilesAcross;
        if (tileCount > MAX_TILES)
            tileCount = MAX_TILES;
        m_tilesDown = tilesDown;
        m_tileCount = tileCount;

        for (int i = 0; i < m_tileCount; ++i)
        {
            FJ_FuiNode* node = FJ_Factory::createObject("DirtTile", m_pRenderNode, true, NULL);
            int row = i / m_tilesAcross;
            node->setX((float)(i - row * m_tilesAcross) * tile->getWidth());
            node->setY((float)row * tile->getHeight());
            m_tiles[i] = node;
            getChildByName(std::string("DRC_Background"))->addChildAt(node, 0);
        }

        if (tile != NULL)
            delete tile;
    }
}

void FJ_WiiUDRC::_UpdateTileBackground(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_WiiUDRC*>(pNode)->UpdateTileBackground(pEvent);
}

void FJ_WiiUDRC::ShowDRCBackground(bool bShow)
{
    if (bShow)
        addEventListener(eFJEventType_EnterFrame, this, _UpdateTileBackground, false);
    else
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateTileBackground);
    getChildByName(std::string("DRC_Background"))->setVisible(bShow);
}

void FJ_WiiUDRC::ShowDRCIcon(bool bOn, bool bConnected)
{
    if (getChildByName(std::string("DRC_On_Icon")) != NULL)
        getChildByName(std::string("DRC_On_Icon"))->setVisible(bOn && bConnected);
    if (getChildByName(std::string("DRC_Off_Icon")) != NULL)
        getChildByName(std::string("DRC_Off_Icon"))->setVisible(bOn && !bConnected);
}

void FJ_WiiUDRC::SetSafeZone(int, int iBottom, int, int iRight)
{
    float scaleX = getStage()->getStageWidth() / s_drcDesignWidth;
    float scaleY = getStage()->getStageHeight() / s_drcDesignHeight;

    if (getChildByName(std::string("DRC_On_Icon")) != NULL)
    {
        getChildByName(std::string("DRC_On_Icon"))
            ->setX((float)Math::round(getStage()->getStageWidth()
                                      - getChildByName(std::string("DRC_On_Icon"))->getWidth() - iRight * scaleX));
        getChildByName(std::string("DRC_On_Icon"))
            ->setY((float)Math::round(getStage()->getStageHeight()
                                      - getChildByName(std::string("DRC_On_Icon"))->getHeight() - iBottom * scaleY));
    }
    if (getChildByName(std::string("DRC_Off_Icon")) != NULL)
    {
        getChildByName(std::string("DRC_Off_Icon"))
            ->setX((float)Math::round(getStage()->getStageWidth()
                                      - getChildByName(std::string("DRC_On_Icon"))->getWidth() - iRight * scaleX));
        getChildByName(std::string("DRC_Off_Icon"))
            ->setY((float)Math::round(getStage()->getStageHeight()
                                      - getChildByName(std::string("DRC_On_Icon"))->getHeight() - iBottom * scaleY));
    }
}

FJ_WiiUDRC* FJ_WiiUDRC::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_WiiUDRC(pRenderNode);
}

unsigned int FJ_WiiUDRC::getType()
{
    return 0x5a;
}

int FJ_WiiUDRC::GetIconX()
{
    float x = getChildByName(std::string("DRC_On_Icon")) != NULL ? getChildByName(std::string("DRC_On_Icon"))->getX()
                                                                 : 0.0f;
    float scale = s_drcDesignWidth / getStage()->getStageWidth();
    return (int)(x * scale);
}

int FJ_WiiUDRC::GetIconY()
{
    float y = getChildByName(std::string("DRC_On_Icon")) != NULL ? getChildByName(std::string("DRC_On_Icon"))->getY()
                                                                 : 0.0f;
    float scale = s_drcDesignHeight / getStage()->getStageHeight();
    return (int)(y * scale);
}

int FJ_WiiUDRC::GetIconWidth()
{
    float width = getChildByName(std::string("DRC_On_Icon")) != NULL
                      ? getChildByName(std::string("DRC_On_Icon"))->getWidth()
                      : 0.0f;
    float scale = s_drcDesignWidth / getStage()->getStageWidth();
    return (int)(width * scale);
}

int FJ_WiiUDRC::GetIconHeight()
{
    float height = getChildByName(std::string("DRC_On_Icon")) != NULL
                       ? getChildByName(std::string("DRC_On_Icon"))->getHeight()
                       : 0.0f;
    float scale = s_drcDesignHeight / getStage()->getStageHeight();
    return (int)(height * scale);
}
