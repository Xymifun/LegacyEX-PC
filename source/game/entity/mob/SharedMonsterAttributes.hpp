#ifndef GAME_ENTITY_MOB_SHAREDMONSTERATTRIBUTES_HPP
#define GAME_ENTITY_MOB_SHAREDMONSTERATTRIBUTES_HPP

#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

class SharedMonsterAttributes
{
public:
    static Attribute* MAX_HEALTH;
    static Attribute* FOLLOW_RANGE;
    static Attribute* KNOCKBACK_RESISTANCE;
    static Attribute* MOVEMENT_SPEED;
    static Attribute* FLYING_SPEED;
    static Attribute* ATTACK_DAMAGE;
    static Attribute* ATTACK_SPEED;
    static Attribute* ARMOR;
    static Attribute* ARMOR_TOUGHNESS;
    static Attribute* LUCK;

    static CompoundTag* saveAttributeModifier(AttributeModifier* modifier);
    static CompoundTag* saveAttribute(AttributeInstance* instance);
    static ListTag* saveAttributes(BaseAttributeMap* attributes);
    static AttributeModifier* loadAttributeModifier(CompoundTag* tag);
    static void loadAttribute(AttributeInstance* instance, CompoundTag* tag);
    static void loadAttributes(BaseAttributeMap* attributes, ListTag* tags);
};

#endif
