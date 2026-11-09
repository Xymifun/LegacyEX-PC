#include "game/entity/mob/HorseChest.hpp"

#include "game/core/StringIds.hpp"

HorseChest::HorseChest(const std::wstring& name, int size) : SimpleContainer(IDS_CONTAINER_ANIMAL, name, false, size)
{
}

HorseChest::HorseChest(int id, const std::wstring& name, bool hasCustomName, int size)
    : SimpleContainer(id, name, hasCustomName, size)
{
}

HorseChest::~HorseChest()
{
}
