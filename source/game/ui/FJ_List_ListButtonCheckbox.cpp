#include "game/ui/FJ_List_ListButtonCheckbox.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonCheckbox.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_List_ListButtonCheckbox::FJ_List_ListButtonCheckbox(fuiRenderNode* renderNode) : FJ_List(renderNode)
{
    m_useIggyIcons = false;
}

FJ_List* FJ_List_ListButtonCheckbox::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonCheckbox(renderNode);
}

uint32_t FJ_List_ListButtonCheckbox::getType()
{
    return 0x8036;
}

void FJ_List_ListButtonCheckbox::UseIggyIcons()
{
    m_useIggyIcons = true;
}

void FJ_List_ListButtonCheckbox::addNewItem(const std::wstring& label, int id, bool checked,
                                            const std::wstring& textureName)
{
    FJ_ListButtonCheckbox* item;
    if (m_checkboxTick)
        item = static_cast<FJ_ListButtonCheckbox*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonCheckbox_Small", m_pRenderNode, true, NULL));
    else
        item = static_cast<FJ_ListButtonCheckbox*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonCheckbox_Normal", m_pRenderNode, true, NULL));

    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetSlotID(m_id, m_useIggyIcons);

    item->SetTextureName(m_useIggyIcons ? L"" : textureName);

    item->SetCheckBox(checked);
}

void FJ_List_ListButtonCheckbox::SetCheckBox(int index, bool checked)
{
    FJ_ListButtonCheckbox* item = static_cast<FJ_ListButtonCheckbox*>(ReturnItem(index));
    if (item != NULL)
        item->SetCheckBox(checked);
}

uint8_t FJ_List_ListButtonCheckbox::GetCheckBox(int index)
{
    FJ_ListButtonCheckbox* item = static_cast<FJ_ListButtonCheckbox*>(ReturnItem(index));
    return item != NULL ? item->GetCheckBox() : 0;
}
