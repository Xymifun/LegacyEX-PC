#ifndef GAME_CORE_ATTRIBUTEMODIFIER_HPP
#define GAME_CORE_ATTRIBUTEMODIFIER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/HtmlString.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Player;

enum eMODIFIER_ID
{
    eMODIFIER_ID_None = 0,
    eMODIFIER_ID_BaseAttackDamage = 1,
    eMODIFIER_ID_AttackingSpeedBoost = 2,
    eMODIFIER_ID_SprintingSpeedBoost = 3,
    eMODIFIER_ID_EnderManAttackingSpeedBoost = 4,
    eMODIFIER_ID_PigZombieAttackingSpeedBoost = 5,
    eMODIFIER_ID_WitchDrinkingSpeedPenalty = 6,
    eMODIFIER_ID_BabyZombieSpeedBoost = 7,
    eMODIFIER_ID_PotionDamageBoost = 8,
    eMODIFIER_ID_PotionHealthBoost = 9,
    eMODIFIER_ID_PotionMoveSpeed = 10,
    eMODIFIER_ID_PotionMoveSlowdown = 11,
    eMODIFIER_ID_PotionWeakness = 12,
    eMODIFIER_ID_PotionDigSpeed = 13,
    eMODIFIER_ID_PotionLuck = 14,
    eMODIFIER_ID_BaseAttackSpeed = 16,
    eMODIFIER_ID_ArmorFeet = 17,
    eMODIFIER_ID_ArmorLegs = 18,
    eMODIFIER_ID_ArmorChest = 19,
    eMODIFIER_ID_ArmorHead = 20,
    eMODIFIER_ID_ArmorBonus = 21
};

class AttributeModifier
{
public:
    AttributeModifier(double amount, int operation);
    AttributeModifier(eMODIFIER_ID id, double amount, int operation);
    AttributeModifier(eMODIFIER_ID id, double amount, int operation, bool serialize);
    AttributeModifier(const AttributeModifier& other);

    eMODIFIER_ID getId();
    std::wstring getName();
    int getOperation();
    double getAmount();
    bool isSerializable();
    AttributeModifier* setSerialize(bool serialize);

    bool equals(AttributeModifier* other);
    std::wstring toString();

    HtmlString getHoverText(eATTRIBUTE_ID attributeId, not_null_ptr<ItemInstance> item,
                            boost::shared_ptr<Player> player);

private:
    void _init(eMODIFIER_ID id, std::wstring name, double amount, int operation);

    double amount;
    int operation;
    std::wstring name;
    eMODIFIER_ID id;
    bool serialize;
};

#endif
