#include "game/item/ProtectionEnchantment.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"

const ProtectionEnchantment::Type* ProtectionEnchantment::Type::ALL = new ProtectionEnchantment::Type((int)0xF84A0F07,
                                                                                                      1, 11, 20);
const ProtectionEnchantment::Type* ProtectionEnchantment::Type::FIRE = new ProtectionEnchantment::Type((int)0xE1AA8A26,
                                                                                                       10, 8, 12);
const ProtectionEnchantment::Type* ProtectionEnchantment::Type::FALL = new ProtectionEnchantment::Type((int)0xF470F5F1,
                                                                                                       5, 6, 10);
const ProtectionEnchantment::Type* ProtectionEnchantment::Type::EXPLOSION = new ProtectionEnchantment::Type(
    (int)0x7FF9AA23, 5, 8, 12);
const ProtectionEnchantment::Type* ProtectionEnchantment::Type::PROJECTILE = new ProtectionEnchantment::Type(
    (int)0xCF623683, 3, 6, 15);

ProtectionEnchantment::Type::Type(int name, int minCost, int levelCost, int levelCostSpan)
    : mName(name),
      mMinCost(minCost),
      mLevelCost(levelCost),
      mLevelCostSpan(levelCostSpan)
{
}

unsigned int ProtectionEnchantment::Type::getMinCost() const
{
    return mMinCost;
}

unsigned int ProtectionEnchantment::Type::getLevelCost() const
{
    return mLevelCost;
}

unsigned int ProtectionEnchantment::Type::getName() const
{
    return mName;
}

ProtectionEnchantment::ProtectionEnchantment(const Rarity* rarity, const Type* type,
                                             arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, type == Type::FALL ? EnchantmentCategory::ARMOR_FEET : EnchantmentCategory::ARMOR, slots),
      mType(type)
{
}

int ProtectionEnchantment::getMinCost(int level)
{
    return mType->getMinCost() + (level - 1) * mType->getLevelCost();
}

int ProtectionEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + mType->getLevelCost();
}

int ProtectionEnchantment::getMaxLevel()
{
    return 4;
}

int ProtectionEnchantment::getDamageProtection(int level, const DamageSource* damageSource)
{
    if (!damageSource->isBypassInvul())
    {
        if (mType == Type::ALL)
        {
            return level;
        }
        if (mType == Type::FIRE && damageSource->isFire())
        {
            return level * 2;
        }
        if (mType == Type::FALL && damageSource == DamageSource::FALL)
        {
            return level * 3;
        }
        if (mType == Type::EXPLOSION && damageSource->isExplosion())
        {
            return level * 2;
        }
        if (mType == Type::PROJECTILE && damageSource->isProjectile())
        {
            return level * 2;
        }
    }
    return 0;
}

int ProtectionEnchantment::getDescriptionId()
{
    return mType->getName();
}

bool ProtectionEnchantment::checkCompatibility(const Enchantment* other) const
{
    const ProtectionEnchantment* protection = dynamic_cast<const ProtectionEnchantment*>(other);
    if (protection != NULL)
    {
        if (protection->mType != mType && (mType == Type::FALL || protection->mType == Type::FALL))
        {
            return true;
        }
        return false;
    }
    return Enchantment::checkCompatibility(other);
}

int ProtectionEnchantment::getFireAfterDampener(const boost::shared_ptr<LivingEntity>& entity, int damage)
{
    int level = EnchantmentHelper::getEnchantmentLevel(Enchantments::FIRE_PROTECTION, entity);
    if (level > 0)
    {
        damage -= Mth::floor((float)damage * ((float)level * 0.15f));
    }
    return damage;
}

double ProtectionEnchantment::getExplosionKnockbackAfterDampener(const boost::shared_ptr<LivingEntity>& entity,
                                                                 double knockback)
{
    int level = EnchantmentHelper::getEnchantmentLevel(Enchantments::BLAST_PROTECTION, entity);
    if (level > 0)
    {
        knockback -= Mth::floor(knockback * (double)((float)level * 0.15f));
    }
    return knockback;
}

ProtectionEnchantment::~ProtectionEnchantment()
{
}
