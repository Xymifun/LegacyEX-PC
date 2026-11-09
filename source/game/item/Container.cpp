#include "game/item/Container.hpp"

#include "game/item/ItemInstance.hpp"

Container::Container()
{
}

bool Container::isFull()
{
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        if (getItem(slot)->isEmpty())
        {
            return false;
        }
    }

    return true;
}
