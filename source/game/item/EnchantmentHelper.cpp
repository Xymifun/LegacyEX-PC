#include "game/item/EnchantmentHelper.hpp"
#include "game/core/EntityType.hpp"

#include <utility>
#include <vector>

#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/WeighedRandom.hpp"
#include "game/world/block/MappedRegistry.hpp"

static EnchantmentHelper::GetDamageProtectionIteration sDamageProtectionIteration;
static EnchantmentHelper::GetDamageBonusIteration sDamageBonusIteration;
static EnchantmentHelper::DoPostHurtIteration sPostHurtIteration;
static EnchantmentHelper::DoPostDamageIteration sPostDamageIteration;

void EnchantmentHelper::GetDamageProtectionIteration::doEnchantment(Enchantment* enchantment, int level)
{
    mProtection += enchantment->getDamageProtection(level, mSource);
}

void EnchantmentHelper::GetDamageBonusIteration::doEnchantment(Enchantment* enchantment, int level)
{
    mBonus += enchantment->getDamageBonus(level, mTarget);
}

void EnchantmentHelper::DoPostHurtIteration::doEnchantment(Enchantment* enchantment, int level)
{
    enchantment->doPostHurt(mTarget, mAttacker, level);
}

void EnchantmentHelper::DoPostDamageIteration::doEnchantment(Enchantment* enchantment, int level)
{
    enchantment->doPostAttack(mAttacker, mTarget, level);
}

int EnchantmentHelper::getItemEnchantmentLevel(const Enchantment* enchantment, const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
    {
        return 0;
    }

    ListTag* tags = item->getEnchantmentTags();
    if (tags == NULL)
    {
        return 0;
    }

    for (int index = 0; index < tags->size(); ++index)
    {
        CompoundTag* tag = tags->getCompound(index);
        Enchantment* stored = Enchantment::byId(tag->getShort(L"id"));
        int level = tag->getShort(L"lvl");

        if (stored == enchantment)
        {
            return level;
        }
    }

    return 0;
}

boost::unordered_map<Enchantment*, int>* EnchantmentHelper::getEnchantments(const not_null_ptr<ItemInstance>& item)
{
    boost::unordered_map<Enchantment*, int>* enchantments = new boost::unordered_map<Enchantment*, int>();

    ListTag* tags;
    if (item->getItem() == Items::enchantedBook)
    {
        tags = EnchantedBookItem::getEnchantments(item);
    }
    else
    {
        tags = item->getEnchantmentTags();
    }

    if (tags != NULL)
    {
        for (int index = 0; index < tags->size(); ++index)
        {
            CompoundTag* tag = tags->getCompound(index);
            Enchantment* enchantment = Enchantment::byId(tag->getShort(L"id"));
            int level = tag->getShort(L"lvl");

            enchantments->insert(std::make_pair(enchantment, level));
        }
    }

    return enchantments;
}

void EnchantmentHelper::setEnchantments(boost::unordered_map<Enchantment*, int>* enchantments,
                                        const not_null_ptr<ItemInstance>& item)
{
    ListTag* tags = new ListTag();

    boost::unordered_map<Enchantment*, int>::const_iterator it;
    for (it = enchantments->begin(); it != enchantments->end(); ++it)
    {
        Enchantment* enchantment = it->first;
        if (enchantment == NULL)
        {
            continue;
        }

        int level = it->second;

        CompoundTag* tag = new CompoundTag();
        tag->putShort(L"id", static_cast<short>(Enchantment::getId(enchantment)));
        tag->putShort(L"lvl", static_cast<short>(level));
        tags->add(tag);

        if (item->getItem() == Items::enchantedBook)
        {
            EnchantedBookItem::addEnchantment(item, new EnchantmentInstance(enchantment, level));
        }
    }

    if (!tags->isEmpty())
    {
        if (item->getItem() != Items::enchantedBook)
        {
            item->addTagElement(L"ench", tags);
        }
    }
    else if (item->hasTag())
    {
        item->getTag()->remove(L"ench");
    }
}

int EnchantmentHelper::getEnchantmentLevel(Enchantment* enchantment, const boost::shared_ptr<LivingEntity>& entity)
{
    if (entity.get() == NULL)
    {
        return 0;
    }

    std::vector<boost::weak_ptr<ItemInstance> > items = enchantment->getSlotItems(entity);
    if (items.empty())
    {
        return 0;
    }

    int level = 0;
    for (std::vector<boost::weak_ptr<ItemInstance> >::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        int itemLevel = getItemEnchantmentLevel(enchantment, not_null_ptr<ItemInstance>(it->lock()));
        if (level < itemLevel)
        {
            level = itemLevel;
        }
    }

    return level;
}

void EnchantmentHelper::runIterationOnItem(EnchantmentIterationMethod& method, const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
    {
        return;
    }

    ListTag* tags = item->getEnchantmentTags();
    if (tags == NULL)
    {
        return;
    }

    for (int index = 0; index < tags->size(); ++index)
    {
        int id = tags->getCompound(index)->getShort(L"id");
        int level = tags->getCompound(index)->getShort(L"lvl");

        if (Enchantment::byId(id) != NULL)
        {
            method.doEnchantment(Enchantment::byId(id), level);
        }
    }
}

void EnchantmentHelper::runIterationOnInventory(EnchantmentIterationMethod& method,
                                                NonNullList<not_null_ptr<ItemInstance> > items)
{
    for (unsigned int index = 0; index < items.size(); ++index)
    {
        runIterationOnItem(method, items[index]);
    }
}

int EnchantmentHelper::getDamageProtection(NonNullList<not_null_ptr<ItemInstance> > items, const DamageSource* source)
{
    sDamageProtectionIteration.mProtection = 0;
    sDamageProtectionIteration.mSource = source;

    runIterationOnInventory(sDamageProtectionIteration, items);

    return sDamageProtectionIteration.mProtection;
}

float EnchantmentHelper::getDamageBonus(const not_null_ptr<ItemInstance>& item,
                                        const boost::shared_ptr<LivingEntity>& entity)
{
    sDamageBonusIteration.mBonus = 0.0f;
    sDamageBonusIteration.mTarget = entity;

    runIterationOnItem(sDamageBonusIteration, item);

    return sDamageBonusIteration.mBonus;
}

void EnchantmentHelper::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& target,
                                          const boost::shared_ptr<Entity>& attacker)
{
    sPostHurtIteration.mAttacker = attacker;
    sPostHurtIteration.mTarget = target;

    if (target.get() != NULL)
    {
        NonNullList<not_null_ptr<ItemInstance> > slots = target->getAllSlots();
        runIterationOnInventory(sPostHurtIteration, slots);
    }

    if (attacker.get() != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
    {
        runIterationOnItem(sPostHurtIteration, target->getMainHandItem());
    }
}

void EnchantmentHelper::doPostDamageEffects(const boost::shared_ptr<LivingEntity>& attacker,
                                            const boost::shared_ptr<Entity>& target)
{
    sPostDamageIteration.mAttacker = attacker;
    sPostDamageIteration.mTarget = target;

    if (attacker.get() != NULL)
    {
        NonNullList<not_null_ptr<ItemInstance> > slots = target->getAllSlots();
        runIterationOnInventory(sPostDamageIteration, slots);
    }

    if (attacker.get() != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
    {
        runIterationOnItem(sPostDamageIteration, attacker->getMainHandItem());
    }
}

int EnchantmentHelper::getKnockbackBonus(const boost::shared_ptr<LivingEntity>& attacker,
                                         const boost::shared_ptr<LivingEntity>& target)
{
    return getEnchantmentLevel(Enchantments::KNOCKBACK, attacker);
}

int EnchantmentHelper::getFireAspect(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::FIRE_ASPECT, entity);
}

int EnchantmentHelper::getRespiration(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::RESPIRATION, entity);
}

int EnchantmentHelper::getDepthStrider(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::DEPTH_STRIDER, entity);
}

int EnchantmentHelper::getBlockEfficiency(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::BLOCK_EFFICIENCY, entity);
}

int EnchantmentHelper::getFishingLuckBonus(const not_null_ptr<ItemInstance>& item)
{
    return getItemEnchantmentLevel(Enchantments::FISHING_LUCK, item);
}

int EnchantmentHelper::getFishingSpeedBonus(const not_null_ptr<ItemInstance>& item)
{
    return getItemEnchantmentLevel(Enchantments::FISHING_SPEED, item);
}

int EnchantmentHelper::getMobLooting(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::MOB_LOOTING, entity);
}

bool EnchantmentHelper::hasAquaAffinity(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::AQUA_AFFINITY, entity) > 0;
}

bool EnchantmentHelper::hasFrostWalker(const boost::shared_ptr<LivingEntity>& entity)
{
    return getEnchantmentLevel(Enchantments::FROST_WALKER, entity) > 0;
}

bool EnchantmentHelper::hasBindingCurse(const not_null_ptr<ItemInstance>& item)
{
    return getItemEnchantmentLevel(Enchantments::BINDING_CURSE, item) > 0;
}

bool EnchantmentHelper::hasVanishingCurse(const not_null_ptr<ItemInstance>& item)
{
    return getItemEnchantmentLevel(Enchantments::VANISHING_CURSE, item) > 0;
}

bool EnchantmentHelper::hasChanneling(const not_null_ptr<ItemInstance>& item)
{
    return getItemEnchantmentLevel(Enchantments::CHANNELING, item) > 0;
}

boost::shared_ptr<ItemInstance> EnchantmentHelper::getRandomItemWith(Enchantment* enchantment,
                                                                     const boost::shared_ptr<LivingEntity>& entity)
{
    std::vector<boost::weak_ptr<ItemInstance> > items = enchantment->getSlotItems(entity);
    if (items.empty())
    {
        return ItemInstance::EMPTY;
    }

    std::vector<boost::weak_ptr<ItemInstance> >::iterator it = items.begin();
    while (it != items.end())
    {
        not_null_ptr<ItemInstance> item(it->lock());

        if (!item->isEmpty() && getItemEnchantmentLevel(enchantment, item) > 0)
        {
            ++it;
        }
        else
        {
            it = items.erase(it);
        }
    }

    if (items.empty())
    {
        return ItemInstance::EMPTY;
    }

    return items.at(entity->getRandom()->nextInt(static_cast<int>(items.size()))).lock();
}

int EnchantmentHelper::getEnchantmentCost(Random* random, int slot, int level, const not_null_ptr<ItemInstance>& item)
{
    if (item->getItem()->getEnchantmentValue() < 1)
    {
        return 0;
    }

    if (level > 15)
    {
        level = 15;
    }

    int value = random->nextInt(8) + 1 + (level >> 1) + random->nextInt(level + 1);

    if (slot == 0)
    {
        int third = value / 3;
        return third < 1 ? 1 : third;
    }

    if (slot == 1)
    {
        return value * 2 / 3 + 1;
    }

    int doubled = level * 2;
    if (doubled < value)
    {
        doubled = value;
    }

    return doubled;
}

std::vector<EnchantmentInstance*>* EnchantmentHelper::getAvailableEnchantmentResults(
    int cost, const not_null_ptr<ItemInstance>& item, bool treasure)
{
    Item* itemType = item->getItem();
    std::vector<EnchantmentInstance*>* results = new std::vector<EnchantmentInstance*>();

    bool isBook = item->getItem() == Items::book;

    const boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>, std::equal_to<Enchantment*> >&
        registry = Enchantment::enchantmentRegistry->mInverseRegistry;

    for (boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>,
                              std::equal_to<Enchantment*> >::const_iterator it = registry.begin();
         it != registry.end(); ++it)
    {
        Enchantment* enchantment = it->first;

        if (enchantment->isTreasureOnly() && !treasure)
        {
            continue;
        }

        if (!enchantment->mCategory->canEnchant(itemType) && !isBook)
        {
            continue;
        }

        for (int level = enchantment->getMaxLevel(); level >= enchantment->getMinLevel(); --level)
        {
            if (enchantment->getMinCost(level) <= cost && cost <= enchantment->getMaxCost(level))
            {
                results->push_back(new EnchantmentInstance(enchantment, level));
                break;
            }
        }
    }

    return results;
}

void EnchantmentHelper::filterCompatibleEnchantments(std::vector<EnchantmentInstance*>* enchantments,
                                                     const EnchantmentInstance* selected)
{
    std::vector<EnchantmentInstance*>::iterator it = enchantments->begin();
    while (it != enchantments->end())
    {
        if (selected->enchantment->isCompatibleWith((*it)->enchantment))
        {
            ++it;
        }
        else
        {
            it = enchantments->erase(it);
        }
    }
}

std::vector<EnchantmentInstance*>* EnchantmentHelper::selectEnchantment(Random* random,
                                                                        const not_null_ptr<ItemInstance>& item,
                                                                        int level, bool treasure)
{
    std::vector<EnchantmentInstance*>* selected = new std::vector<EnchantmentInstance*>();

    int enchantmentValue = item->getItem()->getEnchantmentValue();
    if (enchantmentValue <= 0)
    {
        return selected;
    }

    int spread = enchantmentValue / 4;
    int cost = level + 1 + random->nextInt(spread + 1) + random->nextInt(spread + 1);
    float deviation = (random->nextFloat() + random->nextFloat() - 1.0f) * 0.15f;
    cost = Mth::clamp(Math::round(static_cast<float>(cost) + static_cast<float>(cost) * deviation), 1, 2147483647);

    std::vector<EnchantmentInstance*>* available = getAvailableEnchantmentResults(cost, item, treasure);
    if (!available->empty())
    {
        selected->push_back(static_cast<EnchantmentInstance*>(
            WeighedRandom::getRandomItem(random, available->begin(), available->end())));

        while (random->nextInt(50) <= cost)
        {
            filterCompatibleEnchantments(available, selected->back());
            if (available->empty())
            {
                break;
            }

            selected->push_back(static_cast<EnchantmentInstance*>(
                WeighedRandom::getRandomItem(random, available->begin(), available->end())));

            cost /= 2;
        }
    }

    delete available;

    return selected;
}

not_null_ptr<ItemInstance> EnchantmentHelper::enchantItem(Random* random, not_null_ptr<ItemInstance> item, int level,
                                                          bool treasure)
{
    std::vector<EnchantmentInstance*>* selected = selectEnchantment(random, item, level, treasure);

    bool isBook = item->getItem() == Items::book;
    if (isBook)
    {
        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::enchantedBook));
    }

    if (selected != NULL)
    {
        for (std::vector<EnchantmentInstance*>::iterator it = selected->begin(); it != selected->end(); ++it)
        {
            EnchantmentInstance* instance = *it;

            if (isBook)
            {
                EnchantedBookItem::addEnchantment(item, instance);
            }
            else
            {
                item->enchant(instance->enchantment, instance->level);
            }

            delete instance;
        }

        delete selected;
    }

    return item;
}
