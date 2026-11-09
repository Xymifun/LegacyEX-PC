#include "game/core/BuiltInLootTables.hpp"

#include <string>
#include <utility>
#include <vector>

#include "game/core/ExplorationMapFunction.hpp"
#include "game/core/LootPool.hpp"
#include "game/core/LootPoolEntry.hpp"
#include "game/core/LootPoolTiers.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTableEntry.hpp"
#include "game/core/LootTableReference.hpp"
#include "game/core/SetDataFromColorIndexFunction.hpp"
#include "game/core/SetNBTFunction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/EmptyLootItem.hpp"
#include "game/item/EnchantRandomEquipmentFunction.hpp"
#include "game/item/EnchantRandomlyFunction.hpp"
#include "game/item/EnchantWithLevelsFunction.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/LootItem.hpp"
#include "game/item/LootItemFunction.hpp"
#include "game/item/LootItemRandomChanceWithLootingCondition.hpp"
#include "game/item/LootItemRandomChanceWithSpecialModifierCondition.hpp"
#include "game/item/LootItemRandomDifficultyChanceCondition.hpp"
#include "game/item/LootingEnchantFunction.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/SetItemCountFunction.hpp"
#include "game/item/SetItemDamageFunction.hpp"
#include "game/item/SetItemDataFunction.hpp"
#include "game/item/SmeltItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EntityProperty.hpp"
#include "game/world/block/EntityPropertyOnFire.hpp"
#include "game/world/block/LootItemEntityPropertyCondition.hpp"
#include "game/world/generation/LootItemKilledByPlayerCondition.hpp"
#include "game/core/LootItemCondition.hpp"


ResourceLocation* BuiltInLootTables::EMPTY = NULL;

ResourceLocation BuiltInLootTables::ABANDONED_MINESHAFT(L"minecraft", L"chests/abandoned_mineshaft");
ResourceLocation BuiltInLootTables::BURIEDTREASURE(L"minecraft", L"chests/buriedtreasure");
ResourceLocation BuiltInLootTables::DESERT_PYRAMID(L"minecraft", L"chests/desert_pyramid");
ResourceLocation BuiltInLootTables::DISPENSER_TRAP(L"minecraft", L"chests/dispenser_trap");
ResourceLocation BuiltInLootTables::END_CITY_TREASURE(L"minecraft", L"chests/end_city_treasure");
ResourceLocation BuiltInLootTables::IGLOO_CHEST(L"minecraft", L"chests/igloo_chest");
ResourceLocation BuiltInLootTables::JUNGLE_TEMPLE(L"minecraft", L"chests/jungle_temple");
ResourceLocation BuiltInLootTables::JUNGLE_TEMPLE_DISPENSER(L"minecraft", L"chests/jungle_temple_dispenser");
ResourceLocation BuiltInLootTables::MONSTER_ROOM(L"minecraft", L"chests/monster_room");
ResourceLocation BuiltInLootTables::NETHER_BRIDGE(L"minecraft", L"chests/nether_bridge");
ResourceLocation BuiltInLootTables::SHIPWRECK(L"minecraft", L"chests/shipwreck");
ResourceLocation BuiltInLootTables::SHIPWRECKSUPPLY(L"minecraft", L"chests/shipwrecksupply");
ResourceLocation BuiltInLootTables::SHIPWRECKTREASURE(L"minecraft", L"chests/shipwrecktreasure");
ResourceLocation BuiltInLootTables::SIMPLE_DUNGEON(L"minecraft", L"chests/simple_dungeon");
ResourceLocation BuiltInLootTables::SPAWN_BONUS_CHEST(L"minecraft", L"chests/spawn_bonus_chest");
ResourceLocation BuiltInLootTables::STRONGHOLD_CORRIDOR(L"minecraft", L"chests/stronghold_corridor");
ResourceLocation BuiltInLootTables::STRONGHOLD_CROSSING(L"minecraft", L"chests/stronghold_crossing");
ResourceLocation BuiltInLootTables::STRONGHOLD_LIBRARY(L"minecraft", L"chests/stronghold_library");
ResourceLocation BuiltInLootTables::UNDERWATER_RUIN_BIG(L"minecraft", L"chests/underwater_ruin_big");
ResourceLocation BuiltInLootTables::UNDERWATER_RUIN_SMALL(L"minecraft", L"chests/underwater_ruin_small");
ResourceLocation BuiltInLootTables::VILLAGE_BLACKSMITH(L"minecraft", L"chests/village_blacksmith");
ResourceLocation BuiltInLootTables::VILLAGE_TWO_ROOM_HOUSE(L"minecraft", L"chests/village_two_room_house");
ResourceLocation BuiltInLootTables::WOODLAND_MANSION(L"minecraft", L"chests/woodland_mansion");
ResourceLocation BuiltInLootTables::ARMOR_SET_CHAIN(L"minecraft", L"entities/armor_set_chain");
ResourceLocation BuiltInLootTables::ARMOR_SET_DIAMOND(L"minecraft", L"entities/armor_set_diamond");
ResourceLocation BuiltInLootTables::ARMOR_SET_GOLD(L"minecraft", L"entities/armor_set_gold");
ResourceLocation BuiltInLootTables::ARMOR_SET_IRON(L"minecraft", L"entities/armor_set_iron");
ResourceLocation BuiltInLootTables::ARMOR_SET_LEATHER(L"minecraft", L"entities/armor_set_leather");
ResourceLocation BuiltInLootTables::ARMOR_STAND(L"minecraft", L"entities/armor_stand");
ResourceLocation BuiltInLootTables::BAT(L"minecraft", L"entities/bat");
ResourceLocation BuiltInLootTables::BLAZE(L"minecraft", L"entities/blaze");
ResourceLocation BuiltInLootTables::BOAT(L"minecraft", L"entities/boat");
ResourceLocation BuiltInLootTables::CAVE_SPIDER(L"minecraft", L"entities/cave_spider");
ResourceLocation BuiltInLootTables::CHICKEN(L"minecraft", L"entities/chicken");
ResourceLocation BuiltInLootTables::CLOWNFISH(L"minecraft", L"entities/clownfish");
ResourceLocation BuiltInLootTables::COW(L"minecraft", L"entities/cow");
ResourceLocation BuiltInLootTables::CREEPER(L"minecraft", L"entities/creeper");
ResourceLocation BuiltInLootTables::DOLPHIN(L"minecraft", L"entities/dolphin");
ResourceLocation BuiltInLootTables::DONKEY(L"minecraft", L"entities/donkey");
ResourceLocation BuiltInLootTables::DROWNED(L"minecraft", L"entities/drowned");
ResourceLocation BuiltInLootTables::DROWNED_EQUIPMENT(L"minecraft", L"entities/drowned_equipment");
ResourceLocation BuiltInLootTables::DROWNED_RANGED_EQUIPMENT(L"minecraft", L"entities/drowned_ranged_equipment");
ResourceLocation BuiltInLootTables::ELDER_GUARDIAN(L"minecraft", L"entities/elder_guardian");
ResourceLocation BuiltInLootTables::ENDERMAN(L"minecraft", L"entities/enderman");
ResourceLocation BuiltInLootTables::ENDERMITE(L"minecraft", L"entities/endermite");
ResourceLocation BuiltInLootTables::ENDER_DRAGON(L"minecraft", L"entities/ender_dragon");
ResourceLocation BuiltInLootTables::ENTITY_FISH(L"minecraft", L"entities/entity_fish");
ResourceLocation BuiltInLootTables::EVOCATION_ILLAGER(L"minecraft", L"entities/evocation_illager");
ResourceLocation BuiltInLootTables::GHAST(L"minecraft", L"entities/ghast");
ResourceLocation BuiltInLootTables::GIANT(L"minecraft", L"entities/giant");
ResourceLocation BuiltInLootTables::GUARDIAN(L"minecraft", L"entities/guardian");
ResourceLocation BuiltInLootTables::HORSE(L"minecraft", L"entities/horse");
ResourceLocation BuiltInLootTables::HUSK(L"minecraft", L"entities/husk");
ResourceLocation BuiltInLootTables::IRON_GOLEM(L"minecraft", L"entities/iron_golem");
ResourceLocation BuiltInLootTables::LLAMA(L"minecraft", L"entities/llama");
ResourceLocation BuiltInLootTables::MAGMA_CUBE(L"minecraft", L"entities/magma_cube");
ResourceLocation BuiltInLootTables::MOOSHROOM(L"minecraft", L"entities/mooshroom");
ResourceLocation BuiltInLootTables::MOOSHROOM_SHEAR(L"minecraft", L"entities/mooshroom_shear");
ResourceLocation BuiltInLootTables::MULE(L"minecraft", L"entities/mule");
ResourceLocation BuiltInLootTables::MUSHROOM_COW(L"minecraft", L"entities/mushroom_cow");
ResourceLocation BuiltInLootTables::OCELOT(L"minecraft", L"entities/ocelot");
ResourceLocation BuiltInLootTables::PARROT(L"minecraft", L"entities/parrot");
ResourceLocation BuiltInLootTables::PHANTOM(L"minecraft", L"entities/phantom");
ResourceLocation BuiltInLootTables::PIG(L"minecraft", L"entities/pig");
ResourceLocation BuiltInLootTables::PIG_SADDLED(L"minecraft", L"entities/pig_saddled");
ResourceLocation BuiltInLootTables::POLAR_BEAR(L"minecraft", L"entities/polar_bear");
ResourceLocation BuiltInLootTables::PUFFERFISH(L"minecraft", L"entities/pufferfish");
ResourceLocation BuiltInLootTables::RABBIT(L"minecraft", L"entities/rabbit");
ResourceLocation BuiltInLootTables::SALMON_LARGE(L"minecraft", L"entities/salmon_large");
ResourceLocation BuiltInLootTables::SALMON_NORMAL(L"minecraft", L"entities/salmon_normal");
ResourceLocation BuiltInLootTables::SEA_TURTLE(L"minecraft", L"entities/sea_turtle");
ResourceLocation BuiltInLootTables::SHEEP(L"minecraft", L"entities/sheep");
ResourceLocation BuiltInLootTables::SHEEP_SHEAR(L"minecraft", L"entities/sheep_shear");
ResourceLocation BuiltInLootTables::SHEEP_SHEARED(L"minecraft", L"entities/sheep_sheared");
ResourceLocation BuiltInLootTables::SHULKER(L"minecraft", L"entities/shulker");
ResourceLocation BuiltInLootTables::SILVERFISH(L"minecraft", L"entities/silverfish");
ResourceLocation BuiltInLootTables::SKELETON(L"minecraft", L"entities/skeleton");
ResourceLocation BuiltInLootTables::SKELETON_GEAR(L"minecraft", L"entities/skeleton_gear");
ResourceLocation BuiltInLootTables::SKELETON_HORSE(L"minecraft", L"entities/skeleton_horse");
ResourceLocation BuiltInLootTables::SLIME(L"minecraft", L"entities/slime");
ResourceLocation BuiltInLootTables::SNOWMAN(L"minecraft", L"entities/snowman");
ResourceLocation BuiltInLootTables::SPIDER(L"minecraft", L"entities/spider");
ResourceLocation BuiltInLootTables::SQUID(L"minecraft", L"entities/squid");
ResourceLocation BuiltInLootTables::STRAY(L"minecraft", L"entities/stray");
ResourceLocation BuiltInLootTables::TROPICALFISH(L"minecraft", L"entities/tropicalfish");
ResourceLocation BuiltInLootTables::VEX(L"minecraft", L"entities/vex");
ResourceLocation BuiltInLootTables::VEX_GEAR(L"minecraft", L"entities/vex_gear");
ResourceLocation BuiltInLootTables::VILLAGER(L"minecraft", L"entities/villager");
ResourceLocation BuiltInLootTables::VINDICATION_ILLAGER(L"minecraft", L"entities/vindication_illager");
ResourceLocation BuiltInLootTables::VINDICATOR_GEAR(L"minecraft", L"entities/vindicator_gear");
ResourceLocation BuiltInLootTables::WITCH(L"minecraft", L"entities/witch");
ResourceLocation BuiltInLootTables::WITHER_BOSS(L"minecraft", L"entities/wither_boss");
ResourceLocation BuiltInLootTables::WITHER_SKELETON(L"minecraft", L"entities/wither_skeleton");
ResourceLocation BuiltInLootTables::WITHER_SKELETON_GEAR(L"minecraft", L"entities/wither_skeleton_gear");
ResourceLocation BuiltInLootTables::WOLF(L"minecraft", L"entities/wolf");
ResourceLocation BuiltInLootTables::ZOMBIE(L"minecraft", L"entities/zombie");
ResourceLocation BuiltInLootTables::ZOMBIE_EQUIPMENT(L"minecraft", L"entities/zombie_equipment");
ResourceLocation BuiltInLootTables::ZOMBIE_HORSE(L"minecraft", L"entities/zombie_horse");
ResourceLocation BuiltInLootTables::ZOMBIE_PIGMAN(L"minecraft", L"entities/zombie_pigman");
ResourceLocation BuiltInLootTables::ZOMBIE_PIGMAN_GEAR(L"minecraft", L"entities/zombie_pigman_gear");
ResourceLocation BuiltInLootTables::ZOMBIE_VILLAGER(L"minecraft", L"entities/zombie_villager");
ResourceLocation BuiltInLootTables::LOW_TIER_ITEMS(L"minecraft", L"equipment/low_tier_items");
ResourceLocation BuiltInLootTables::FISHING(L"minecraft", L"gameplay/fishing");
ResourceLocation BuiltInLootTables::FISH(L"minecraft", L"gameplay/fishing/fish");
ResourceLocation BuiltInLootTables::JUNK(L"minecraft", L"gameplay/fishing/junk");
ResourceLocation BuiltInLootTables::TREASURE(L"minecraft", L"gameplay/fishing/treasure");

boost::unordered_map<ResourceLocation*, LootTable*> BuiltInLootTables::lootTables;

void BuiltInLootTables::staticLootTable_ABANDONED_MINESHAFT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::goldenApple, 20, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::goldenApple, 1, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 30, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::ironPickaxe, 5, 0, NULL, NULL, NULL));
    entries->push_back(new EmptyLootItem(5, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldIngot, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 9)));
    entries->push_back(new LootItem(Items::redstone, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(4)));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 9)));
    entries->push_back(new LootItem(Items::dye, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::diamond, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 8)));
    entries->push_back(new LootItem(Items::coal, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::bread, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::melonSeeds, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::pumpkinSeeds, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::beetrootSeeds, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 4), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 8)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::rail), 20, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::golden_rail), 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::detector_rail), 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::activator_rail), 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 16)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::torch), 15, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ABANDONED_MINESHAFT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_BURIEDTREASURE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::nautilusCore, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::prismarineCrystals, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 20, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::goldIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::tnt), 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::diamond, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::recordWait, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::recordMellohi, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::chainmailChestplate, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::chainmailHelmet, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::chainmailLeggings, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::chainmailBoots, 20, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::writableBook, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::lead, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::experienceBottle, 3, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    CompoundTag* tag = new CompoundTag();
    tag->putString(PotionUtils::TAG_POTION, L"water_breathing");
    functions->push_back(new SetNBTFunction(NULL, tag));
    entries->push_back(new LootItem(Items::potion, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    tag = new CompoundTag();
    tag->putString(PotionUtils::TAG_POTION, L"regeneration");
    functions->push_back(new SetNBTFunction(NULL, tag));
    entries->push_back(new LootItem(Items::potion, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::cake, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(5, 12), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&BURIEDTREASURE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DESERT_PYRAMID()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::diamond, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::goldIngot, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::emerald, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 6)));
    entries->push_back(new LootItem(Items::bone, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::spiderEye, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 7)));
    entries->push_back(new LootItem(Items::rottenFlesh, 25, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 20, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenApple, 20, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::goldenApple, 2, 0, functions, NULL, NULL));
    entries->push_back(new EmptyLootItem(15, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 4), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::bone, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::gunpowder, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::rottenFlesh, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::string, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::sand), 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(4), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DESERT_PYRAMID, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DISPENSER_TRAP()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::arrow, 30, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 2), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DISPENSER_TRAP, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_END_CITY_TREASURE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::diamond, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 8)));
    entries->push_back(new LootItem(Items::ironIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::goldIngot, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 6)));
    entries->push_back(new LootItem(Items::emerald, 2, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 10)));
    entries->push_back(new LootItem(Items::beetrootSeeds, 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 3, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondSword, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondBoots, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondChestplate, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondLeggings, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondHelmet, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondPickaxe, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::diamondShovel, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironSword, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironBoots, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironChestplate, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironLeggings, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironHelmet, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironPickaxe, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(20, 39), true));
    entries->push_back(new LootItem(Items::ironShovel, 3, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 6), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&END_CITY_TREASURE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_IGLOO_CHEST()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::apple, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::coal, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldNugget, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::stoneAxe, 2, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::rottenFlesh, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::emerald, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 3)));
    entries->push_back(new LootItem(Items::wheat, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 8), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::goldenApple, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&IGLOO_CHEST, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_JUNGLE_TEMPLE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::diamond, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::goldIngot, 75, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::emerald, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 6)));
    entries->push_back(new LootItem(Items::bone, 100, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 7)));
    entries->push_back(new LootItem(Items::rottenFlesh, 80, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::book, 6, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 6), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&JUNGLE_TEMPLE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_JUNGLE_TEMPLE_DISPENSER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::arrow, 30, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 2), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&JUNGLE_TEMPLE_DISPENSER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MONSTER_ROOM()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::redstone, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bread, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenApple, 10, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::wheat, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::gunpowder, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::string, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::bucket, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 2, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MONSTER_ROOM, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_NETHER_BRIDGE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::diamond, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldIngot, 15, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenSword, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenChestplate, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::flintAndSteel, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 7)));
    entries->push_back(new LootItem(Items::netherWart, 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 8, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 3, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::obsidian), 2, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 4), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&NETHER_BRIDGE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHIPWRECK()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new ExplorationMapFunction(NULL, L"BuriedTreasure"));
    entries->push_back(new LootItem(Items::map, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::compass, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::map, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::clock, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 10)));
    entries->push_back(new LootItem(Items::paper, 20, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::feather, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::book, 5, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHIPWRECK, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHIPWRECKSUPPLY()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 12)));
    entries->push_back(new LootItem(Items::paper, 8, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 6)));
    entries->push_back(new LootItem(Items::potato, 7, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 6)));
    entries->push_back(new LootItem(Items::poisonousPotato, 7, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 8)));
    entries->push_back(new LootItem(Items::carrot, 7, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(8, 21)));
    entries->push_back(new LootItem(Items::wheat, 7, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 8)));
    entries->push_back(new LootItem(Items::coal, 6, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(5, 24)));
    entries->push_back(new LootItem(Items::rottenFlesh, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::pumpkin), 2, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::gunpowder, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::tnt), 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::leatherHelmet, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::leatherChestplate, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::leatherLeggings, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::leatherBoots, 3, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3, 10), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHIPWRECKSUPPLY, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHIPWRECKTREASURE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 90, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::goldIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::emerald, 40, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::diamond, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::experienceBottle, 5, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3, 6), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 10)));
    entries->push_back(new LootItem(Items::ironNugget, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 10)));
    entries->push_back(new LootItem(Items::goldNugget, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 10)));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(4)));
    entries->push_back(new LootItem(Items::dye, 20, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 5), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHIPWRECKTREASURE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SIMPLE_DUNGEON()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::saddle, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenApple, 15, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::goldenApple, 2, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::record13, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::recordCat, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 20, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 15, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 3), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::ironIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::goldIngot, 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bread, 20, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::wheat, 20, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bucket, 10, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::redstone, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::coal, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::melonSeeds, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::pumpkinSeeds, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::beetrootSeeds, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 4), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::bone, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::gunpowder, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::rottenFlesh, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::string, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SIMPLE_DUNGEON, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SPAWN_BONUS_CHEST()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(3)));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::sapling), 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::melonSeeds, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::pumpkinSeeds, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::beetrootSeeds, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::stoneAxe, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::woodenAxe, 3, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::stonePickaxe, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::woodenPickaxe, 3, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::apple, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::bread, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::fish, 3, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::stick, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 12)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::planks), 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(0, 3)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::log), 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(0, 1)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::log2), 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(4), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SPAWN_BONUS_CHEST, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_STRONGHOLD_CORRIDOR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::enderPearl, 50, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::emerald, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::diamond, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldIngot, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 9)));
    entries->push_back(new LootItem(Items::redstone, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::bread, 75, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::apple, 75, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::ironPickaxe, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironSword, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironChestplate, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHelmet, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironLeggings, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironBoots, 25, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenApple, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::book, 6, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&STRONGHOLD_CORRIDOR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_STRONGHOLD_CROSSING()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldIngot, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 9)));
    entries->push_back(new LootItem(Items::redstone, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 8)));
    entries->push_back(new LootItem(Items::coal, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::bread, 75, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::apple, 75, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::ironPickaxe, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::book, 6, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(0)));
    entries->push_back(new LootItem(Items::dye, 75, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 4), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&STRONGHOLD_CROSSING, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_STRONGHOLD_LIBRARY()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::book, 100, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 7)));
    entries->push_back(new LootItem(Items::paper, 100, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::map, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::compass, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::book, 60, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 10), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&STRONGHOLD_LIBRARY, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_UNDERWATER_RUIN_BIG()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::coal, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldNugget, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::emerald, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 3)));
    entries->push_back(new LootItem(Items::wheat, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 8), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::goldenApple, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::leatherChestplate, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHelmet, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::fishingRod, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new ExplorationMapFunction(NULL, L"BuriedTreasure"));
    entries->push_back(new LootItem(Items::map, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&UNDERWATER_RUIN_BIG, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_UNDERWATER_RUIN_SMALL()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::coal, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::stoneAxe, 2, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::rottenFlesh, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::emerald, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 3)));
    entries->push_back(new LootItem(Items::wheat, 10, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(2, 8), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::leatherChestplate, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHelmet, 1, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::fishingRod, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new ExplorationMapFunction(NULL, L"BuriedTreasure"));
    entries->push_back(new LootItem(Items::map, 5, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&UNDERWATER_RUIN_SMALL, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_VILLAGE_BLACKSMITH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::diamond, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::goldIngot, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::bread, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Items::apple, 15, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::ironPickaxe, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironSword, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironChestplate, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHelmet, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironLeggings, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironBoots, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 7)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::obsidian), 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 7)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::sapling), 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 3, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHorseArmor, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenHorseArmor, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHorseArmor, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3, 8), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&VILLAGE_BLACKSMITH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_VILLAGE_TWO_ROOM_HOUSE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(5, 8)));
    entries->push_back(new LootItem(Items::potato, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(4, 8)));
    entries->push_back(new LootItem(Items::carrot, 10, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(8, 12)));
    entries->push_back(new LootItem(Items::wheat, 15, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::wheatSeeds, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(5, 8)));
    entries->push_back(new LootItem(Items::beetroot, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::woodenHoe, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(6, 8), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&VILLAGE_TWO_ROOM_HOUSE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WOODLAND_MANSION()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::lead, 100, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::goldenApple, 75, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::goldenApple, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::record13, 75, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::recordCat, 75, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 100, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::chainmailChestplate, 50, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondHoe, 75, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::diamondChestplate, 25, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomlyFunction(NULL, std::vector<Enchantment*>()));
    entries->push_back(new LootItem(Items::book, 60, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 3), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::ironIngot, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::goldIngot, 25, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bread, 100, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::wheat, 100, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bucket, 50, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::redstone, 75, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 4)));
    entries->push_back(new LootItem(Items::coal, 75, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::melonSeeds, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::pumpkinSeeds, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(2, 4)));
    entries->push_back(new LootItem(Items::beetrootSeeds, 50, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 4), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::bone, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::gunpowder, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::rottenFlesh, 50, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 8)));
    entries->push_back(new LootItem(Items::string, 50, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&WOODLAND_MANSION, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_SET_CHAIN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* helmetEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* chestplateEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* leggingsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* bootsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    bootsEntries->push_back(new LootItem(Items::chainmailBoots, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* bootsPools = new std::vector<LootPool*>;
    bootsPools->push_back(new LootPool(bootsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* bootsTable = new LootTableEntry(new LootTable(bootsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    leggingsEntries->push_back(new LootItem(Items::chainmailLeggings, 1, 0, functions, NULL, bootsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* leggingsPools = new std::vector<LootPool*>;
    leggingsPools->push_back(
        new LootPool(leggingsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* leggingsTable = new LootTableEntry(new LootTable(leggingsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    chestplateEntries->push_back(new LootItem(Items::chainmailChestplate, 1, 0, functions, NULL, leggingsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* chestplatePools = new std::vector<LootPool*>;
    chestplatePools->push_back(
        new LootPool(chestplateEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* chestplateTable = new LootTableEntry(new LootTable(chestplatePools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    helmetEntries->push_back(new LootItem(Items::chainmailHelmet, 1, 0, functions, NULL, chestplateTable));
    pools->push_back(new LootPool(helmetEntries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ARMOR_SET_CHAIN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_SET_DIAMOND()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* helmetEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* chestplateEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* leggingsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* bootsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    bootsEntries->push_back(new LootItem(Items::diamondBoots, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* bootsPools = new std::vector<LootPool*>;
    bootsPools->push_back(new LootPool(bootsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* bootsTable = new LootTableEntry(new LootTable(bootsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    leggingsEntries->push_back(new LootItem(Items::diamondLeggings, 1, 0, functions, NULL, bootsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* leggingsPools = new std::vector<LootPool*>;
    leggingsPools->push_back(
        new LootPool(leggingsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* leggingsTable = new LootTableEntry(new LootTable(leggingsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    chestplateEntries->push_back(new LootItem(Items::diamondChestplate, 1, 0, functions, NULL, leggingsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* chestplatePools = new std::vector<LootPool*>;
    chestplatePools->push_back(
        new LootPool(chestplateEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* chestplateTable = new LootTableEntry(new LootTable(chestplatePools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    helmetEntries->push_back(new LootItem(Items::diamondHelmet, 1, 0, functions, NULL, chestplateTable));
    pools->push_back(new LootPool(helmetEntries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ARMOR_SET_DIAMOND, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_SET_GOLD()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* helmetEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* chestplateEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* leggingsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* bootsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    bootsEntries->push_back(new LootItem(Items::goldenBoots, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* bootsPools = new std::vector<LootPool*>;
    bootsPools->push_back(new LootPool(bootsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* bootsTable = new LootTableEntry(new LootTable(bootsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    leggingsEntries->push_back(new LootItem(Items::goldenLeggings, 1, 0, functions, NULL, bootsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* leggingsPools = new std::vector<LootPool*>;
    leggingsPools->push_back(
        new LootPool(leggingsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* leggingsTable = new LootTableEntry(new LootTable(leggingsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    chestplateEntries->push_back(new LootItem(Items::goldenChestplate, 1, 0, functions, NULL, leggingsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* chestplatePools = new std::vector<LootPool*>;
    chestplatePools->push_back(
        new LootPool(chestplateEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* chestplateTable = new LootTableEntry(new LootTable(chestplatePools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    helmetEntries->push_back(new LootItem(Items::goldenHelmet, 1, 0, functions, NULL, chestplateTable));
    pools->push_back(new LootPool(helmetEntries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ARMOR_SET_GOLD, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_SET_IRON()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* helmetEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* chestplateEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* leggingsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* bootsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    bootsEntries->push_back(new LootItem(Items::ironBoots, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* bootsPools = new std::vector<LootPool*>;
    bootsPools->push_back(new LootPool(bootsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* bootsTable = new LootTableEntry(new LootTable(bootsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    leggingsEntries->push_back(new LootItem(Items::ironLeggings, 1, 0, functions, NULL, bootsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* leggingsPools = new std::vector<LootPool*>;
    leggingsPools->push_back(
        new LootPool(leggingsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* leggingsTable = new LootTableEntry(new LootTable(leggingsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    chestplateEntries->push_back(new LootItem(Items::ironChestplate, 1, 0, functions, NULL, leggingsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* chestplatePools = new std::vector<LootPool*>;
    chestplatePools->push_back(
        new LootPool(chestplateEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* chestplateTable = new LootTableEntry(new LootTable(chestplatePools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    helmetEntries->push_back(new LootItem(Items::ironHelmet, 1, 0, functions, NULL, chestplateTable));
    pools->push_back(new LootPool(helmetEntries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ARMOR_SET_IRON, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_SET_LEATHER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* helmetEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* chestplateEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* leggingsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootPoolEntry*>* bootsEntries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    bootsEntries->push_back(new LootItem(Items::leatherBoots, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* bootsPools = new std::vector<LootPool*>;
    bootsPools->push_back(new LootPool(bootsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* bootsTable = new LootTableEntry(new LootTable(bootsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    leggingsEntries->push_back(new LootItem(Items::leatherLeggings, 1, 0, functions, NULL, bootsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* leggingsPools = new std::vector<LootPool*>;
    leggingsPools->push_back(
        new LootPool(leggingsEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* leggingsTable = new LootTableEntry(new LootTable(leggingsPools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    chestplateEntries->push_back(new LootItem(Items::leatherChestplate, 1, 0, functions, NULL, leggingsTable));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.5f, 0.5f, 0.6f));
    std::vector<LootPool*>* chestplatePools = new std::vector<LootPool*>;
    chestplatePools->push_back(
        new LootPool(chestplateEntries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    LootTableEntry* chestplateTable = new LootTableEntry(new LootTable(chestplatePools));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    helmetEntries->push_back(new LootItem(Items::leatherHelmet, 1, 0, functions, NULL, chestplateTable));
    pools->push_back(new LootPool(helmetEntries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ARMOR_SET_LEATHER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ARMOR_STAND()
{
    lootTables.insert(std::make_pair(&ARMOR_STAND, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_BAT()
{
    lootTables.insert(std::make_pair(&BAT, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_BLAZE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::blazeRod, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    entries->push_back(new LootItem(Items::glowstoneDust, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&BLAZE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_BOAT()
{
    lootTables.insert(std::make_pair(&BOAT, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_CAVE_SPIDER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::string, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::spiderEye, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&CAVE_SPIDER, new LootTable(pools)));
}

void BuiltInLootTables::addSmeltItemWhenOnFireFunc(std::vector<LootItemFunction*>* functions)
{
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    std::vector<EntityProperty*> properties;
    properties.push_back(new EntityPropertyOnFire(true));
    conditions->push_back(new LootItemEntityPropertyCondition(properties, LootTableContext::THIS));
    functions->push_back(new SmeltItemFunction(conditions));
}

void BuiltInLootTables::staticLootTable_CHICKEN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::feather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::chicken, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&CHICKEN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_CLOWNFISH()
{
    lootTables.insert(std::make_pair(&CLOWNFISH, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_COW()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::beef, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&COW, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_CREEPER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::gunpowder, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&CREEPER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DOLPHIN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    addSmeltItemWhenOnFireFunc(functions);
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DOLPHIN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DONKEY()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DONKEY, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DROWNED()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDamageFunction(NULL, RandomValueBounds(0.2, 0.9)));
    entries->push_back(new LootItem(Items::trident, 5, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::goldIngot, 5, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.11f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DROWNED, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DROWNED_EQUIPMENT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::fishingRod, 5, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.01f, 0.0f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::nautilus, 5, 0, NULL, NULL, NULL));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.08f, 0.0f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DROWNED_EQUIPMENT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_DROWNED_RANGED_EQUIPMENT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::trident, 5, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::nautilus, 5, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.08f, 0.0f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&DROWNED_RANGED_EQUIPMENT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ELDER_GUARDIAN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::prismarineShard, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::fish, 3, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::prismarineCrystals, 2, 0, functions, NULL, NULL));
    entries->push_back(new EmptyLootItem(1, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::sponge), 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootTableReference(FISH, 1, 0, NULL, NULL));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ELDER_GUARDIAN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ENDERMAN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::enderPearl, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ENDERMAN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ENDERMITE()
{
    lootTables.insert(std::make_pair(&ENDERMITE, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_ENDER_DRAGON()
{
    lootTables.insert(std::make_pair(&ENDER_DRAGON, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_ENTITY_FISH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::fish, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(1, 2), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.25f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ENTITY_FISH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_EVOCATION_ILLAGER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::emerald, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 1)));
    entries->push_back(new LootItem(Items::totemOfUndying, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&EVOCATION_ILLAGER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_GHAST()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::ghastTear, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::gunpowder, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&GHAST, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_GIANT()
{
    lootTables.insert(std::make_pair(&GIANT, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_GUARDIAN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::prismarineShard, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::fish, 2, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::prismarineCrystals, 2, 0, functions, NULL, NULL));
    entries->push_back(new EmptyLootItem(1, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootTableReference(FISH, 1, 0, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&GUARDIAN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_HORSE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&HORSE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_HUSK()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::ironIngot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::carrot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::potato, 1, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&HUSK, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_IRON_GOLEM()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::red_flower), 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(3, 5)));
    entries->push_back(new LootItem(Items::ironIngot, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&IRON_GOLEM, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_LLAMA()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&LLAMA, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MAGMA_CUBE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::magmaCream, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MAGMA_CUBE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MOOSHROOM()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::beef, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MOOSHROOM, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MOOSHROOM_SHEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetDataFromColorIndexFunction(NULL));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 1)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::red_mushroom), 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(5), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MOOSHROOM_SHEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MULE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MULE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_MUSHROOM_COW()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::leather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::beef, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&MUSHROOM_COW, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_OCELOT()
{
    lootTables.insert(std::make_pair(&OCELOT, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_PARROT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    entries->push_back(new LootItem(Items::feather, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&PARROT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_PHANTOM()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::phantomMembrane, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&PHANTOM, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_PIG()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::porkchop, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&PIG, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_PIG_SADDLED()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::porkchop, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 1)));
    entries->push_back(new LootItem(Items::saddle, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&PIG_SADDLED, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_POLAR_BEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::fish, 3, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&POLAR_BEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_PUFFERFISH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(3)));
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(1, 2), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.25f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&PUFFERFISH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_RABBIT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rabbitHide, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    addSmeltItemWhenOnFireFunc(functions);
    entries->push_back(new LootItem(Items::rabbit, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::rabbitFoot, 1, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.1f, 0.03f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&RABBIT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SALMON_LARGE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(1, 2), 0));
    entries->push_back(new LootItem(Items::bone, 2, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.25f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SALMON_LARGE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SALMON_NORMAL()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(1, 2), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.25f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SALMON_NORMAL, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SEA_TURTLE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Item::byBlock(Blocks::sea_grass), 3, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SEA_TURTLE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHEEP()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetDataFromColorIndexFunction(NULL));
    entries->push_back(new LootItem(Item::byBlock(Blocks::wool), 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::mutton, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHEEP, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHEEP_SHEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetDataFromColorIndexFunction(NULL));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    entries->push_back(new LootItem(Item::byBlock(Blocks::wool), 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHEEP_SHEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHEEP_SHEARED()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 2)));
    addSmeltItemWhenOnFireFunc(functions);
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::mutton, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHEEP_SHEARED, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SHULKER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    entries->push_back(new LootItem(Items::shulkerShell, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SHULKER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SILVERFISH()
{
    lootTables.insert(std::make_pair(&SILVERFISH, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_SKELETON()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::arrow, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SKELETON, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SKELETON_GEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.1f));
    entries->push_back(new LootItem(Items::bow, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootTableReference(ARMOR_SET_LEATHER, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_GOLD, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_CHAIN, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_IRON, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_DIAMOND, 1, 0, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.0f, 1.0f, 1.0f));
    conditions->push_back(new LootItemRandomChanceWithSpecialModifierCondition(0.15f));
    pools->push_back(
        new LootPool(entries, conditions, RandomValueBounds(0), RandomValueBounds(0), new LootPoolTiers(2, 3, 0.095f)));
    lootTables.insert(std::make_pair(&SKELETON_GEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SKELETON_HORSE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SKELETON_HORSE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SLIME()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::slimeBall, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SLIME, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SNOWMAN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 15)));
    entries->push_back(new LootItem(Items::snowball, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SNOWMAN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SPIDER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::string, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::spiderEye, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SPIDER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_SQUID()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(1, 3)));
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(0)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::dye, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&SQUID, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_STRAY()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::arrow, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    CompoundTag* tag = new CompoundTag();
    tag->putString(PotionUtils::TAG_POTION, L"long_slowness");
    functions->push_back(new SetNBTFunction(NULL, tag));
    entries->push_back(new LootItem(Items::tippedArrow, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&STRAY, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_TROPICALFISH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(2)));
    entries->push_back(new LootItem(Items::fish, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(1, 2), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.25f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&TROPICALFISH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_VEX()
{
    lootTables.insert(std::make_pair(&VEX, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_VEX_GEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::ironSword, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&VEX_GEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_VILLAGER()
{
    lootTables.insert(std::make_pair(&VILLAGER, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_VINDICATION_ILLAGER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::emerald, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&VINDICATION_ILLAGER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_VINDICATOR_GEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    entries->push_back(new LootItem(Items::ironAxe, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&VINDICATOR_GEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WITCH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::glowstoneDust, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::sugar, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::redstone, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::spiderEye, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::glassBottle, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::gunpowder, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::stick, 2, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1, 3), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&WITCH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WITHER_BOSS()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::netherStar, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&WITHER_BOSS, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WITHER_SKELETON()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::coal, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::bone, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::skull, 1, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&WITHER_SKELETON, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WITHER_SKELETON_GEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::stoneSword, 1, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&WITHER_SKELETON_GEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_WOLF()
{
    lootTables.insert(std::make_pair(&WOLF, LootTable::EMPTY_LOOT_TABLE));
}

void BuiltInLootTables::staticLootTable_ZOMBIE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::ironIngot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::carrot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::potato, 1, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ZOMBIE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ZOMBIE_EQUIPMENT()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    entries->push_back(new LootItem(Items::ironSword, 1, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    entries->push_back(new LootItem(Items::ironShovel, 3, 0, functions, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomChanceWithSpecialModifierCondition(0.05f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootTableReference(ARMOR_SET_LEATHER, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_GOLD, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_CHAIN, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_IRON, 1, 0, NULL, NULL));
    entries->push_back(new LootTableReference(ARMOR_SET_DIAMOND, 1, 0, NULL, NULL));
    conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemRandomDifficultyChanceCondition(0.0f, 0.0f, 1.0f, 1.0f));
    conditions->push_back(new LootItemRandomChanceWithSpecialModifierCondition(0.15f));
    pools->push_back(
        new LootPool(entries, conditions, RandomValueBounds(0), RandomValueBounds(0), new LootPoolTiers(2, 3, 0.095f)));
    lootTables.insert(std::make_pair(&ZOMBIE_EQUIPMENT, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ZOMBIE_HORSE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ZOMBIE_HORSE, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ZOMBIE_PIGMAN()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 1)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::goldNugget, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::goldIngot, 1, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ZOMBIE_PIGMAN, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ZOMBIE_PIGMAN_GEAR()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    functions->push_back(new EnchantRandomEquipmentFunction(NULL, 0.25f));
    entries->push_back(new LootItem(Items::goldenSword, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ZOMBIE_PIGMAN_GEAR, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_ZOMBIE_VILLAGER()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(0, 2)));
    functions->push_back(new LootingEnchantFunction(NULL, RandomValueBounds(0, 1), 0));
    entries->push_back(new LootItem(Items::rottenFlesh, 1, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::ironIngot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::carrot, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::potato, 1, 0, NULL, NULL, NULL));
    std::vector<LootItemCondition*>* conditions = new std::vector<LootItemCondition*>;
    conditions->push_back(new LootItemKilledByPlayerCondition(false));
    conditions->push_back(new LootItemRandomChanceWithLootingCondition(0.025f, 0.01f));
    pools->push_back(new LootPool(entries, conditions, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&ZOMBIE_VILLAGER, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_LOW_TIER_ITEMS()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::leatherChestplate, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironChestplate, 1, 0, NULL, NULL, NULL));
    entries->push_back(new EmptyLootItem(5, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::leatherHelmet, 1, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::ironHelmet, 1, 0, NULL, NULL, NULL));
    entries->push_back(new EmptyLootItem(5, 0, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&LOW_TIER_ITEMS, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_FISHING()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootTableReference(JUNK, 10, -2, NULL, NULL));
    entries->push_back(new LootTableReference(TREASURE, 5, 2, NULL, NULL));
    entries->push_back(new LootTableReference(FISH, 85, -1, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&FISHING, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_FISH()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::fish, 60, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(1)));
    entries->push_back(new LootItem(Items::fish, 25, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(2)));
    entries->push_back(new LootItem(Items::fish, 2, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(3)));
    entries->push_back(new LootItem(Items::fish, 13, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&FISH, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_JUNK()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDamageFunction(NULL, RandomValueBounds(0, 0.9)));
    entries->push_back(new LootItem(Items::leatherBoots, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::leather, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::bone, 10, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    CompoundTag* tag = new CompoundTag();
    tag->putString(PotionUtils::TAG_POTION, L"water");
    functions->push_back(new SetNBTFunction(NULL, tag));
    entries->push_back(new LootItem(Items::potion, 10, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::string, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDamageFunction(NULL, RandomValueBounds(0, 0.9)));
    entries->push_back(new LootItem(Items::fishingRod, 2, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Items::bowl, 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::stick, 5, 0, NULL, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDataFunction(NULL, RandomValueBounds(0)));
    functions->push_back(new SetItemCountFunction(NULL, RandomValueBounds(10)));
    entries->push_back(new LootItem(Items::dye, 1, 0, functions, NULL, NULL));
    entries->push_back(new LootItem(Item::byBlock(Blocks::tripwire_hook), 10, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::rottenFlesh, 10, 0, NULL, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&JUNK, new LootTable(pools)));
}

void BuiltInLootTables::staticLootTable_TREASURE()
{
    std::vector<LootPool*>* pools = new std::vector<LootPool*>;
    std::vector<LootPoolEntry*>* entries = new std::vector<LootPoolEntry*>;
    entries->push_back(new LootItem(Items::nautilus, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Item::byBlock(Blocks::waterlily), 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::nameTag, 5, 0, NULL, NULL, NULL));
    entries->push_back(new LootItem(Items::saddle, 5, 0, NULL, NULL, NULL));
    std::vector<LootItemFunction*>* functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDamageFunction(NULL, RandomValueBounds(0, 0.25)));
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::bow, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new SetItemDamageFunction(NULL, RandomValueBounds(0, 0.25)));
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::fishingRod, 5, 0, functions, NULL, NULL));
    functions = new std::vector<LootItemFunction*>;
    functions->push_back(new EnchantWithLevelsFunction(NULL, RandomValueBounds(30), true));
    entries->push_back(new LootItem(Items::book, 6, 0, functions, NULL, NULL));
    pools->push_back(new LootPool(entries, NULL, RandomValueBounds(1), RandomValueBounds(0), NULL));
    lootTables.insert(std::make_pair(&TREASURE, new LootTable(pools)));
}

void BuiltInLootTables::staticCtor()
{
    staticLootTable_ABANDONED_MINESHAFT();
    staticLootTable_BURIEDTREASURE();
    staticLootTable_DESERT_PYRAMID();
    staticLootTable_DISPENSER_TRAP();
    staticLootTable_END_CITY_TREASURE();
    staticLootTable_IGLOO_CHEST();
    staticLootTable_JUNGLE_TEMPLE();
    staticLootTable_JUNGLE_TEMPLE_DISPENSER();
    staticLootTable_MONSTER_ROOM();
    staticLootTable_NETHER_BRIDGE();
    staticLootTable_SHIPWRECK();
    staticLootTable_SHIPWRECKSUPPLY();
    staticLootTable_SHIPWRECKTREASURE();
    staticLootTable_SIMPLE_DUNGEON();
    staticLootTable_SPAWN_BONUS_CHEST();
    staticLootTable_STRONGHOLD_CORRIDOR();
    staticLootTable_STRONGHOLD_CROSSING();
    staticLootTable_STRONGHOLD_LIBRARY();
    staticLootTable_UNDERWATER_RUIN_BIG();
    staticLootTable_UNDERWATER_RUIN_SMALL();
    staticLootTable_VILLAGE_BLACKSMITH();
    staticLootTable_VILLAGE_TWO_ROOM_HOUSE();
    staticLootTable_WOODLAND_MANSION();
    staticLootTable_ARMOR_SET_CHAIN();
    staticLootTable_ARMOR_SET_DIAMOND();
    staticLootTable_ARMOR_SET_GOLD();
    staticLootTable_ARMOR_SET_IRON();
    staticLootTable_ARMOR_SET_LEATHER();
    staticLootTable_ARMOR_STAND();
    staticLootTable_BAT();
    staticLootTable_BLAZE();
    staticLootTable_BOAT();
    staticLootTable_CAVE_SPIDER();
    staticLootTable_CHICKEN();
    staticLootTable_CLOWNFISH();
    staticLootTable_COW();
    staticLootTable_CREEPER();
    staticLootTable_DOLPHIN();
    staticLootTable_DONKEY();
    staticLootTable_DROWNED();
    staticLootTable_DROWNED_EQUIPMENT();
    staticLootTable_DROWNED_RANGED_EQUIPMENT();
    staticLootTable_ELDER_GUARDIAN();
    staticLootTable_ENDERMAN();
    staticLootTable_ENDERMITE();
    staticLootTable_ENDER_DRAGON();
    staticLootTable_ENTITY_FISH();
    staticLootTable_EVOCATION_ILLAGER();
    staticLootTable_GHAST();
    staticLootTable_GIANT();
    staticLootTable_GUARDIAN();
    staticLootTable_HORSE();
    staticLootTable_HUSK();
    staticLootTable_IRON_GOLEM();
    staticLootTable_LLAMA();
    staticLootTable_MAGMA_CUBE();
    staticLootTable_MOOSHROOM();
    staticLootTable_MOOSHROOM_SHEAR();
    staticLootTable_MULE();
    staticLootTable_MUSHROOM_COW();
    staticLootTable_OCELOT();
    staticLootTable_PARROT();
    staticLootTable_PHANTOM();
    staticLootTable_PIG();
    staticLootTable_PIG_SADDLED();
    staticLootTable_POLAR_BEAR();
    staticLootTable_PUFFERFISH();
    staticLootTable_RABBIT();
    staticLootTable_SALMON_LARGE();
    staticLootTable_SALMON_NORMAL();
    staticLootTable_SEA_TURTLE();
    staticLootTable_SHEEP();
    staticLootTable_SHEEP_SHEAR();
    staticLootTable_SHEEP_SHEARED();
    staticLootTable_SHULKER();
    staticLootTable_SILVERFISH();
    staticLootTable_SKELETON();
    staticLootTable_SKELETON_GEAR();
    staticLootTable_SKELETON_HORSE();
    staticLootTable_SLIME();
    staticLootTable_SNOWMAN();
    staticLootTable_SPIDER();
    staticLootTable_SQUID();
    staticLootTable_STRAY();
    staticLootTable_TROPICALFISH();
    staticLootTable_VEX();
    staticLootTable_VEX_GEAR();
    staticLootTable_VILLAGER();
    staticLootTable_VINDICATION_ILLAGER();
    staticLootTable_VINDICATOR_GEAR();
    staticLootTable_WITCH();
    staticLootTable_WITHER_BOSS();
    staticLootTable_WITHER_SKELETON();
    staticLootTable_WITHER_SKELETON_GEAR();
    staticLootTable_WOLF();
    staticLootTable_ZOMBIE();
    staticLootTable_ZOMBIE_EQUIPMENT();
    staticLootTable_ZOMBIE_HORSE();
    staticLootTable_ZOMBIE_PIGMAN();
    staticLootTable_ZOMBIE_PIGMAN_GEAR();
    staticLootTable_ZOMBIE_VILLAGER();
    staticLootTable_LOW_TIER_ITEMS();
    staticLootTable_FISHING();
    staticLootTable_FISH();
    staticLootTable_JUNK();
    staticLootTable_TREASURE();
}

LootTable* BuiltInLootTables::getLootTable(ResourceLocation* location)
{
    return lootTables[location];
}

boost::unordered_map<ResourceLocation*, LootTable*>* BuiltInLootTables::getLootTables()
{
    return &lootTables;
}
