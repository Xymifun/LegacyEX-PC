#ifndef GAME_ITEM_ENCHANTMENT_HPP
#define GAME_ITEM_ENCHANTMENT_HPP

#include <functional>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/HtmlString.hpp"
#include "game/core/Rarity.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DamageSource;
class EnchantmentCategory;
class Entity;
class EquipmentSlot;
class ItemInstance;
class LivingEntity;
class Rarity;
class ResourceLocation;


template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
class MappedRegistry;

class Enchantment
{
public:
    static MappedRegistry<ResourceLocation, Enchantment*, boost::hash<ResourceLocation>,
                          std::equal_to<ResourceLocation>, boost::hash<Enchantment*>, std::equal_to<Enchantment*> >*
        enchantmentRegistry;

    Enchantment(const Rarity* rarity, const EnchantmentCategory* category, arrayWithLength<const EquipmentSlot*> slots);

    static void staticCtor();

    static Enchantment* byId(int id);
    static int getId(Enchantment* enchantment);
    static Enchantment* byName(const std::wstring& name);

    bool isCompatibleWith(Enchantment* other) const;
    std::wstring getLevelString(int level);

    virtual const Rarity* getRarity();
    virtual std::vector<boost::weak_ptr<ItemInstance> > getSlotItems(boost::shared_ptr<LivingEntity> entity);
    virtual int getMinLevel();
    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getDamageProtection(int level, const DamageSource* damageSource);
    virtual float getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target);
    virtual bool checkCompatibility(const Enchantment* other) const;
    virtual void setDescriptionId(int descriptionId);
    virtual int getDescriptionId();
    virtual HtmlString getFullname(int level);
    virtual bool canEnchant(not_null_ptr<ItemInstance> itemInstance);
    virtual void doPostAttack(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target, int level);
    virtual void doPostHurt(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<Entity> attacker, int level);
    virtual bool isTreasureOnly();
    virtual bool isCurse();
    virtual ~Enchantment() {}

    const EnchantmentCategory* mCategory;

protected:
    int mDescriptionId;
    arrayWithLength<const EquipmentSlot*> mSlots;
    const Rarity* mRarity;
};

#endif
