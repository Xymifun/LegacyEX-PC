#include "game/item/PotionBrewing.hpp"

#include "game/item/FishFoodItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"

std::vector<PotionBrewing::Mix<Potion>*> PotionBrewing::potionMixes;
std::vector<PotionBrewing::Mix<Item>*> PotionBrewing::containerMixes;
std::vector<PotionBrewing::ItemInstancePredicate*> PotionBrewing::containerPredicates;
PotionBrewing::PredicateItemInstance* PotionBrewing::containerPredicate = new PotionBrewing::PredicateItemInstance();

PotionBrewing::ItemInstancePredicate::ItemInstancePredicate(Item* item) : mItem(item), mAuxValue(-1)
{
}

PotionBrewing::ItemInstancePredicate::ItemInstancePredicate(Item* item, int auxValue) : mItem(item), mAuxValue(auxValue)
{
}

bool PotionBrewing::ItemInstancePredicate::apply(not_null_ptr<ItemInstance> itemInstance) const
{
    return itemInstance->getItem() == mItem && (mAuxValue == -1 || mAuxValue == itemInstance->getAuxValue());
}

PotionBrewing::ItemInstancePredicate::~ItemInstancePredicate()
{
}

bool PotionBrewing::PredicateItemInstance::apply(not_null_ptr<ItemInstance> itemInstance) const
{
    for (std::vector<PotionBrewing::ItemInstancePredicate*>::iterator it = containerPredicates.begin();
         it != containerPredicates.end(); ++it)
    {
        ItemInstancePredicate* predicate = *it;
        if (predicate->apply(itemInstance))
        {
            return true;
        }
    }
    return false;
}

PotionBrewing::PredicateItemInstance::~PredicateItemInstance()
{
}

bool PotionBrewing::isContainerIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    int i = 0;
    int count = containerMixes.size();
    for (; i < count; ++i)
    {
        if (containerMixes.at(i)->ingredient->apply(itemInstance))
        {
            return true;
        }
    }
    return false;
}

bool PotionBrewing::isPotionIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    int i = 0;
    int count = potionMixes.size();
    for (; i < count; ++i)
    {
        if (potionMixes.at(i)->ingredient->apply(itemInstance))
        {
            return true;
        }
    }
    return false;
}

bool PotionBrewing::isIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    return isContainerIngredient(itemInstance) || isPotionIngredient(itemInstance);
}

bool PotionBrewing::hasContainerMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    Item* itemType = item->getItem();
    int i = 0;
    int count = containerMixes.size();
    for (; i < count; ++i)
    {
        Mix<Item>* containerMix = containerMixes.at(i);
        if (containerMix->from == itemType && containerMix->ingredient->apply(ingredient))
        {
            return true;
        }
    }
    return false;
}

bool PotionBrewing::hasPotionMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    Potion* potion = PotionUtils::getPotion(item);
    int i = 0;
    int count = potionMixes.size();
    for (; i < count; ++i)
    {
        Mix<Potion>* potionMix = potionMixes.at(i);
        if (potionMix->from == potion && potionMix->ingredient->apply(ingredient))
        {
            return true;
        }
    }
    return false;
}

bool PotionBrewing::hasMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    if (!containerPredicate->apply(item))
    {
        return false;
    }
    return hasContainerMix(item, ingredient) || hasPotionMix(item, ingredient);
}

not_null_ptr<ItemInstance> PotionBrewing::mix(not_null_ptr<ItemInstance> ingredient, not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        Potion* potion = PotionUtils::getPotion(item);
        Item* itemType = item->getItem();
        int i = 0;
        int count = containerMixes.size();
        for (; i < count; ++i)
        {
            Mix<Item>* containerMix = containerMixes.at(i);
            if (containerMix->from == itemType && containerMix->ingredient->apply(ingredient))
            {
                return PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(containerMix->result)),
                                              potion);
            }
        }
        i = 0;
        count = potionMixes.size();
        for (; i < count; ++i)
        {
            Mix<Potion>* potionMix = potionMixes.at(i);
            if (potionMix->from == potion && potionMix->ingredient->apply(ingredient))
            {
                return PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(itemType)),
                                              potionMix->result);
            }
        }
    }
    return item;
}

void PotionBrewing::addContainer(ItemInstancePredicate* predicate)
{
    containerPredicates.push_back(predicate);
}

void PotionBrewing::addContainerRecipe(PotionItem* from, ItemInstancePredicate* ingredient, PotionItem* result)
{
    Mix<Item>* containerMix = new Mix<Item>(from, ingredient, result);
    containerMixes.push_back(containerMix);
}

void PotionBrewing::addMix(Potion* from, Predicate<not_null_ptr<ItemInstance> >* ingredient, Potion* result)
{
    Mix<Potion>* potionMix = new Mix<Potion>(from, ingredient, result);
    potionMixes.push_back(potionMix);
}

void PotionBrewing::StaticCtor()
{
    ItemInstancePredicate* netherWart = new ItemInstancePredicate(Items::netherWart);
    ItemInstancePredicate* goldenCarrot = new ItemInstancePredicate(Items::goldenCarrot);
    ItemInstancePredicate* redstone = new ItemInstancePredicate(Items::redstone);
    ItemInstancePredicate* fermentedSpiderEye = new ItemInstancePredicate(Items::fermentedSpiderEye);
    ItemInstancePredicate* rabbitFoot = new ItemInstancePredicate(Items::rabbitFoot);
    ItemInstancePredicate* glowstoneDust = new ItemInstancePredicate(Items::glowstoneDust);
    ItemInstancePredicate* magmaCream = new ItemInstancePredicate(Items::magmaCream);
    ItemInstancePredicate* sugar = new ItemInstancePredicate(Items::sugar);
    ItemInstancePredicate* pufferfish = new ItemInstancePredicate(Items::fish, FishFoodItem::Type::PUFFERFISH->getId());
    ItemInstancePredicate* speckledMelon = new ItemInstancePredicate(Items::speckledMelon);
    ItemInstancePredicate* spiderEye = new ItemInstancePredicate(Items::spiderEye);
    ItemInstancePredicate* ghastTear = new ItemInstancePredicate(Items::ghastTear);
    ItemInstancePredicate* blazePowder = new ItemInstancePredicate(Items::blazePowder);
    ItemInstancePredicate* phantomMembrane = new ItemInstancePredicate(Items::phantomMembrane);
    ItemInstancePredicate* turtleHelmet = new ItemInstancePredicate(Items::turtleHelmet);

    addContainer(new ItemInstancePredicate(Items::potion));
    addContainer(new ItemInstancePredicate(Items::splashPotion));
    addContainer(new ItemInstancePredicate(Items::lingeringPotion));

    addContainerRecipe(static_cast<PotionItem*>(Items::potion), new ItemInstancePredicate(Items::gunpowder),
                       static_cast<PotionItem*>(Items::splashPotion));
    addContainerRecipe(static_cast<PotionItem*>(Items::splashPotion), new ItemInstancePredicate(Items::dragonBreath),
                       static_cast<PotionItem*>(Items::lingeringPotion));

    addMix(Potions::WATER, speckledMelon, Potions::MUNDANE);
    addMix(Potions::WATER, ghastTear, Potions::MUNDANE);
    addMix(Potions::WATER, rabbitFoot, Potions::MUNDANE);
    addMix(Potions::WATER, blazePowder, Potions::MUNDANE);
    addMix(Potions::WATER, spiderEye, Potions::MUNDANE);
    addMix(Potions::WATER, sugar, Potions::MUNDANE);
    addMix(Potions::WATER, magmaCream, Potions::MUNDANE);
    addMix(Potions::WATER, glowstoneDust, Potions::THICK);
    addMix(Potions::WATER, redstone, Potions::MUNDANE);
    addMix(Potions::WATER, netherWart, Potions::AWKWARD);
    addMix(Potions::AWKWARD, goldenCarrot, Potions::NIGHT_VISION);
    addMix(Potions::NIGHT_VISION, redstone, Potions::LONG_NIGHT_VISION);
    addMix(Potions::NIGHT_VISION, fermentedSpiderEye, Potions::INVISIBILITY);
    addMix(Potions::LONG_NIGHT_VISION, fermentedSpiderEye, Potions::LONG_INVISIBILITY);
    addMix(Potions::INVISIBILITY, redstone, Potions::LONG_INVISIBILITY);
    addMix(Potions::AWKWARD, magmaCream, Potions::FIRE_RESISTANCE);
    addMix(Potions::FIRE_RESISTANCE, redstone, Potions::LONG_FIRE_RESISTANCE);
    addMix(Potions::AWKWARD, rabbitFoot, Potions::LEAPING);
    addMix(Potions::LEAPING, redstone, Potions::LONG_LEAPING);
    addMix(Potions::LEAPING, glowstoneDust, Potions::STRONG_LEAPING);
    addMix(Potions::LEAPING, fermentedSpiderEye, Potions::SLOWNESS);
    addMix(Potions::LONG_LEAPING, fermentedSpiderEye, Potions::LONG_SLOWNESS);
    addMix(Potions::SLOWNESS, redstone, Potions::LONG_SLOWNESS);
    addMix(Potions::AWKWARD, turtleHelmet, Potions::TURTLE_MASTER);
    addMix(Potions::TURTLE_MASTER, redstone, Potions::LONG_TURTLE_MASTER);
    addMix(Potions::TURTLE_MASTER, glowstoneDust, Potions::STRONG_TURTLE_MASTER);
    addMix(Potions::SWIFTNESS, fermentedSpiderEye, Potions::SLOWNESS);
    addMix(Potions::LONG_SWIFTNESS, fermentedSpiderEye, Potions::LONG_SLOWNESS);
    addMix(Potions::AWKWARD, sugar, Potions::SWIFTNESS);
    addMix(Potions::SWIFTNESS, redstone, Potions::LONG_SWIFTNESS);
    addMix(Potions::SWIFTNESS, glowstoneDust, Potions::STRONG_SWIFTNESS);
    addMix(Potions::AWKWARD, pufferfish, Potions::WATER_BREATHING);
    addMix(Potions::WATER_BREATHING, redstone, Potions::LONG_WATER_BREATHING);
    addMix(Potions::AWKWARD, speckledMelon, Potions::HEALING);
    addMix(Potions::HEALING, glowstoneDust, Potions::STRONG_HEALING);
    addMix(Potions::HEALING, fermentedSpiderEye, Potions::HARMING);
    addMix(Potions::STRONG_HEALING, fermentedSpiderEye, Potions::STRONG_HARMING);
    addMix(Potions::HARMING, glowstoneDust, Potions::STRONG_HARMING);
    addMix(Potions::POISON, fermentedSpiderEye, Potions::HARMING);
    addMix(Potions::LONG_POISON, fermentedSpiderEye, Potions::HARMING);
    addMix(Potions::STRONG_POISON, fermentedSpiderEye, Potions::STRONG_HARMING);
    addMix(Potions::AWKWARD, spiderEye, Potions::POISON);
    addMix(Potions::POISON, redstone, Potions::LONG_POISON);
    addMix(Potions::POISON, glowstoneDust, Potions::STRONG_POISON);
    addMix(Potions::AWKWARD, ghastTear, Potions::REGENERATION);
    addMix(Potions::REGENERATION, redstone, Potions::LONG_REGENERATION);
    addMix(Potions::REGENERATION, glowstoneDust, Potions::STRONG_REGENERATION);
    addMix(Potions::AWKWARD, blazePowder, Potions::STRENGTH);
    addMix(Potions::STRENGTH, redstone, Potions::LONG_STRENGTH);
    addMix(Potions::STRENGTH, glowstoneDust, Potions::STRONG_STRENGTH);
    addMix(Potions::WATER, fermentedSpiderEye, Potions::WEAKNESS);
    addMix(Potions::WEAKNESS, redstone, Potions::LONG_WEAKNESS);
    addMix(Potions::AWKWARD, phantomMembrane, Potions::SLOW_FALLING);
    addMix(Potions::SLOW_FALLING, redstone, Potions::LONG_SLOW_FALLING);
}
