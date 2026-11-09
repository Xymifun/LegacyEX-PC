#include "game/item/EnchantRandomlyFunction.hpp"

#include <boost/unordered_map.hpp>

#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/MappedRegistry.hpp"

EnchantRandomlyFunction::EnchantRandomlyFunction(std::vector<LootItemCondition*>* conditions,
                                                 const std::vector<Enchantment*>& enchantments)
    : LootItemFunction(conditions)
{
    this->enchantments = enchantments;
}

not_null_ptr<ItemInstance> EnchantRandomlyFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                          Random* random, LootTableContext& context)
{
    not_null_ptr<ItemInstance> result = itemInstance;

    if (result->isEmpty())
    {
        return ItemInstance::EMPTY;
    }

    Enchantment* enchantment;

    if (enchantments.empty())
    {
        possibleEnchantments.clear();

        const boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>,
                                   std::equal_to<Enchantment*> >& registry =
            Enchantment::enchantmentRegistry->mInverseRegistry;

        for (boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>,
                                  std::equal_to<Enchantment*> >::const_iterator it = registry.begin();
             it != registry.end(); ++it)
        {
            if (result->getItem() == Items::book || (it->first != NULL && it->first->canEnchant(result)))
            {
                possibleEnchantments.push_back(it->first);
            }
        }

        if (possibleEnchantments.empty())
        {
            return result;
        }

        enchantment = possibleEnchantments.at(random->nextInt((int)possibleEnchantments.size()));
    }
    else
    {
        enchantment = enchantments.at(random->nextInt((int)enchantments.size()));
    }

    if (enchantment != NULL)
    {
        int level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());

        if (result->getItem() == Items::book)
        {
            result = not_null_ptr<ItemInstance>(new ItemInstance(Items::enchantedBook));
            EnchantedBookItem::addEnchantment(result, new EnchantmentInstance(enchantment, level));
        }
        else
        {
            result->enchant(enchantment, level);
        }
    }

    return result;
}
