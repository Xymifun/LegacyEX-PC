#include "game/entity/mob/ItemInstanceEntityTagWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"

ItemInstanceEntityTagWalker::~ItemInstanceEntityTagWalker()
{
}

CompoundTag* ItemInstanceEntityTagWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(L"tag"))
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");
        if (itemTag->contains(L"EntityTag", Tag::TAG_COMPOUND))
        {
            CompoundTag* entityTag = itemTag->getCompound(L"EntityTag");
            std::wstring entityId;

            if (CheckID(tag, L"minecraft:armor_stand"))
            {
                entityId = (version < 515) ? L"ArmorStand" : L"minecraft:armor_stand";
            }
            else if (CheckID(tag, L"minecraft:spawn_egg"))
            {
                entityId = entityTag->getString(L"id");
            }
            else
            {
                return tag;
            }

            bool removeId;
            if (entityId.empty())
            {
                removeId = false;
            }
            else
            {
                removeId = !entityTag->contains(L"id", Tag::TAG_STRING);
                entityTag->putString(L"id", entityId);
            }

            fixer->update(DataFixTypes::eDataFixTypes_Entity, entityTag, version);

            if (removeId)
            {
                entityTag->remove(L"id");
            }
        }
    }

    return tag;
}
