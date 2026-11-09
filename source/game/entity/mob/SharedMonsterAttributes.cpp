#include "game/entity/mob/SharedMonsterAttributes.hpp"

#include <string>
#include <vector>

#include <boost/unordered_set.hpp>

#include "game/core/RangedAttribute.hpp"

Attribute* SharedMonsterAttributes::MAX_HEALTH =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_MaxHealth, 20.0, 0.0, 2014.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::FOLLOW_RANGE = new RangedAttribute(NULL, eATTRIBUTE_ID_FollowRange, 32.0, 0.0,
                                                                       2048.0);
Attribute* SharedMonsterAttributes::KNOCKBACK_RESISTANCE = new RangedAttribute(NULL, eATTRIBUTE_ID_KnockbackResistance,
                                                                               0.0, 0.0, 1.0);
Attribute* SharedMonsterAttributes::MOVEMENT_SPEED =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_MovementSpeed, 0.7f, 0.0, 1024.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::FLYING_SPEED =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_FlyingSpeed, 0.4f, 0.0, 1024.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::ATTACK_DAMAGE = new RangedAttribute(NULL, eATTRIBUTE_ID_AttackDamage, 2.0, 0.0,
                                                                        1024.0);
Attribute* SharedMonsterAttributes::ATTACK_SPEED =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_AttackSpeed, 4.0, 0.0, 1024.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::ARMOR =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_Armor, 0.0, 0.0, 30.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::ARMOR_TOUGHNESS =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_ArmorToughness, 0.0, 0.0, 20.0))->setSyncable(true);
Attribute* SharedMonsterAttributes::LUCK =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_Luck, 0.0, -1024.0, 1024.0))->setSyncable(true);

CompoundTag* SharedMonsterAttributes::saveAttributeModifier(AttributeModifier* modifier)
{
    CompoundTag* tag = new CompoundTag();

    tag->putDouble(std::wstring(L"Amount"), modifier->getAmount());
    tag->putInt(std::wstring(L"Operation"), modifier->getOperation());
    tag->putInt(std::wstring(L"UUID"), (int)modifier->getId());

    return tag;
}

CompoundTag* SharedMonsterAttributes::saveAttribute(AttributeInstance* instance)
{
    CompoundTag* tag = new CompoundTag();

    tag->putInt(std::wstring(L"ID"), (int)instance->getAttribute()->getId());
    tag->putDouble(std::wstring(L"Base"), instance->getBaseValue());

    boost::unordered_set<AttributeModifier*> modifiers;
    instance->getModifiers(modifiers);

    if (!modifiers.empty())
    {
        ListTag* list = new ListTag();

        for (boost::unordered_set<AttributeModifier*>::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
        {
            AttributeModifier* modifier = *it;

            if (modifier->isSerializable())
            {
                list->add(saveAttributeModifier(modifier));
            }
        }

        tag->put(std::wstring(L"Modifiers"), list);
    }

    return tag;
}

ListTag* SharedMonsterAttributes::saveAttributes(BaseAttributeMap* attributes)
{
    ListTag* tag = new ListTag();

    std::vector<AttributeInstance*> instances;
    attributes->getAttributes(instances);

    for (std::vector<AttributeInstance*>::iterator it = instances.begin(); it != instances.end(); ++it)
    {
        tag->add(saveAttribute(*it));
    }

    return tag;
}

AttributeModifier* SharedMonsterAttributes::loadAttributeModifier(CompoundTag* tag)
{
    eMODIFIER_ID id = (eMODIFIER_ID)tag->getInt(std::wstring(L"UUID"));

    return new AttributeModifier(id, tag->getDouble(std::wstring(L"Amount")), tag->getInt(std::wstring(L"Operation")));
}

void SharedMonsterAttributes::loadAttribute(AttributeInstance* instance, CompoundTag* tag)
{
    instance->setBaseValue(tag->getDouble(std::wstring(L"Base")));

    if (tag->contains(std::wstring(L"Modifiers"), 9))
    {
        ListTag* modifiers = tag->getList(std::wstring(L"Modifiers"), 10);

        for (int i = 0; i < modifiers->size(); ++i)
        {
            AttributeModifier* modifier = loadAttributeModifier(modifiers->getCompound(i));

            if (modifier != NULL)
            {
                AttributeModifier* existing = instance->getModifier(modifier->getId());

                if (existing != NULL)
                {
                    instance->removeModifier(existing);
                }

                instance->addModifier(modifier);
            }
        }
    }
}

void SharedMonsterAttributes::loadAttributes(BaseAttributeMap* attributes, ListTag* tags)
{
    for (int i = 0; i < tags->size(); ++i)
    {
        CompoundTag* tag = tags->getCompound(i);

        AttributeInstance* instance = attributes->getInstance((eATTRIBUTE_ID)tag->getInt(std::wstring(L"ID")));

        if (instance != NULL)
        {
            loadAttribute(instance, tag);
        }
    }
}
