#include "game/ui/FJ_BitmapIcon.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fui.hpp"

FJ_BitmapIcon::FJ_BitmapIcon(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode), m_pBitmapNode(NULL)
{
}

FJ_FuiNode* FJ_BitmapIcon::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_BitmapIcon(pRenderNode);
}

unsigned int FJ_BitmapIcon::getType()
{
    return 0x67;
}

void FJ_BitmapIcon::SetTextureName(const std::wstring& textureName)
{
    if (textureName.empty())
    {
        if (m_pBitmapNode != NULL)
            m_pBitmapNode->setVisible(false);
        return;
    }

    if (m_pBitmapNode != NULL)
    {
        if (!m_pBitmapNode->isVisible())
        {
            if (m_pBitmapNode != NULL)
                m_pBitmapNode->setVisible(true);
        }
    }

    if (m_pBitmapNode == NULL)
    {
        fuiRect rect;
        rect.yMin = rect.xMin = 0.0f;
        rect.xMax = getWidth() / getScaleX();
        rect.yMax = getHeight() / getScaleY();
        unsigned int width;
        unsigned int height;
        bool hasTexture = false;
        unsigned int texture = gFui->getCustomTexture(textureName, &width, &height, &hasTexture);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
            getStage()->m_pRenderNode, &rect, "CustomTexture", width, height, texture, true, hasTexture);
        m_pBitmapNode = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(m_pBitmapNode);
        addChild(m_pBitmapNode);
    }
    else
    {
        unsigned int width;
        unsigned int height;
        bool hasTexture = false;
        unsigned int texture = gFui->getCustomTexture(textureName, &width, &height, &hasTexture);
        static_cast<fuiRenderNodeCodeGenRect*>(m_pBitmapNode->m_pRenderNode)
            ->setTexture(width, height, texture, hasTexture);
    }
}
