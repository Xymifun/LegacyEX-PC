#include "game/ui/UIControl_TexturePackList.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_TexturePackList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_TexturePackList::UIControl_TexturePackList()
{
}

void UIControl_TexturePackList::init(const std::wstring& label, int id)
{
    m_label = UIString(label);
    m_controlId = id;
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->Init(label, id);
    switch (m_scene->m_layer->m_layerId)
    {
    case EUILayer_Fullscreen:
    case EUILayer_Scene:
    case EUILayer_HUD:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    }
}

void UIControl_TexturePackList::setEnabled(bool enabled)
{
    if (m_fuiRenderNode == NULL)
    {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->EnableSelector(enabled);
}

bool UIControl_TexturePackList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_TexturePackList;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TexturePackList::addPack(int id, const std::wstring& name, bool alreadyAdded)
{
    if (m_fuiRenderNode == NULL)
    {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->addPack(id, name);
    if (!alreadyAdded)
    {
        PackDetails details = {id, name};
        m_packDetails.push_back(details);
    }
}

void UIControl_TexturePackList::ReInit()
{
    init(m_label.getString(), m_controlId);
    for (unsigned int i = 0; i < m_packDetails.size(); ++i)
    {
        addPack(m_packDetails[i].id, m_packDetails[i].name, true);
    }
    UIControl::ReInit();
}

void UIControl_TexturePackList::selectSlot(int slot)
{
    if (m_fuiRenderNode == NULL)
    {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->SelectSlot(slot);
}

void UIControl_TexturePackList::clearSlots()
{
    if (m_fuiRenderNode != NULL)
    {
        static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->removeAllItems();
    }
    m_packDetails.clear();
}

void UIControl_TexturePackList::SetTouchFocus(int x, int y, bool focus)
{
    if (m_fuiRenderNode == NULL)
    {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->SetTouchFocus(x, y, focus);
}

unsigned int UIControl_TexturePackList::CanTouchTrigger(int x, int y)
{
    if (m_fuiRenderNode == NULL)
    {
        return 0;
    }
    return static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->CanTouchTrigger(x, y);
}

int UIControl_TexturePackList::GetRealHeight()
{
    int height = 0;
    if (m_fuiRenderNode != NULL)
    {
        height = (int)static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getFuiNode())->GetRealHeight();
    }
    return height;
}
