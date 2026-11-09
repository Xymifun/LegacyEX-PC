#include "game/core/SetAttributesFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

not_null_ptr<ItemInstance> SetAttributesFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                        LootTableContext& context)
{
    if (!itemInstance || modifiers == NULL || modifiers->empty())
    {
        (void)context;
        return itemInstance;
    }

    (void)context;
    return itemInstance;
}
