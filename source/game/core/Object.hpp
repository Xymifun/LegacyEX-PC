#ifndef GAME_CORE_OBJECT_HPP
#define GAME_CORE_OBJECT_HPP

class ArmorItem;
class Block;
class Item;
class ItemInstance;

struct Object
{
    enum EType
    {
        eType_Item = 0x4000202,
        eType_ItemInstance = 0x4000203,
        eType_ArmorItem = 0x4000205,
        eType_Block = 0x4000206
    };

    Object(Item* item)
    {
        this->type = eType_Item;
        this->object = item;
    }

    Object(ItemInstance* itemInstance)
    {
        this->type = eType_ItemInstance;
        this->object = itemInstance;
    }

    Object(ArmorItem* armorItem)
    {
        this->type = eType_ArmorItem;
        this->object = armorItem;
    }

    Object(Block* block)
    {
        this->type = eType_Block;
        this->object = block;
    }

    void* object;
    int value;
    unsigned long long type;
};

#endif
