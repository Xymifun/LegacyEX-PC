#include "game/ui/FJ_EndPoem.hpp"

#include <cmath>
#include <cstdio>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/util/Math.hpp"

FJ_EndPoem::FJ_EndPoem(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_tileCount = 0;
    m_tileColumns = 0;
    m_tileRows = 0;
    m_moveTiles = false;
    m_textNodeCount = 0;
    m_lastLabelShown = false;
    m_lastLabel = 0;
    m_scrollSpeed = 0;
    m_appending = false;
    m_tilePrototype = NULL;
}

FJ_EndPoem::~FJ_EndPoem()
{
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

FJ_Document* FJ_EndPoem::Create(fuiRenderNode* renderNode)
{
    return new FJ_EndPoem(renderNode);
}

unsigned int FJ_EndPoem::getType()
{
    return 0x4d;
}

void FJ_EndPoem::RequestNextLabel(int label)
{
    getRoot()->m_pCallbackScene->handleRequestMoreData(label, false);
}

void FJ_EndPoem::UpdateTileBackground()
{
    if (m_tileCount == 0)
    {
        m_tileColumns = (int)ceilf(getStage()->getStageWidth() / m_tilePrototype->getWidth());
        int rows = (int)ceilf(getStage()->getStageHeight() / m_tilePrototype->getHeight());
        m_tileRows = rows + 1;
        int tileCount = (rows + 1) * m_tileColumns;
        if (tileCount > 48)
            tileCount = 48;
        m_tileCount = tileCount;
        for (int i = 0; i < m_tileCount; ++i)
        {
            FJ_FuiNode* tile = FJ_Factory::createObject("DirtTile", m_pRenderNode, true, NULL);
            int row = i / m_tileColumns;
            int column = i % m_tileColumns;
            tile->setX(column * m_tilePrototype->getWidth());
            tile->setY(row * m_tilePrototype->getHeight());
            m_tiles[i] = tile;
            addChildAt(tile, 0);
        }
    }
    else
    {
        m_moveTiles = !m_moveTiles;
        if (m_moveTiles)
        {
            for (int i = 0; i < m_tileCount; ++i)
            {
                m_tiles[i]->decY(1.0f);
                if (m_tiles[i]->getY() + m_tiles[i]->getHeight() < 0.0f)
                    m_tiles[i]->incY(m_tileRows * m_tilePrototype->getHeight());
            }
        }
    }
}

void FJ_EndPoem::UpdateScrolling(FJ_Event* event)
{
    if (m_scrollSpeed > 0)
    {
        if (m_lastLabel != 0 && m_visibleNodes.size() != 0 && m_visibleLabels[m_visibleNodes.size() - 1] < m_lastLabel)
            m_lastLabelShown = false;
        if (!m_lastLabelShown)
        {
            if (!m_visibleNodes.empty())
            {
                if (m_visibleNodes[m_visibleNodes.size() - 1]->getY()
                        + m_visibleNodes[m_visibleNodes.size() - 1]->getHeight()
                    < getStage()->getStageHeight())
                {
                    m_appending = true;
                    RequestNextLabel(m_visibleLabels[m_visibleNodes.size() - 1] + 1);
                }
            }
            else
            {
                m_appending = true;
                RequestNextLabel(0);
            }
        }
    }
    else if (m_visibleNodes.size() != 0)
    {
        if (m_visibleNodes.front()->getY() > 0.0f && m_visibleLabels.front() > 0)
        {
            m_appending = false;
            RequestNextLabel(m_visibleLabels.front() - 1);
        }
    }

    if (!(m_scrollSpeed > 0 && m_lastLabelShown && m_visibleNodes.size() != 0
          && m_visibleNodes[m_visibleNodes.size() - 1]->getY()
                     + m_visibleNodes[m_visibleNodes.size() - 1]->getHeight() * 0.5f
                 < getStage()->getStageHeight() * 0.5f))
    {
        for (unsigned int i = 0; i < m_visibleNodes.size(); ++i)
            m_visibleNodes[i]->decY((float)Math::round(1.0f) * m_scrollSpeed);
    }

    if (m_visibleNodes.size() != 0)
    {
        if (m_scrollSpeed > 0 && m_visibleNodes.front()->getY() + m_visibleNodes.front()->getHeight() < 0.0f)
        {
            m_visibleNodes.front()->asFJ_Base()->SetLabel(L"");
            m_visibleNodes.front()->setVisible(false);
            m_visibleNodes.erase(m_visibleNodes.begin());
            m_visibleLabels.erase(m_visibleLabels.begin());
        }
        if (m_scrollSpeed < 0 && m_visibleNodes[m_visibleNodes.size() - 1]->getY() > getStage()->getStageHeight())
        {
            m_visibleNodes[m_visibleNodes.size() - 1]->asFJ_Base()->SetLabel(L"");
            m_visibleNodes[m_visibleNodes.size() - 1]->setVisible(false);
            m_visibleNodes[m_visibleNodes.size() - 1]->setY(-getStage()->getStageHeight());
            m_visibleNodes.pop_back();
            m_visibleLabels.pop_back();
        }
    }
    else
    {
        m_lastLabelShown = false;
    }
    UpdateTileBackground();
}

void FJ_EndPoem::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_EndPoem*>(node)->UpdateScrolling(event);
}

void FJ_EndPoem::keyUpHandler(FJ_Event* event)
{
    m_scrollSpeed = 1;
}

void FJ_EndPoem::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_EndPoem*>(node)->keyUpHandler(event);
}

void FJ_EndPoem::init(FJ_Event* event)
{
    m_lastLabel = 0;
    m_lastLabelShown = false;
    m_textNodeCount = 0;
    m_appending = true;
    m_scrollSpeed = 1;
    addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    getStage()->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    for (int i = 0; i < 20; ++i)
    {
        char name[64];
        sprintf(name, "EndPoemText_%d", i);
        FJ_FuiNode* node = getChildByName(name);
        if (node != NULL)
        {
            m_textNodes[i] = node;
            m_textNodes[i]->setX(Math::round(getStage()->getStageWidth() * 0.5f - m_textNodes[i]->getWidth() * 0.5f
                                             + getRect(this).xMin));
            m_textNodes[i]->setY(-getStage()->getStageHeight());
            m_textNodes[i]->setVisible(false);
            ++m_textNodeCount;
        }
    }
    m_tilePrototype = FJ_Factory::createObject("DirtTile", m_pRenderNode, true, NULL);
}

unsigned int FJ_EndPoem::SetNextLabel(const std::wstring& label, int index, bool lastLabel)
{
    unsigned int added = 0;
    if (m_appending)
    {
        m_lastLabelShown = lastLabel;
        m_lastLabel = index;
    }

    std::wstring text = label;
    if (text.empty())
        text = L" ";

    for (int i = 0; i < m_textNodeCount; ++i)
    {
        if (m_textNodes[i]->getY() + m_textNodes[i]->getHeight() < 0.0f)
        {
            m_textNodes[i]->setVisible(true);
            m_textNodes[i]->asFJ_Base()->SetLabel(text);
            if (!m_appending)
            {
                m_visibleNodes.insert(m_visibleNodes.begin(), m_textNodes[i]);
                m_visibleLabels.insert(m_visibleLabels.begin(), index);
            }
            else
            {
                m_visibleNodes.push_back(m_textNodes[i]);
                m_visibleLabels.push_back(index);
            }
            added = 1;
            if (m_visibleNodes.size() != 0)
            {
                if (!m_appending)
                {
                    if (m_visibleNodes.size() > 1)
                        m_visibleNodes.front()->setY(m_visibleNodes[1]->getY() - m_visibleNodes.front()->getHeight());
                }
                else if (m_visibleNodes.size() != 1)
                {
                    m_visibleNodes[m_visibleNodes.size() - 1]->setY(
                        m_visibleNodes[m_visibleNodes.size() - 2]->getY()
                        + m_visibleNodes[m_visibleNodes.size() - 2]->getHeight());
                }
                else
                {
                    m_visibleNodes[m_visibleNodes.size() - 1]->setY(getStage()->getStageHeight());
                }
            }
            break;
        }
    }
    return added;
}

void FJ_EndPoem::keyDownHandler(FJ_Event* event)
{
    int keyCode = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (keyCode == 3 || keyCode == 17)
        m_scrollSpeed = -3;
    if (keyCode == 4 || keyCode == 18)
        m_scrollSpeed = 3;
}
