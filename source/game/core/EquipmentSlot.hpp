#ifndef GAME_CORE_EQUIPMENTSLOT_HPP
#define GAME_CORE_EQUIPMENTSLOT_HPP

#include <string>

class EquipmentSlot
{
public:
    enum EIndex
    {
        eIndex_MainHand,
        eIndex_OffHand,
        eIndex_Feet,
        eIndex_Legs,
        eIndex_Chest,
        eIndex_Head
    };

    enum Type
    {
        HAND,
        ARMOR
    };

    static const EquipmentSlot* MAINHAND;
    static const EquipmentSlot* OFFHAND;
    static const EquipmentSlot* FEET;
    static const EquipmentSlot* LEGS;
    static const EquipmentSlot* CHEST;
    static const EquipmentSlot* HEAD;

    static const EquipmentSlot* SLOTS[6];

    EquipmentSlot(EIndex eIndex, Type type, int index, int filterFlag, std::wstring name);

    static void staticCtor();

    EIndex getEIndex() const { return eIndex; }

    Type getType() const { return type; }

    int getIndex() const { return index; }

    int getFilterFlag() const { return filterFlag; }

    const std::wstring& getName() const { return name; }

private:
    EIndex eIndex;
    Type type;
    int index;
    int filterFlag;
    std::wstring name;
};

#endif
