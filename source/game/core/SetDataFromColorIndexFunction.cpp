#include "game/core/SetDataFromColorIndexFunction.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/LootTableContext.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/DyeColor.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"

SetDataFromColorIndexFunction::SetDataFromColorIndexFunction(std::vector<LootItemCondition*>* conditions)
    : LootItemFunction(conditions)
{
}

not_null_ptr<ItemInstance> SetDataFromColorIndexFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                                Random* random, LootTableContext& context)
{
    if (!itemInstance->isDamageableItem())
    {
        if (eTYPE_DERIVED_FROM(EntityType::SHEEP, context.mThisEntity.lock()->GetType()))
        {
            boost::shared_ptr<Sheep> sheep = boost::static_pointer_cast<Sheep>(context.mThisEntity.lock());

            int data = sheep->getColor()->getBlockData();
            itemInstance->setAuxValue(data);

            if (sheep->isSheared())
            {
                itemInstance->setCount(0);
            }
        }
    }

    return itemInstance;
}
