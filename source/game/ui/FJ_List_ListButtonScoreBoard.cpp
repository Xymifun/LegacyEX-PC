#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonScoreBoard.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_List_ListButtonScoreBoard::FJ_List_ListButtonScoreBoard(fuiRenderNode* renderNode)
    : FJ_List(renderNode),
      m_numColumns(0)
{
    m_buttonHeight = static_cast<int>(getRenderNodeParent()->getWidth());
}

FJ_List* FJ_List_ListButtonScoreBoard::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonScoreBoard(renderNode);
}

uint32_t FJ_List_ListButtonScoreBoard::getType()
{
    return 0x803b;
}

void FJ_List_ListButtonScoreBoard::SetNumColumns(int numColumns)
{
    m_numColumns = numColumns;
}

void FJ_List_ListButtonScoreBoard::addNewItem(const std::wstring& label, int id, int playerIcon, int voipIcon)
{
    FJ_ListButtonScoreBoard* item;
    if (m_checkboxTick)
    {
        item = static_cast<FJ_ListButtonScoreBoard*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonScoreBoard_Small", m_pRenderNode, true, NULL));
    }
    else
    {
        item = static_cast<FJ_ListButtonScoreBoard*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonScoreBoard_Normal", m_pRenderNode, true, NULL));
    }

    addNewListItem(item, label, id);
    item->SetNumIcons(m_numColumns, m_buttonHeight, static_cast<int>(getX()));
    item->Init(label, id);
    item->SetPlayerIcon(playerIcon);
    item->SetVOIPIcon(voipIcon);
}

void FJ_List_ListButtonScoreBoard::SetPlayerIcon(int item, int icon)
{
    FJ_ListButtonScoreBoard* button = static_cast<FJ_ListButtonScoreBoard*>(ReturnItem(item));
    if (button != NULL)
        button->SetPlayerIcon(icon);
}

void FJ_List_ListButtonScoreBoard::SetVOIPIcon(int item, int icon)
{
    FJ_ListButtonScoreBoard* button = static_cast<FJ_ListButtonScoreBoard*>(ReturnItem(item));
    if (button != NULL)
        button->SetVOIPIcon(icon);
}

void FJ_List_ListButtonScoreBoard::SetSlotText(int item, int slot, const std::wstring& text)
{
    FJ_ListButtonScoreBoard* button = static_cast<FJ_ListButtonScoreBoard*>(ReturnItem(item));
    if (button != NULL)
        button->SetSlotText(slot, text);
}

void FJ_List_ListButtonScoreBoard::SetSlotTextureName(int item, int slot, const std::wstring& textureName)
{
    FJ_ListButtonScoreBoard* button = static_cast<FJ_ListButtonScoreBoard*>(ReturnItem(item));
    if (button != NULL)
        button->SetSlotTextureName(slot, textureName);
}

int FJ_List_ListButtonScoreBoard::GetButtonHeight()
{
    FJ_ListButtonScoreBoard* button;
    if (m_checkboxTick)
    {
        button = static_cast<FJ_ListButtonScoreBoard*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonScoreBoard_Small", m_pRenderNode, true, NULL));
    }
    else
    {
        button = static_cast<FJ_ListButtonScoreBoard*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonScoreBoard_Normal", m_pRenderNode, true, NULL));
    }

    int height = static_cast<int>(button->getHeight());
    if (button != NULL)
        delete button;
    return height;
}
