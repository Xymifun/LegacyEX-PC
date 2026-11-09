#include "game/item/EnchantmentContainer.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/EnchantmentMenu.hpp"

EnchantmentContainer::EnchantmentContainer(EnchantmentMenu* menu)
    : SimpleContainer(IDS_ENCHANT, std::wstring(L""), false, 2),
      mMenu(menu)
{
}

EnchantmentContainer::~EnchantmentContainer()
{
}

int EnchantmentContainer::getMaxStackSize()
{
    return 64;
}

void EnchantmentContainer::setChanged()
{
    SimpleContainer::setChanged();
    mMenu->slotsChanged();
}
