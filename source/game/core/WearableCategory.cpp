#include "game/core/WearableCategory.hpp"

#include "game/item/ArmorItem.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/SkullItem.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/PumpkinBlock.hpp"

bool WearableCategory::canEnchant(Item* item) const
{
    bool isPumpkin = dynamic_cast<BlockItem*>(item) != NULL
                     && dynamic_cast<PumpkinBlock*>(dynamic_cast<BlockItem*>(item)->getBlock()) != NULL;

    return dynamic_cast<ArmorItem*>(item) != NULL || dynamic_cast<ElytraItem*>(item) != NULL
           || dynamic_cast<SkullItem*>(item) != NULL || isPumpkin;
}
