#include "game/ui/FJ_List.hpp"

#include "game/ui/FJ_List_ListButtonLobby.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonLobby.hpp"

FJ_List_ListButtonLobby::FJ_List_ListButtonLobby(fuiRenderNode* renderNode) : FJ_List(renderNode)
{
}

FJ_List* FJ_List_ListButtonLobby::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonLobby(renderNode);
}

uint32_t FJ_List_ListButtonLobby::getType()
{
    return 0x8039;
}

void FJ_List_ListButtonLobby::addNewItem(const std::wstring& label, int id, bool showTick, int playerIcon, int voipIcon)
{
    const char* objectName = m_checkboxTick ? "fourj.Buttons.FJ_ListButtonLobby_Small"
                                            : "fourj.Buttons.FJ_ListButtonLobby_Normal";
    FJ_ListButtonLobby* item = static_cast<FJ_ListButtonLobby*>(
        FJ_Factory::createObject(objectName, m_pRenderNode, true, NULL));
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->ShowTick(showTick);
    item->SetPlayerIcon(playerIcon);
    item->SetVOIPIcon(voipIcon);
}
