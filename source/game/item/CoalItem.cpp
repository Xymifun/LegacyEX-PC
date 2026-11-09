#include "game/item/CoalItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"

CoalItem::CoalItem() : Item()
{
    setStackedByData(true);
    setMaxDamage(0);
}

int CoalItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getAuxValue() == 1)
        return (int)IDS_ITEM_CHARCOAL;
    return (int)IDS_ITEM_COAL;
}

TextureAtlasSprite* CoalItem::getIcon(int auxValue)
{
    if (auxValue == 1)
        return mCharcoalIcon;
    return Item::getIcon(auxValue);
}

void CoalItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mCharcoalIcon = iconRegister->registerIcon(std::wstring(L"charcoal"));
}
