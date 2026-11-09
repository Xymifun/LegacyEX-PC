#include "game/ui/FJ_ListButtonCheckbox.hpp"
#include "game/ui/fui.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

extern fui* gFui;

FJ_ListButtonCheckbox::FJ_ListButtonCheckbox(fuiRenderNode* renderNode) : FJ_Button(renderNode), m_checkBox(true)
{
    SetCheckBox(true);
    SetKeyDownDelegate(_HandleTriggerCheckBox, this);
}

void FJ_ListButtonCheckbox::SetCheckBox(bool checked)
{
    FJ_FuiNode* checkBoxTick = getChildByName(std::string("CheckBoxTick"))->asMovieClip();
    m_checkBox = checked;
    checkBoxTick->setVisible(checked);
}

void FJ_ListButtonCheckbox::HandleTriggerCheckBox(int)
{
    if (getEnabled())
        SetCheckBox(!m_checkBox);
}

void FJ_ListButtonCheckbox::_HandleTriggerCheckBox(FJ_FuiNode* node, int id)
{
    static_cast<FJ_ListButtonCheckbox*>(node)->HandleTriggerCheckBox(id);
}

void FJ_ListButtonCheckbox::UpdateCheckBoxScales()
{
    FJ_FuiNode* checkBox = getChildByName(std::string("CheckBox"))->asMovieClip();
    FJ_FuiNode* checkBoxTick = getChildByName(std::string("CheckBoxTick"))->asMovieClip();
    checkBox->setScaleX(checkBoxTick->getScaleX());
    checkBox->setScaleY(checkBoxTick->getScaleY());
}

void FJ_ListButtonCheckbox::_UpdateCheckBoxScales(FJ_Button* button)
{
    static_cast<FJ_ListButtonCheckbox*>(button)->UpdateCheckBoxScales();
}

void FJ_ListButtonCheckbox::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    FJ_FuiNode* checkBoxTick = getChildByName(std::string("CheckBoxTick"))->asMovieClip();
    checkBoxTick->setScaleX(checkBoxTick->getScaleX() / getScaleX());
    checkBoxTick->setScaleY(checkBoxTick->getScaleY() / getScaleY());
    UpdateCheckBoxScales();
    m_updateLabelDelegate = _UpdateCheckBoxScales;
    FJ_Button::Init(name, id);
}

void FJ_ListButtonCheckbox::SetSlotID(int slotId, bool visible)
{
    FJ_FuiNode* slotControl = FJ_FuiNode::asMovieClip(getChildByName(std::string("SlotControl")));
    if (slotControl != NULL)
    {
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
        FJ_FuiNode* node = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(node);
        addChild(node);
    }
}

void FJ_ListButtonCheckbox::SetTextureName(const std::wstring& textureName)
{
    FJ_FuiNode* iconPlaceholder = getChildByName(std::string("IconPlaceholder"))->asMovieClip();
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
    FJ_FuiNode* node = new FJ_FuiNode(renderNode);
    renderNode->setFuiNode(node);
    iconPlaceholder->addChild(node);
}

void FJ_ListButtonCheckbox::EnableButton(bool enabled)
{
    SetCheckBox(false);
    FJ_Button::EnableButton(enabled);
}

uint8_t FJ_ListButtonCheckbox::GetCheckBox()
{
    return m_checkBox ? 1 : 0;
}
