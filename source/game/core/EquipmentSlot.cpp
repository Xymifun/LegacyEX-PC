#include "game/core/EquipmentSlot.hpp"

const EquipmentSlot* EquipmentSlot::MAINHAND;
const EquipmentSlot* EquipmentSlot::OFFHAND;
const EquipmentSlot* EquipmentSlot::FEET;
const EquipmentSlot* EquipmentSlot::LEGS;
const EquipmentSlot* EquipmentSlot::CHEST;
const EquipmentSlot* EquipmentSlot::HEAD;

const EquipmentSlot* EquipmentSlot::SLOTS[6];

EquipmentSlot::EquipmentSlot(EIndex eIndex, Type type, int index, int filterFlag, std::wstring name)
    : eIndex(eIndex),
      type(type),
      index(index),
      filterFlag(filterFlag),
      name(name)
{
}

void EquipmentSlot::staticCtor()
{
    MAINHAND = new EquipmentSlot(eIndex_MainHand, HAND, 0, 0, L"mainhand");
    OFFHAND = new EquipmentSlot(eIndex_OffHand, HAND, 1, 5, L"offhand");
    FEET = new EquipmentSlot(eIndex_Feet, ARMOR, 0, 1, L"feet");
    LEGS = new EquipmentSlot(eIndex_Legs, ARMOR, 1, 2, L"legs");
    CHEST = new EquipmentSlot(eIndex_Chest, ARMOR, 2, 3, L"chest");
    HEAD = new EquipmentSlot(eIndex_Head, ARMOR, 3, 4, L"head");

    SLOTS[eIndex_MainHand] = MAINHAND;
    SLOTS[eIndex_OffHand] = OFFHAND;
    SLOTS[eIndex_Feet] = FEET;
    SLOTS[eIndex_Legs] = LEGS;
    SLOTS[eIndex_Chest] = CHEST;
    SLOTS[eIndex_Head] = HEAD;
}
