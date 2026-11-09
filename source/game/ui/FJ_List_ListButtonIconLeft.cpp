#include "game/ui/FJ_List_ListButtonIconLeft.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonIconLeft.hpp"

FJ_List_ListButtonIconLeft::FJ_List_ListButtonIconLeft(fuiRenderNode* renderNode)
    : FJ_List(renderNode),
      m_useLayerIcon(false),
      m_useIggyIcons(false)
{
}

void FJ_List_ListButtonIconLeft::UseIggyIcons()
{
    m_useIggyIcons = true;
}

void FJ_List_ListButtonIconLeft::addNewItem(const std::wstring& label, int id, const std::wstring& textureName,
                                            const std::wstring& textureName1)
{
    const char* objectName;
    if (m_checkboxTick)
        objectName = m_useLayerIcon ? "fourj.Buttons.FJ_ListButtonIconX2Left_Small"
                                    : "fourj.Buttons.FJ_ListButtonIconLeft_Small";
    else
        objectName = m_useLayerIcon ? "fourj.Buttons.FJ_ListButtonIconX2Left_Normal"
                                    : "fourj.Buttons.FJ_ListButtonIconLeft_Normal";

    FJ_ListButtonIconLeft* item = static_cast<FJ_ListButtonIconLeft*>(
        FJ_Factory::createObject(objectName, m_pRenderNode, true, NULL));
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetSlotID(m_id, m_useIggyIcons);

    if (m_useIggyIcons)
        item->SetTextureName(L"", textureName1);
    else
        item->SetTextureName(textureName, textureName1);
}

void FJ_List_ListButtonIconLeft::SetTextureName(int index, const std::wstring& textureName)
{
    FJ_ListButtonIconLeft* item = static_cast<FJ_ListButtonIconLeft*>(ReturnItem(index));
    if (item != NULL)
        item->SetTextureName(textureName, L"");
}

FJ_List* FJ_List_ListButtonIconLeft::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonIconLeft(renderNode);
}

uint32_t FJ_List_ListButtonIconLeft::getType()
{
    return 0x8032;
}
