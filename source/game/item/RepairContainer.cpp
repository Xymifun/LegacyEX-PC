#include "game/item/RepairContainer.hpp"

#include <string>

#include "game/ui/AnvilMenu.hpp"

RepairContainer::RepairContainer(AnvilMenu* menu, int id, bool hasCustomName, int size)
    : SimpleContainer(id, std::wstring(L""), hasCustomName, size),
      mMenu(menu)
{
}

RepairContainer::~RepairContainer()
{
}

void RepairContainer::setChanged()
{
    SimpleContainer::setChanged();
    mMenu->slotsChanged(shared_from_this());
}
