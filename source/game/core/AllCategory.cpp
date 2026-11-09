#include "game/core/AllCategory.hpp"

#include "game/item/Item.hpp"

bool AllCategory::canEnchant(Item* item) const
{
    for (std::vector<const EnchantmentCategory*>::const_iterator it = CATEGORIES.begin(); it != CATEGORIES.end(); ++it)
    {
        const EnchantmentCategory* category = *it;
        if (category != ALL && category->canEnchant(item))
        {
            return true;
        }
    }

    return false;
}
