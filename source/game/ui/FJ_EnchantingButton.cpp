#include "game/ui/FJ_EnchantingButton.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

static std::string s_enchantingButtonStates[3] = {std::string("Inactive"), std::string("Active"),
                                                  std::string("Selected")};

FJ_EnchantingButton::FJ_EnchantingButton(fuiRenderNode* pRenderNode) : FJ_Base(pRenderNode)
{
    SetLabelAlignment(0);
    m_isLabel = false;
    InitBase(L"", 0);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
    SetupIggySlot();
    m_state = 0;
}

void FJ_EnchantingButton::HandleUpdateLabel(FJ_Event*)
{
    SetLabel(m_label);
}

void FJ_EnchantingButton::_HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_EnchantingButton*>(pNode)->HandleUpdateLabel(pEvent);
}

void FJ_EnchantingButton::SetupIggySlot()
{
    FJ_FuiNode* icon = getChildByName(std::string("IconPlaceholder"))->asMovieClip();
    fuiRect rect;
    rect.xMin = 0.0f;
    rect.yMin = 0.0f;
    rect.xMax = icon->getWidth() / icon->getScaleX();
    rect.yMax = icon->getHeight() / icon->getScaleY();
    char name[64];
    std::sprintf(name, "iggy_slot_%s", getName());
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
    renderNode->setAlpha(0.25f);
    FJ_FuiNode* blank = new FJ_FuiNode(renderNode);
    renderNode->setFuiNode(blank);
    icon->addChild(blank);
}

void FJ_EnchantingButton::SetEnchantmentLevel(int level)
{
    FJ_FuiNode* levelNode = getChildByName(std::string("EnchantmentLevel"))->asMovieClip();
    int frameOffset = m_state != 0 ? 3 : 0;
    levelNode->gotoAndStop(level + frameOffset);
    m_enchantmentLevel = level;
}

void FJ_EnchantingButton::ChangeState(int state)
{
    gotoAndStop(s_enchantingButtonStates[state]);
    m_state = state;
    SetEnchantmentLevel(m_enchantmentLevel);
}
