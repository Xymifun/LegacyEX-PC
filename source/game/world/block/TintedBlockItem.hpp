#ifndef GAME_WORLD_BLOCK_TINTEDBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_TINTEDBLOCKITEM_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockItem.hpp"

class Block;
class ItemInstance;
class TextureAtlasSprite;

class TintedBlockItem : public BlockItem
{
public:
    TintedBlockItem(Block* block, bool stackedByData);

    virtual int getLevelDataForAuxValue(int auxValue);
    TintedBlockItem* setDescriptionPostfixes(arrayWithLength<int> postfixes);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual TextureAtlasSprite* getIcon(int auxValue);

private:
    arrayWithLength<int> mDescriptionPostfixes;
};

#endif
