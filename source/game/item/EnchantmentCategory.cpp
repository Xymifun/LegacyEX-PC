#include "game/item/EnchantmentCategory.hpp"

#include "game/core/AllCategory.hpp"
#include "game/core/ArmorCategory.hpp"
#include "game/core/BowCategory.hpp"
#include "game/core/BreakableCategory.hpp"
#include "game/core/ChestArmorCategory.hpp"
#include "game/core/DiggerCategory.hpp"
#include "game/core/FeetArmorCategory.hpp"
#include "game/core/FishingRodCategory.hpp"
#include "game/core/HeadArmorCategory.hpp"
#include "game/core/LegsArmorCategory.hpp"
#include "game/core/TridentCategory.hpp"
#include "game/core/WeaponCategory.hpp"
#include "game/core/WearableCategory.hpp"

std::vector<const EnchantmentCategory*> EnchantmentCategory::CATEGORIES;

const EnchantmentCategory* EnchantmentCategory::ALL = new AllCategory();
const EnchantmentCategory* EnchantmentCategory::ARMOR = new ArmorCategory();
const EnchantmentCategory* EnchantmentCategory::ARMOR_FEET = new FeetArmorCategory();
const EnchantmentCategory* EnchantmentCategory::ARMOR_LEGS = new LegsArmorCategory();
const EnchantmentCategory* EnchantmentCategory::ARMOR_CHEST = new ChestArmorCategory();
const EnchantmentCategory* EnchantmentCategory::ARMOR_HEAD = new HeadArmorCategory();
const EnchantmentCategory* EnchantmentCategory::WEAPON = new WeaponCategory();
const EnchantmentCategory* EnchantmentCategory::DIGGER = new DiggerCategory();
const EnchantmentCategory* EnchantmentCategory::FISHING_ROD = new FishingRodCategory();
const EnchantmentCategory* EnchantmentCategory::BREAKABLE = new BreakableCategory();
const EnchantmentCategory* EnchantmentCategory::BOW = new BowCategory();
const EnchantmentCategory* EnchantmentCategory::WEARABLE = new WearableCategory();
const EnchantmentCategory* EnchantmentCategory::TRIDENT = new TridentCategory();

EnchantmentCategory::EnchantmentCategory()
{
    CATEGORIES.push_back(this);
}
