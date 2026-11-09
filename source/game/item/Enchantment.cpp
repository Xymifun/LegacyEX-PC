#include "game/item/Enchantment.hpp"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/EquipmentSlot.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Rarity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/ArrowDamageEnchantment.hpp"
#include "game/entity/projectile/ArrowFireEnchantment.hpp"
#include "game/entity/projectile/ArrowInfiniteEnchantment.hpp"
#include "game/entity/projectile/ArrowKnockbackEnchantment.hpp"
#include "game/item/BindingCurseEnchantment.hpp"
#include "game/item/DamageEnchantment.hpp"
#include "game/item/DigDurabilityEnchantment.hpp"
#include "game/item/DiggingEnchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/FireAspectEnchantment.hpp"
#include "game/item/FishingSpeedEnchantment.hpp"
#include "game/item/FrostWalkerEnchantment.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/KnockbackEnchantment.hpp"
#include "game/item/LootBonusEnchantment.hpp"
#include "game/item/MendingEnchantment.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/item/OxygenEnchantment.hpp"
#include "game/item/ProtectionEnchantment.hpp"
#include "game/item/ThornsEnchantment.hpp"
#include "game/item/TridentChannelingEnchantment.hpp"
#include "game/item/TridentImpalerEnchantment.hpp"
#include "game/item/TridentLoyaltyEnchantment.hpp"
#include "game/item/TridentRiptideEnchantment.hpp"
#include "game/item/UntouchingEnchantment.hpp"
#include "game/item/VanishingCurseEnchantment.hpp"
#include "game/item/WaterWalkerEnchantment.hpp"
#include "game/item/WaterWorkerEnchantment.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/IdMapper.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/core/StringFormat.hpp"

MappedRegistry<ResourceLocation, Enchantment*, boost::hash<ResourceLocation>, std::equal_to<ResourceLocation>,
               boost::hash<Enchantment*>, std::equal_to<Enchantment*> >* Enchantment::enchantmentRegistry =
    new MappedRegistry<ResourceLocation, Enchantment*, boost::hash<ResourceLocation>, std::equal_to<ResourceLocation>,
                       boost::hash<Enchantment*>, std::equal_to<Enchantment*> >();

Enchantment::Enchantment(const Rarity* rarity, const EnchantmentCategory* category,
                         arrayWithLength<const EquipmentSlot*> slots)
    : mCategory(category),
      mSlots(slots),
      mRarity(rarity)
{
}

Enchantment* Enchantment::byId(int id)
{
    return enchantmentRegistry->byId(id);
}

int Enchantment::getId(Enchantment* enchantment)
{
    return enchantmentRegistry->getId(enchantment);
}

Enchantment* Enchantment::byName(const std::wstring& name)
{
    return enchantmentRegistry->get(ResourceLocation(name));
}

bool Enchantment::isCompatibleWith(Enchantment* other) const
{
    return checkCompatibility(other) && other->checkCompatibility(this);
}

std::wstring Enchantment::getLevelString(int level)
{
    const unsigned int levelStringIds[] = {0x66E0206A, 0x66E0206A, 0x40DDA601, 0x1ADB2B98, 0x24EC8477, 0xFEEA0A0E,
                                           0xD8E78FA5, 0xB2E5153C, 0x5CCEC78B, 0x36CC4D22, 0x9E0C28EA};

    if (level < 12)
    {
        return L10N::GetString(levelStringIds[level]);
    }

    return L10N::GetString(levelStringIds[0]);
}

const Rarity* Enchantment::getRarity()
{
    return mRarity;
}

std::vector<boost::weak_ptr<ItemInstance> > Enchantment::getSlotItems(boost::shared_ptr<LivingEntity> entity)
{
    std::vector<boost::weak_ptr<ItemInstance> > items;

    for (const EquipmentSlot** slot = mSlots.data; slot != mSlots.data + mSlots.length; slot++)
    {
        not_null_ptr<ItemInstance> itemInstance = entity->getItemSlot(*slot);

        if (!itemInstance->isEmpty())
        {
            items.push_back(itemInstance);
        }
    }

    return items;
}

int Enchantment::getMinLevel()
{
    return 1;
}

int Enchantment::getMaxLevel()
{
    return 1;
}

int Enchantment::getMinCost(int level)
{
    return level * 10 + 1;
}

int Enchantment::getMaxCost(int level)
{
    return getMinCost(level) + 5;
}

int Enchantment::getDamageProtection(int level, const DamageSource* damageSource)
{
    return 0;
}

float Enchantment::getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target)
{
    return 0.0f;
}

bool Enchantment::checkCompatibility(const Enchantment* other) const
{
    return this != other;
}

void Enchantment::setDescriptionId(int descriptionId)
{
    mDescriptionId = descriptionId;
}

int Enchantment::getDescriptionId()
{
    return mDescriptionId;
}

HtmlString Enchantment::getFullname(int level)
{
    eMinecraftColour colour = eMinecraftColour_HTMLColor_f;

    if (isCurse())
    {
        colour = eMinecraftColour_HTMLColor_4;
    }

    return HtmlString(formatwstr(L"%ls %ls", L10N::GetString(getDescriptionId()), getLevelString(level).c_str()),
                      colour, false, false, 0);
}

bool Enchantment::canEnchant(not_null_ptr<ItemInstance> itemInstance)
{
    return mCategory->canEnchant(itemInstance->getItem());
}

void Enchantment::doPostAttack(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target, int level)
{
}

void Enchantment::doPostHurt(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<Entity> attacker, int level)
{
}

bool Enchantment::isTreasureOnly()
{
    return false;
}

bool Enchantment::isCurse()
{
    return false;
}

void Enchantment::staticCtor()
{
    arrayWithLength<const EquipmentSlot*> armorSlots(4, true);
    armorSlots.data[0] = EquipmentSlot::HEAD;
    armorSlots.data[1] = EquipmentSlot::CHEST;
    armorSlots.data[2] = EquipmentSlot::LEGS;
    armorSlots.data[3] = EquipmentSlot::FEET;

    arrayWithLength<const EquipmentSlot*> mainHandSlots(1, true);
    mainHandSlots.data[0] = EquipmentSlot::MAINHAND;

    arrayWithLength<const EquipmentSlot*> feetSlots(1, true);
    feetSlots.data[0] = EquipmentSlot::FEET;

    arrayWithLength<const EquipmentSlot*> allSlots(6, true);
    allSlots.data[0] = EquipmentSlot::MAINHAND;
    allSlots.data[1] = EquipmentSlot::OFFHAND;
    allSlots.data[2] = EquipmentSlot::FEET;
    allSlots.data[3] = EquipmentSlot::LEGS;
    allSlots.data[4] = EquipmentSlot::CHEST;
    allSlots.data[5] = EquipmentSlot::HEAD;

    enchantmentRegistry->registerMapping(
        0, ResourceLocation(L"protection"),
        new ProtectionEnchantment(Rarity::COMMON, ProtectionEnchantment::Type::ALL, armorSlots));
    enchantmentRegistry->registerMapping(
        1, ResourceLocation(L"fire_protection"),
        new ProtectionEnchantment(Rarity::UNCOMMON, ProtectionEnchantment::Type::FIRE, armorSlots));
    enchantmentRegistry->registerMapping(
        2, ResourceLocation(L"feather_falling"),
        new ProtectionEnchantment(Rarity::UNCOMMON, ProtectionEnchantment::Type::FALL, armorSlots));
    enchantmentRegistry->registerMapping(
        3, ResourceLocation(L"blast_protection"),
        new ProtectionEnchantment(Rarity::RARE, ProtectionEnchantment::Type::EXPLOSION, armorSlots));
    enchantmentRegistry->registerMapping(
        4, ResourceLocation(L"projectile_protection"),
        new ProtectionEnchantment(Rarity::UNCOMMON, ProtectionEnchantment::Type::PROJECTILE, armorSlots));
    enchantmentRegistry->registerMapping(5, ResourceLocation(L"respiration"),
                                         new OxygenEnchantment(Rarity::RARE, armorSlots));
    enchantmentRegistry->registerMapping(6, ResourceLocation(L"aqua_affinity"),
                                         new WaterWorkerEnchantment(Rarity::RARE, armorSlots));
    enchantmentRegistry->registerMapping(7, ResourceLocation(L"thorns"),
                                         new ThornsEnchantment(Rarity::EPIC, armorSlots));
    enchantmentRegistry->registerMapping(8, ResourceLocation(L"depth_strider"),
                                         new WaterWalkerEnchantment(Rarity::RARE, armorSlots));
    enchantmentRegistry->registerMapping(9, ResourceLocation(L"frost_walker"),
                                         new FrostWalkerEnchantment(Rarity::RARE, feetSlots));
    enchantmentRegistry->registerMapping(10, ResourceLocation(L"binding_curse"),
                                         new BindingCurseEnchantment(Rarity::EPIC, feetSlots));

    enchantmentRegistry->registerMapping(16, ResourceLocation(L"sharpness"),
                                         new DamageEnchantment(Rarity::COMMON, 0, mainHandSlots));
    enchantmentRegistry->registerMapping(17, ResourceLocation(L"smite"),
                                         new DamageEnchantment(Rarity::UNCOMMON, 1, mainHandSlots));
    enchantmentRegistry->registerMapping(18, ResourceLocation(L"bane_of_arthropods"),
                                         new DamageEnchantment(Rarity::UNCOMMON, 2, mainHandSlots));
    enchantmentRegistry->registerMapping(19, ResourceLocation(L"knockback"),
                                         new KnockbackEnchantment(Rarity::UNCOMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(20, ResourceLocation(L"fire_aspect"),
                                         new FireAspectEnchantment(Rarity::RARE, mainHandSlots));
    enchantmentRegistry->registerMapping(
        21, ResourceLocation(L"looting"),
        new LootBonusEnchantment(Rarity::RARE, EnchantmentCategory::WEAPON, mainHandSlots));

    enchantmentRegistry->registerMapping(32, ResourceLocation(L"efficiency"),
                                         new DiggingEnchantment(Rarity::COMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(33, ResourceLocation(L"silk_touch"),
                                         new UntouchingEnchantment(Rarity::EPIC, mainHandSlots));
    enchantmentRegistry->registerMapping(34, ResourceLocation(L"unbreaking"),
                                         new DigDurabilityEnchantment(Rarity::UNCOMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(
        35, ResourceLocation(L"fortune"),
        new LootBonusEnchantment(Rarity::RARE, EnchantmentCategory::DIGGER, mainHandSlots));

    enchantmentRegistry->registerMapping(48, ResourceLocation(L"power"),
                                         new ArrowDamageEnchantment(Rarity::COMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(49, ResourceLocation(L"punch"),
                                         new ArrowKnockbackEnchantment(Rarity::UNCOMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(50, ResourceLocation(L"flame"),
                                         new ArrowFireEnchantment(Rarity::RARE, mainHandSlots));
    enchantmentRegistry->registerMapping(51, ResourceLocation(L"infinity"),
                                         new ArrowInfiniteEnchantment(Rarity::EPIC, mainHandSlots));

    enchantmentRegistry->registerMapping(
        61, ResourceLocation(L"luck_of_the_sea"),
        new LootBonusEnchantment(Rarity::RARE, EnchantmentCategory::FISHING_ROD, mainHandSlots));
    enchantmentRegistry->registerMapping(
        62, ResourceLocation(L"lure"),
        new FishingSpeedEnchantment(Rarity::RARE, EnchantmentCategory::FISHING_ROD, mainHandSlots));

    enchantmentRegistry->registerMapping(70, ResourceLocation(L"mending"),
                                         new MendingEnchantment(Rarity::RARE, allSlots));
    enchantmentRegistry->registerMapping(71, ResourceLocation(L"vanishing_curse"),
                                         new VanishingCurseEnchantment(Rarity::EPIC, allSlots));

    enchantmentRegistry->registerMapping(80, ResourceLocation(L"impaling"),
                                         new TridentImpalerEnchantment(Rarity::UNCOMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(81, ResourceLocation(L"riptide"),
                                         new TridentRiptideEnchantment(Rarity::RARE, mainHandSlots));
    enchantmentRegistry->registerMapping(82, ResourceLocation(L"loyalty"),
                                         new TridentLoyaltyEnchantment(Rarity::COMMON, mainHandSlots));
    enchantmentRegistry->registerMapping(83, ResourceLocation(L"channeling"),
                                         new TridentChannelingEnchantment(Rarity::EPIC, mainHandSlots));
}
