#include "game/world/block/MultiBlockItem.hpp"

MultiBlockItem::MultiBlockItem(Block* block, Block* blockForAuxValue, ItemInstanceResolver nameFunction,
                               ItemInstanceResolver descriptionFunction)
    : BlockItem(block),
      mBlockForAuxValue(blockForAuxValue),
      mNameFunction(nameFunction),
      mDescriptionFunction(descriptionFunction)
{
    setMaxDamage(0);
    setStackedByData(true);
}

int MultiBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int MultiBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mNameFunction == NULL ? BlockItem::getDescriptionId(itemInstance) : mNameFunction(itemInstance);
}

int MultiBlockItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mDescriptionFunction == NULL ? BlockItem::getUseDescriptionId(itemInstance)
                                        : mDescriptionFunction(itemInstance);
}
