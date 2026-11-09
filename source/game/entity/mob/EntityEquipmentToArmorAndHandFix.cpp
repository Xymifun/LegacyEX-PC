#include "game/entity/mob/EntityEquipmentToArmorAndHandFix.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"

EntityEquipmentToArmorAndHandFix::~EntityEquipmentToArmorAndHandFix()
{
}

int EntityEquipmentToArmorAndHandFix::getVersion()
{
    return 100;
}

CompoundTag* EntityEquipmentToArmorAndHandFix::fix(CompoundTag* tag)
{
    ListTag* equipment = tag->getList(L"Equipment", Tag::TAG_COMPOUND);

    if (!equipment->isEmpty() && !tag->contains(L"HandItems", Tag::TAG_COMPOUND))
    {
        ListTag* handItems = new ListTag();

        handItems->add(equipment->get(0)->copy());
        handItems->add(new CompoundTag());

        tag->put(L"HandItems", handItems);
    }

    if (equipment->size() > 1 && !tag->contains(L"ArmorItem", Tag::TAG_COMPOUND))
    {
        ListTag* armorItems = new ListTag();

        armorItems->add(equipment->getCompound(1)->copy());
        armorItems->add(equipment->getCompound(2)->copy());
        armorItems->add(equipment->getCompound(3)->copy());
        armorItems->add(equipment->getCompound(4)->copy());

        tag->put(L"ArmorItems", armorItems);
    }

    delete equipment;

    tag->remove(L"Equipment");

    if (tag->contains(L"DropChances", Tag::TAG_LIST))
    {
        ListTag* dropChances = tag->getList(L"DropChances", Tag::TAG_FLOAT);

        if (!tag->contains(L"HandDropChances", Tag::TAG_COMPOUND))
        {
            ListTag* handDropChances = new ListTag();

            handDropChances->add(new FloatTag(dropChances->getFloat(0)));
            handDropChances->add(new FloatTag(0.0f));

            tag->put(L"HandDropChances", handDropChances);
        }

        if (!tag->contains(L"ArmorDropChances", Tag::TAG_COMPOUND))
        {
            ListTag* armorDropChances = new ListTag();

            armorDropChances->add(new FloatTag(dropChances->getFloat(1)));
            armorDropChances->add(new FloatTag(dropChances->getFloat(2)));
            armorDropChances->add(new FloatTag(dropChances->getFloat(3)));
            armorDropChances->add(new FloatTag(dropChances->getFloat(4)));

            tag->put(L"ArmorDropChances", armorDropChances);
        }

        delete dropChances;

        tag->remove(L"DropChances");
    }

    return tag;
}
