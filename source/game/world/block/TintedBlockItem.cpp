#include "game/world/block/TintedBlockItem.hpp"

#include "game/item/ItemInstance.hpp"

TintedBlockItem::TintedBlockItem(Block* block, bool stackedByData) : BlockItem(block)
{
    if (stackedByData)
    {
        setMaxDamage(0);
        setStackedByData(true);
    }
}

int TintedBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

TintedBlockItem* TintedBlockItem::setDescriptionPostfixes(arrayWithLength<int> postfixes)
{
    mDescriptionPostfixes = arrayWithLength<int>(postfixes.length, true);

    for (unsigned int i = 0; i < postfixes.length; ++i)
    {
        mDescriptionPostfixes[i] = postfixes[i];
    }

    return this;
}

int TintedBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();

    if (auxValue >= 0 && auxValue < (int)mDescriptionPostfixes.length)
    {
        return mDescriptionPostfixes.data[auxValue];
    }

    return BlockItem::getDescriptionId(itemInstance);
}

TextureAtlasSprite* TintedBlockItem::getIcon(int auxValue)
{
    return BlockItem::getIcon(auxValue);
}
