#include "game/ui/FJ_List_MenuButton.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_MenuButton.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_List_MenuButton::FJ_List_MenuButton(fuiRenderNode* renderNode) : FJ_List(renderNode), m_useEllipsis(false)
{
}

void FJ_List_MenuButton::addNewItem(std::wstring label, int id)
{
    FJ_MenuButton* button;
    if (m_checkboxTick)
    {
        button = static_cast<FJ_MenuButton*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Small", m_pRenderNode, true, 0));
        addNewListItem(button, label, id);
        button->UseEllipsis(m_useEllipsis);
        button->Init(label, id);
    }
    else
    {
        button = static_cast<FJ_MenuButton*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Normal", m_pRenderNode, true, 0));
        addNewListItem(button, label, id);
        button->UseEllipsis(m_useEllipsis);
        button->Init(label, id);
    }
}

FJ_List* FJ_List_MenuButton::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_MenuButton(renderNode);
}

uint32_t FJ_List_MenuButton::getType()
{
    return 0x8033;
}
