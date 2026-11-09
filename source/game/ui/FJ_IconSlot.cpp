#include "game/ui/FJ_IconSlot.hpp"

#include <cstdio>

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/util/Math.hpp"

FJ_IconSlot::FJ_IconSlot(fuiRenderNode* pRenderNode) : FJ_Base(pRenderNode)
{
}

FJ_Base* FJ_IconSlot::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_IconSlot(pRenderNode);
}

unsigned int FJ_IconSlot::getType()
{
    return 0x800b;
}

void FJ_IconSlot::ShowSlotHolder(bool bVisible)
{
    m_pIconHolder->setVisible(bVisible);
}

void FJ_IconSlot::ShowRedSlot(bool bRed)
{
    m_pIconHolder->gotoAndStop(bRed ? "SlotRed" : "SlotNormal");
}

void FJ_IconSlot::SetParams(int iWidth, bool bUseSmallText)
{
    m_pIconHolder = getChildByName("IconHolder")->asMovieClip();
    m_pBitmapIcon = getChildByName("BitmapIcon")->asFJ_BitmapIcon();

    if (bUseSmallText)
    {
        removeChild(getChildByName("FJ_TextContainer"), true);
        FJ_FuiNode* smallText =
            FJ_FuiNode::asMovieClip(getChildByName("SmallTextContainer"))->getChildAt(0)->asMovieClip();
        addChild(smallText);
        removeChild(getChildByName("SmallTextContainer"), true);
    }
    else
    {
        removeChild(getChildByName("SmallTextContainer"), true);
    }

    if (iWidth > 0)
    {
        setWidth((float)iWidth);
        setHeight((float)iWidth);
    }
    ShowRedSlot(false);
}

void FJ_IconSlot::Init(const std::wstring& label)
{
    SetLabelAlignment(2);
    InitBase(label, -1);
}

void FJ_IconSlot::SetLabel(const std::wstring& label)
{
    SetLabelAlignment(2);
    m_initialised = true;
    FJ_Base::SetLabel(label);
}

void FJ_IconSlot::SetSlotID(const std::string& slotName, int iSlotId)
{
    m_iSlotId = iSlotId;
    FJ_FuiNode* oldControl = m_pIconHolder->getChildByName("SlotControl");
    if (oldControl != NULL)
    {
        fuiRect rect;
        int roundedScale = Math::round(getScaleX());
        rect.xMin = (float)Math::round((float)roundedScale / getScaleX());
        rect.xMax = rect.xMin + (float)Math::round(getWidth() - (float)(roundedScale * 2)) / getScaleX();
        rect.yMin = rect.xMin;
        rect.yMax = rect.xMax;

        char name[64];
        std::sprintf(name, "iggy_slot_%s_%d", slotName.c_str(), iSlotId);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
        renderNode->setAlpha(0.25f);
        m_pIconHolder->removeChild(oldControl, true);
        FJ_FuiNode* slotNode = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(slotNode);
        addChild(slotNode);
    }
}

void FJ_IconSlot::SetTextureName(const std::wstring& textureName)
{
    m_pBitmapIcon->SetTextureName(textureName);
}
