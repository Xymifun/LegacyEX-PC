#include "game/ui/FJ_List_ListButtonPlayerList.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonPlayerList.hpp"

FJ_List_ListButtonPlayerList::FJ_List_ListButtonPlayerList(fuiRenderNode* renderNode) : FJ_List(renderNode)
{
}

FJ_List* FJ_List_ListButtonPlayerList::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonPlayerList(renderNode);
}

uint32_t FJ_List_ListButtonPlayerList::getType()
{
    return 0x803a;
}

void FJ_List_ListButtonPlayerList::addNewItem(const std::wstring& label, int id, int playerIcon, int voipIcon)
{
    FJ_ListButtonPlayerList* item;
    if (m_checkboxTick)
    {
        item = static_cast<FJ_ListButtonPlayerList*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonPlayerList_Small", m_pRenderNode, true, NULL));
    }
    else
    {
        item = static_cast<FJ_ListButtonPlayerList*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonPlayerList_Normal", m_pRenderNode, true, NULL));
    }

    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetPlayerIcon(playerIcon);
    item->SetVOIPIcon(voipIcon);
}

void FJ_List_ListButtonPlayerList::SetPlayerIcon(int item, int icon)
{
    FJ_ListButtonPlayerList* button = static_cast<FJ_ListButtonPlayerList*>(ReturnItem(item));
    if (button != NULL)
        button->SetPlayerIcon(icon);
}

void FJ_List_ListButtonPlayerList::SetVOIPIcon(int item, int icon)
{
    FJ_ListButtonPlayerList* button = static_cast<FJ_ListButtonPlayerList*>(ReturnItem(item));
    if (button != NULL)
        button->SetVOIPIcon(icon);
}
