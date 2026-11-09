#include "game/ui/FJ_List_ListButtonDLC.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonDLC.hpp"

FJ_List_ListButtonDLC::FJ_List_ListButtonDLC(fuiRenderNode* renderNode) : FJ_List(renderNode)
{
}

FJ_List* FJ_List_ListButtonDLC::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonDLC(renderNode);
}

uint32_t FJ_List_ListButtonDLC::getType()
{
    return 0x8037;
}

void FJ_List_ListButtonDLC::addNewItem(const std::wstring& label, int id, bool showTick)
{
    const char* objectName = m_checkboxTick ? "fourj.Buttons.FJ_ListButtonDLC_Small"
                                            : "fourj.Buttons.FJ_ListButtonDLC_Normal";
    FJ_ListButtonDLC* item = static_cast<FJ_ListButtonDLC*>(
        FJ_Factory::createObject(objectName, m_pRenderNode, true, NULL));
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->ShowTick(showTick);
}

void FJ_List_ListButtonDLC::ShowTick(int index, bool showTick)
{
    FJ_ListButtonDLC* item = static_cast<FJ_ListButtonDLC*>(ReturnItem(index));
    if (item != NULL)
        item->ShowTick(showTick);
}
