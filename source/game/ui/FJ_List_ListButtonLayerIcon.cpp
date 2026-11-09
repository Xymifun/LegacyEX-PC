#include "game/ui/FJ_List_ListButtonLayerIcon.hpp"

#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonLayerIcon.hpp"

FJ_List_ListButtonLayerIcon::FJ_List_ListButtonLayerIcon(fuiRenderNode* renderNode)
    : FJ_List(renderNode),
      m_useIggyIcons(false)
{
}

FJ_List* FJ_List_ListButtonLayerIcon::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonLayerIcon(renderNode);
}

unsigned int FJ_List_ListButtonLayerIcon::getType()
{
    return 0x8038;
}

void FJ_List_ListButtonLayerIcon::UseIggyIcons()
{
    m_useIggyIcons = true;
}

void FJ_List_ListButtonLayerIcon::addNewItem(const std::wstring& label, const std::wstring& secondaryText, int id,
                                             const std::wstring& textureName)
{
    FJ_ListButtonLayerIcon* item;
    if (!m_checkboxTick)
    {
        item = static_cast<FJ_ListButtonLayerIcon*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonLayerIcon_Normal", m_pRenderNode, true, NULL));
    }
    else
    {
        item = static_cast<FJ_ListButtonLayerIcon*>(
            FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonLayerIcon_Small", m_pRenderNode, true, NULL));
    }

    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetSecondaryText(secondaryText, false, false);
    item->SetSlotID(m_id, m_useIggyIcons);

    std::wstring itemTextureName = m_useIggyIcons ? L"" : textureName;
    std::wstring layerName = L"";
    item->SetTextureName(itemTextureName, layerName);
}

void FJ_List_ListButtonLayerIcon::SetLayerLabel(int index, const std::wstring& label)
{
    FJ_Button* item = static_cast<FJ_Button*>(ReturnItem(index));
    if (item != NULL)
        item->SetSecondaryText(label, false, false);
}

void FJ_List_ListButtonLayerIcon::SetTextureName(int index, const std::wstring& textureName)
{
    FJ_ListButtonLayerIcon* item = static_cast<FJ_ListButtonLayerIcon*>(ReturnItem(index));
    if (item != NULL)
    {
        std::wstring layerName(L"");
        item->SetTextureName(textureName, layerName);
    }
}
