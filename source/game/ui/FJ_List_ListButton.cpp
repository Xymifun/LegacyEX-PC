#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButton.hpp"

#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_List_ListButton::FJ_List_ListButton(fuiRenderNode* renderNode)
    : FJ_List(renderNode),
      m_labelAlignment(1),
      m_mediumButtons(false)
{
}

void FJ_List_ListButton::addNewItem(const std::wstring& label, int id)
{
    FJ_ListButton* button;
    if (m_checkboxTick)
    {
        button = static_cast<FJ_ListButton*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Small", m_pRenderNode, true, NULL));
    }
    else if (m_mediumButtons)
    {
        button = static_cast<FJ_ListButton*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Medium", m_pRenderNode, true, NULL));
    }
    else
    {
        button = static_cast<FJ_ListButton*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Normal", m_pRenderNode, true, NULL));
    }

    addNewListItem(button, label, id);
    button->m_labelAlignment = m_labelAlignment;
    button->Init(label, id);
}

FJ_List* FJ_List_ListButton::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButton(renderNode);
}

uint32_t FJ_List_ListButton::getType()
{
    return 0x8034;
}
