#include "game/ui/FJ_ListButtonLayerIcon.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_ListButtonLayerIcon::FJ_ListButtonLayerIcon(fuiRenderNode* pRenderNode) : FJ_Button(pRenderNode)
{
}

void FJ_ListButtonLayerIcon::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    FJ_Button::Init(name, id);
}

void FJ_ListButtonLayerIcon::SetSlotID(int slotId, bool visible)
{
    FJ_FuiNode* slotControl = FJ_FuiNode::asMovieClip(getChildByName("SlotControl"));
    if (slotControl == NULL)
        return;

    if (!visible)
    {
        removeChild(slotControl, true);
        return;
    }

    fuiRect rect;
    rect.xMin = slotControl->getX();
    rect.yMin = slotControl->getY();
    rect.xMax = rect.xMin + slotControl->getWidth() / getScaleX();
    rect.yMax = rect.yMin + slotControl->getHeight();

    char name[64];
    std::sprintf(name, "iggy_slot_%d_%d", slotId, m_id);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
    renderNode->setAlpha(0.25f);
    removeChild(slotControl, true);
    FJ_FuiNode* slotNode = new FJ_FuiNode(renderNode);
    renderNode->setFuiNode(slotNode);
    addChild(slotNode);
}

void FJ_ListButtonLayerIcon::SetTextureName(const std::wstring& textureName, const std::wstring&)
{
    if (!textureName.empty() && m_textureNode == NULL)
    {
        FJ_FuiNode* iconPlaceholder = getChildByName("IconPlaceholder")->asMovieClip();
        iconPlaceholder->setScaleX(iconPlaceholder->getScaleX() / getScaleX());
        iconPlaceholder->setScaleY(iconPlaceholder->getScaleY() / getScaleY());

        fuiRect rect;
        rect.xMin = 0.0f;
        rect.yMin = 0.0f;
        rect.xMax = iconPlaceholder->getWidth() / iconPlaceholder->getScaleX();
        rect.yMax = iconPlaceholder->getHeight() / iconPlaceholder->getScaleY();

        unsigned int width;
        unsigned int height;
        bool hasTexture = false;
        unsigned int texture = gFui->getCustomTexture(textureName, &width, &height, &hasTexture);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
            getStage()->m_pRenderNode, &rect, "CustomTexture", width, height, texture, true, hasTexture);
        m_textureNode = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(m_textureNode);
        iconPlaceholder->addChild(m_textureNode);
    }
}
