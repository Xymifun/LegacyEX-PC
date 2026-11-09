#include "game/ui/UIControl_ScoreboardButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_ScoreboardButtonList::PendingAddItem::PendingAddItem(UIControl_ScoreboardButtonList* list, int type,
                                                               std::wstring label, int playerIcon, int voipIcon)
    : UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>(list, type),
      m_label(label),
      m_playerIcon(playerIcon),
      m_voipIcon(voipIcon)
{
}

unsigned int UIControl_ScoreboardButtonList::PendingAddItem::OnTick()
{
    m_list->addItem(m_label, m_playerIcon, m_voipIcon, true);
    return 1;
}

unsigned int UIControl_ScoreboardButtonList::PendingSetSlotTextureName::OnTick()
{
    m_list->SetSlotTextureName(m_item, m_slot, m_textureName, true);
    return 1;
}

UIControl_ScoreboardButtonList::PendingSetSlotsText::PendingSetSlotsText(UIControl_ScoreboardButtonList* list, int item,
                                                                         std::wstring slot0, std::wstring slot1,
                                                                         std::wstring slot2, std::wstring slot3,
                                                                         std::wstring slot4, std::wstring slot5,
                                                                         std::wstring slot6)
    : UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>(list, item),
      m_slot0(slot0),
      m_slot1(slot1),
      m_slot2(slot2),
      m_slot3(slot3),
      m_slot4(slot4),
      m_slot5(slot5),
      m_slot6(slot6)
{
}

UIControl_ScoreboardButtonList::PendingSetSlotsText::~PendingSetSlotsText()
{
}

unsigned int UIControl_ScoreboardButtonList::PendingSetSlotsText::OnTick()
{
    m_list->setSlotsText(m_item, m_slot0, m_slot1, m_slot2, m_slot3, m_slot4, m_slot5, m_slot6, true);
    return 1;
}

bool UIControl_ScoreboardButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_ScoreboardButtonList;
    return UIControl_ButtonList::setupControl(scene, node, name);
}

void UIControl_ScoreboardButtonList::addItem(const std::wstring& label, int playerIcon, int voipIcon, bool immediate)
{
    if (immediate)
    {
        static_cast<FJ_List_ListButtonScoreBoard*>(m_fuiRenderNode->getFuiNode())
            ->addNewItem(label, m_itemIndex, playerIcon + 1, voipIcon + 1);
        addElement(UIControl::eUIControlType_Button, -1);
        return;
    }

    m_pendingUpdates.push_back(new PendingAddItem(this, 0, label, playerIcon, voipIcon));
}

void UIControl_ScoreboardButtonList::SetSlotTextureName(int item, int slot, const std::wstring& textureName,
                                                        bool immediate)
{
    if (immediate)
    {
        static_cast<FJ_List_ListButtonScoreBoard*>(m_fuiRenderNode->getFuiNode())
            ->SetSlotTextureName(item, slot, textureName);
        return;
    }

    m_pendingUpdates.push_back(new PendingSetSlotTextureName(this, item, slot, textureName));
}

void UIControl_ScoreboardButtonList::setSlotsText(int item, const std::wstring& slot0, const std::wstring& slot1,
                                                  const std::wstring& slot2, const std::wstring& slot3,
                                                  const std::wstring& slot4, const std::wstring& slot5,
                                                  const std::wstring& slot6, bool immediate)
{
    if (immediate)
    {
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 0, slot0);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 1, slot1);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 2, slot2);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 3, slot3);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 4, slot4);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 5, slot5);
        m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->SetSlotText(item, 6, slot6);
        return;
    }

    m_pendingUpdates.push_back(new PendingSetSlotsText(this, item, slot0, slot1, slot2, slot3, slot4, slot5, slot6));
}

void UIControl_ScoreboardButtonList::setPlayerIcon(int item, int icon)
{
    static_cast<FJ_List_ListButtonScoreBoard*>(m_fuiRenderNode->getFuiNode())->SetPlayerIcon(item, icon + 1);
}

void UIControl_ScoreboardButtonList::setVOIPIcon(int item, int icon)
{
    static_cast<FJ_List_ListButtonScoreBoard*>(m_fuiRenderNode->getFuiNode())->SetVOIPIcon(item, icon + 1);
}
