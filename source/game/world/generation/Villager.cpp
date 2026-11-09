#include "game/world/generation/Villager.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/core/UniqueTypeWalker.hpp"
#include "game/core/Vex.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/core/Witch.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/InteractGoal.hpp"
#include "game/entity/ai/MakeLoveGoal.hpp"
#include "game/entity/ai/MoveIndoorsGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/OpenDoorGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/PlayGoal.hpp"
#include "game/entity/ai/RestrictOpenDoorGoal.hpp"
#include "game/entity/ai/TakeFlowerGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HarvestFarmBlockGoal.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/LookAtTradingPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/generation/TradeWithPlayerGoal.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/VillagerInteractGoal.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/item/ItemId.hpp"


const EntityDataAccessor<int>* Villager::DATA_PROFESSION = NULL;
const EntityDataAccessor<int>* Villager::DATA_CAREER = NULL;
const EntityDataAccessor<int>* Villager::DATA_TRADING = NULL;
std::vector<std::vector<std::vector<arrayWithLength<Villager::ItemListing*> > > > Villager::sTrades;

Villager::PriceRange::PriceRange()
{
    mMin = 0;
    mMax = 0;
}

Villager::PriceRange::PriceRange(int min, int max)
{
    mMin = min;
    mMax = max;
}

int Villager::PriceRange::getValue(Random* random)
{
    if (mMin >= mMax)
    {
        return mMin;
    }

    return mMin + random->nextInt(mMax - mMin + 1);
}

Villager::EmeraldForItems::EmeraldForItems(Item* item, PriceRange price)
{
    mItem = item;
    mPrice = price;
}

void Villager::EmeraldForItems::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes,
                                                  Random* random)
{
    recipes->push_back(new MerchantRecipe(
        not_null_ptr<ItemInstance>(new ItemInstance(mItem, mPrice.getValue(random), 0)), Items::emerald));
}

Villager::ItemForEmeralds::ItemForEmeralds(Item* item, const PriceRange& price) : mItem(item)
{
    mPrice = price;
}

Villager::ItemForEmeralds::ItemForEmeralds(const ItemInstance& item, const PriceRange& price) : mItem(item)
{
    mPrice = price;
}

void Villager::ItemForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes,
                                                  Random* random)
{
    int value = mPrice.getValue(random);
    not_null_ptr<ItemInstance> buy = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> sell = ItemInstance::EMPTY;

    if (value < 0)
    {
        buy = new ItemInstance(Items::emerald, 1, 0);
        sell = new ItemInstance(mItem.getItem(), -value, mItem.getAuxValue());
    }
    else
    {
        buy = new ItemInstance(Items::emerald, value, 0);
        sell = new ItemInstance(mItem.getItem(), 1, mItem.getAuxValue());
    }

    recipes->push_back(new MerchantRecipe(buy, sell));
}

Villager::EnchantedItemForEmeralds::EnchantedItemForEmeralds(Item* item, const PriceRange& price) : mItem(item)
{
    mPrice = price;
}

void Villager::EnchantedItemForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes,
                                                           Random* random)
{
    not_null_ptr<ItemInstance> buy(new ItemInstance(Items::emerald, mPrice.getValue(random), 0));
    not_null_ptr<ItemInstance> sell(new ItemInstance(mItem.getItem(), 1, mItem.getAuxValue()));
    sell = EnchantmentHelper::enchantItem(random, sell, random->nextInt(15) + 5, false);
    recipes->push_back(new MerchantRecipe(buy, sell));
}

Villager::ItemAndEmeraldToItem::ItemAndEmeraldToItem(Item* firstItem, const PriceRange& firstPrice, Item* secondItem,
                                                     const PriceRange& secondPrice)
    : mFirstItem(firstItem),
      mSecondItem(secondItem)
{
    mFirstPrice = firstPrice;
    mSecondPrice = secondPrice;
}

void Villager::ItemAndEmeraldToItem::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes,
                                                       Random* random)
{
    int firstCount = mFirstPrice.getValue(random);
    int secondCount = mSecondPrice.getValue(random);
    recipes->push_back(new MerchantRecipe(
        not_null_ptr<ItemInstance>(new ItemInstance(mFirstItem.getItem(), firstCount, mFirstItem.getAuxValue())),
        not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald)),
        not_null_ptr<ItemInstance>(new ItemInstance(mSecondItem.getItem(), secondCount, mSecondItem.getAuxValue())), 0,
        7));
}

Villager::EnchantBookForEmeralds::EnchantBookForEmeralds()
{
}

void Villager::EnchantBookForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes,
                                                         Random* random)
{
    Enchantment* enchantment = Enchantment::enchantmentRegistry->getRandom(random);
    int level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());
    not_null_ptr<ItemInstance> enchantedBook = EnchantedBookItem::createForEnchantment(
        new EnchantmentInstance(enchantment, level));
    int cost = random->nextInt(level * 10 + 5) + 2 + level * 3;

    if (enchantment->isTreasureOnly())
    {
        cost *= 2;
    }

    if (cost > 64)
    {
        cost = 64;
    }

    recipes->push_back(new MerchantRecipe(not_null_ptr<ItemInstance>(new ItemInstance(Items::book)),
                                          not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald, cost)),
                                          enchantedBook, 0, 7));
}

Villager::TreasureMapForEmeralds::TreasureMapForEmeralds(const PriceRange& price, std::wstring name,
                                                         const MapDecoration::Type* icon)
{
    mPrice = price;
    mName = name;
    mIcon = icon;
}

void Villager::TreasureMapForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant> merchant,
                                                         MerchantRecipeList* recipes, Random* random)
{
    int value = mPrice.getValue(random);
    Level* level = merchant->getLevel();
    BlockPos position = merchant->getPosition();
    BlockPos* target = level->findNearestMapFeature(mName, position, false);

    if (target != NULL)
    {
        not_null_ptr<ItemInstance> map = MapItem::setupNewMap(PlayerUID(0), level, 0, (double)target->x,
                                                              (double)target->z, 3, true, true);
        MapItem::renderBiomePreviewMap(level, map);
        MapItemSavedData::addTargetDecoration(map, *target, L"+", mIcon);

        if (mName == LargeFeature::MONUMENT)
        {
            map->setTranslatableName(IDS_ITEM_FILLED_MAP_OCEAN_MONUMENT);
        }
        else
        {
            map->setTranslatableName(mName == LargeFeature::MANSION ? IDS_ITEM_FILLED_MAP_WOODLAND_MANSION
                                                                    : IDS_DESC_MAP);
        }

        recipes->push_back(new MerchantRecipe(not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald, value)),
                                              not_null_ptr<ItemInstance>(new ItemInstance(Items::compass)), map, 0, 7));
    }
}

Villager::Villager(Level* level) : AgableMob(level)
{
    _init(0);
}

Villager::~Villager()
{
    if (mInventory != NULL)
    {
        delete mInventory;
    }

    delete mOffers;
}

Entity* Villager::create(Level* level)
{
    return new Villager(level);
}

void Villager::_init(int profession)
{
    fjDerivedCtorCalls();
    setProfession(profession);
    setSize(0.6f, 1.95f);
    mRandomTickDivider = 0;
    mInLove = false;
    mChasing = false;
    mVillage = boost::weak_ptr<Village>();
    mTradingPlayer = boost::weak_ptr<Player>();
    mOffers = NULL;
    mTimeUntilReset = 0;
    mNeedsInitialization = false;
    mWillingToBreed = false;
    mRiches = 0;
    mLastTradedPlayerName = L"";
    mRewardPlayersInVillage = false;
    mCareerLevel = 0;
    mAdditionalTasksSet = false;
    mInventory = new SimpleContainer(IDS_ITEMS, L"", false, 8);
    static_cast<GroundPathNavigation*>(getNavigation())->setCanOpenDoors(true);
    setCanPickUpLoot(true);
    mCategories |= EntityCategory::eEntityCategory_Villager;
}

void Villager::staticCtor()
{
    DATA_PROFESSION = SynchedEntityData::defineId(EntityType::VILLAGER, EntityType::AGABLE_MOB,
                                                  EntityDataSerializers::INT);
    DATA_CAREER = SynchedEntityData::defineId(EntityType::VILLAGER, EntityType::AGABLE_MOB, EntityDataSerializers::INT);
    DATA_TRADING = SynchedEntityData::defineId(EntityType::VILLAGER, EntityType::AGABLE_MOB,
                                               EntityDataSerializers::INT);

    std::vector<std::vector<arrayWithLength<ItemListing*> > > profession;
    std::vector<arrayWithLength<ItemListing*> > career;

    static ItemListing* farmerTier1[] = {
        new EmeraldForItems(Items::wheat, PriceRange(18, 22)), new EmeraldForItems(Items::potato, PriceRange(15, 19)),
        new EmeraldForItems(Items::carrot, PriceRange(15, 19)), new ItemForEmeralds(Items::bread, PriceRange(-4, -2))};
    career.push_back(arrayWithLength<ItemListing*>(farmerTier1, 4));

    static ItemListing* farmerTier2[] = {new EmeraldForItems(Item::byBlock(Blocks::pumpkin), PriceRange(8, 13)),
                                         new ItemForEmeralds(Items::pumpkinPie, PriceRange(-3, -2))};
    career.push_back(arrayWithLength<ItemListing*>(farmerTier2, 2));

    static ItemListing* farmerTier3[] = {new EmeraldForItems(Item::byBlock(Blocks::melon_block), PriceRange(7, 12)),
                                         new ItemForEmeralds(Items::apple, PriceRange(-7, -5))};
    career.push_back(arrayWithLength<ItemListing*>(farmerTier3, 2));

    static ItemListing* farmerTier4[] = {new ItemForEmeralds(Items::cookie, PriceRange(-10, -6)),
                                         new ItemForEmeralds(Items::cake, PriceRange(1, 1))};
    career.push_back(arrayWithLength<ItemListing*>(farmerTier4, 2));
    profession.push_back(career);
    career.clear();

    static ItemListing* fishermanTier1[] = {
        new EmeraldForItems(Items::string, PriceRange(15, 20)), new EmeraldForItems(Items::coal, PriceRange(16, 24)),
        new ItemAndEmeraldToItem(Items::fish, PriceRange(6, 6), Items::cookedFish, PriceRange(6, 6))};
    career.push_back(arrayWithLength<ItemListing*>(fishermanTier1, 3));

    static ItemListing* fishermanTier2[] = {new EnchantedItemForEmeralds(Items::fishingRod, PriceRange(7, 8))};
    career.push_back(arrayWithLength<ItemListing*>(fishermanTier2, 1));
    profession.push_back(career);
    career.clear();

    static ItemListing* shepherdTier1[] = {new EmeraldForItems(Item::byBlock(Blocks::wool), PriceRange(16, 22)),
                                           new ItemForEmeralds(Items::shears, PriceRange(3, 4))};
    career.push_back(arrayWithLength<ItemListing*>(shepherdTier1, 2));

    static ItemListing* shepherdTier2[] = {
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 0), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 1), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 2), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 3), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 4), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 5), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 6), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 7), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 8), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 9), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 10), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 11), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 12), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 13), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 14), PriceRange(1, 2)),
        new ItemForEmeralds(ItemInstance(Item::byBlock(Blocks::wool), 1, 15), PriceRange(1, 2))};
    career.push_back(arrayWithLength<ItemListing*>(shepherdTier2, 16));
    profession.push_back(career);
    career.clear();

    static ItemListing* fletcherTier1[] = {new EmeraldForItems(Items::string, PriceRange(15, 20)),
                                           new ItemForEmeralds(Items::arrow, PriceRange(-12, -8))};
    career.push_back(arrayWithLength<ItemListing*>(fletcherTier1, 2));

    static ItemListing* fletcherTier2[] = {
        new ItemForEmeralds(Items::bow, PriceRange(2, 3)),
        new ItemAndEmeraldToItem(Item::byBlock(Blocks::gravel), PriceRange(10, 10), Items::flint, PriceRange(6, 10))};
    career.push_back(arrayWithLength<ItemListing*>(fletcherTier2, 2));
    profession.push_back(career);
    career.clear();

    sTrades.push_back(profession);
    profession.clear();

    static ItemListing* librarianTier1[] = {new EmeraldForItems(Items::paper, PriceRange(24, 36)),
                                            new EnchantBookForEmeralds()};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier1, 2));

    static ItemListing* librarianTier2[] = {new EmeraldForItems(Items::book, PriceRange(8, 10)),
                                            new ItemForEmeralds(Items::compass, PriceRange(10, 12)),
                                            new ItemForEmeralds(Item::byBlock(Blocks::bookshelf), PriceRange(3, 4))};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier2, 3));

    static ItemListing* librarianTier3[] = {new EmeraldForItems(Items::writtenBook, PriceRange(2, 2)),
                                            new ItemForEmeralds(Items::clock, PriceRange(10, 12)),
                                            new ItemForEmeralds(Item::byBlock(Blocks::glass), PriceRange(-5, -3))};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier3, 3));

    static ItemListing* librarianTier4[] = {new EnchantBookForEmeralds()};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier4, 1));

    static ItemListing* librarianTier5[] = {new EnchantBookForEmeralds()};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier5, 1));

    static ItemListing* librarianTier6[] = {new ItemForEmeralds(Items::nameTag, PriceRange(20, 22))};
    career.push_back(arrayWithLength<ItemListing*>(librarianTier6, 1));
    profession.push_back(career);
    career.clear();

    static ItemListing* cartographerTier1[] = {new EmeraldForItems(Items::paper, PriceRange(24, 36))};
    career.push_back(arrayWithLength<ItemListing*>(cartographerTier1, 1));

    static ItemListing* cartographerTier2[] = {new EmeraldForItems(Items::compass, PriceRange(1, 1))};
    career.push_back(arrayWithLength<ItemListing*>(cartographerTier2, 1));

    static ItemListing* cartographerTier3[] = {new ItemForEmeralds(Items::map, PriceRange(7, 11))};
    career.push_back(arrayWithLength<ItemListing*>(cartographerTier3, 1));

    static ItemListing* cartographerTier4[] = {
        new TreasureMapForEmeralds(PriceRange(12, 20), LargeFeature::MONUMENT, &MapDecoration::Type::TYPES[15]),
        new TreasureMapForEmeralds(PriceRange(16, 28), LargeFeature::MANSION, &MapDecoration::Type::TYPES[14])};
    career.push_back(arrayWithLength<ItemListing*>(cartographerTier4, 2));
    profession.push_back(career);
    career.clear();

    sTrades.push_back(profession);
    profession.clear();

    static ItemListing* clericTier1[] = {new EmeraldForItems(Items::rottenFlesh, PriceRange(36, 40)),
                                         new EmeraldForItems(Items::goldIngot, PriceRange(8, 10))};
    career.push_back(arrayWithLength<ItemListing*>(clericTier1, 2));

    static ItemListing* clericTier2[] = {
        new ItemForEmeralds(Items::redstone, PriceRange(-4, -1)),
        new ItemForEmeralds(ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()), PriceRange(-2, -1))};
    career.push_back(arrayWithLength<ItemListing*>(clericTier2, 2));

    static ItemListing* clericTier3[] = {new ItemForEmeralds(Items::enderPearl, PriceRange(4, 7)),
                                         new ItemForEmeralds(Item::byBlock(Blocks::glowstone), PriceRange(-3, -1))};
    career.push_back(arrayWithLength<ItemListing*>(clericTier3, 2));

    static ItemListing* clericTier4[] = {new ItemForEmeralds(Items::experienceBottle, PriceRange(3, 11))};
    career.push_back(arrayWithLength<ItemListing*>(clericTier4, 1));
    profession.push_back(career);
    career.clear();

    sTrades.push_back(profession);
    profession.clear();

    static ItemListing* armorerTier1[] = {new EmeraldForItems(Items::coal, PriceRange(16, 24)),
                                          new ItemForEmeralds(Items::ironHelmet, PriceRange(4, 6))};
    career.push_back(arrayWithLength<ItemListing*>(armorerTier1, 2));

    static ItemListing* armorerTier2[] = {new EmeraldForItems(Items::ironIngot, PriceRange(7, 9)),
                                          new ItemForEmeralds(Items::ironChestplate, PriceRange(10, 14))};
    career.push_back(arrayWithLength<ItemListing*>(armorerTier2, 2));

    static ItemListing* armorerTier3[] = {new EmeraldForItems(Items::diamond, PriceRange(3, 4)),
                                          new EnchantedItemForEmeralds(Items::diamondChestplate, PriceRange(16, 19))};
    career.push_back(arrayWithLength<ItemListing*>(armorerTier3, 2));

    static ItemListing* armorerTier4[] = {new ItemForEmeralds(Items::chainmailBoots, PriceRange(5, 7)),
                                          new ItemForEmeralds(Items::chainmailLeggings, PriceRange(9, 11)),
                                          new ItemForEmeralds(Items::chainmailHelmet, PriceRange(5, 7)),
                                          new ItemForEmeralds(Items::chainmailChestplate, PriceRange(11, 15))};
    career.push_back(arrayWithLength<ItemListing*>(armorerTier4, 4));
    profession.push_back(career);
    career.clear();

    static ItemListing* weaponSmithTier1[] = {new EmeraldForItems(Items::coal, PriceRange(16, 24)),
                                              new ItemForEmeralds(Items::ironAxe, PriceRange(6, 8))};
    career.push_back(arrayWithLength<ItemListing*>(weaponSmithTier1, 2));

    static ItemListing* weaponSmithTier2[] = {new EmeraldForItems(Items::ironIngot, PriceRange(7, 9)),
                                              new EnchantedItemForEmeralds(Items::ironSword, PriceRange(9, 10))};
    career.push_back(arrayWithLength<ItemListing*>(weaponSmithTier2, 2));

    static ItemListing* weaponSmithTier3[] = {new EmeraldForItems(Items::diamond, PriceRange(3, 4)),
                                              new EnchantedItemForEmeralds(Items::diamondSword, PriceRange(12, 15)),
                                              new EnchantedItemForEmeralds(Items::diamondAxe, PriceRange(9, 12))};
    career.push_back(arrayWithLength<ItemListing*>(weaponSmithTier3, 3));
    profession.push_back(career);
    career.clear();

    static ItemListing* toolSmithTier1[] = {new EmeraldForItems(Items::coal, PriceRange(16, 24)),
                                            new EnchantedItemForEmeralds(Items::ironShovel, PriceRange(5, 7))};
    career.push_back(arrayWithLength<ItemListing*>(toolSmithTier1, 2));

    static ItemListing* toolSmithTier2[] = {new EmeraldForItems(Items::ironIngot, PriceRange(7, 9)),
                                            new EnchantedItemForEmeralds(Items::ironPickaxe, PriceRange(9, 11))};
    career.push_back(arrayWithLength<ItemListing*>(toolSmithTier2, 2));

    static ItemListing* toolSmithTier3[] = {new EmeraldForItems(Items::diamond, PriceRange(3, 4)),
                                            new EnchantedItemForEmeralds(Items::diamondPickaxe, PriceRange(12, 15))};
    career.push_back(arrayWithLength<ItemListing*>(toolSmithTier3, 2));
    profession.push_back(career);
    career.clear();

    sTrades.push_back(profession);
    profession.clear();

    static ItemListing* butcherTier1[] = {new EmeraldForItems(Items::porkchop, PriceRange(14, 18)),
                                          new EmeraldForItems(Items::chicken, PriceRange(14, 18))};
    career.push_back(arrayWithLength<ItemListing*>(butcherTier1, 2));

    static ItemListing* butcherTier2[] = {new EmeraldForItems(Items::coal, PriceRange(16, 24)),
                                          new ItemForEmeralds(Items::cookedPorkchop, PriceRange(-7, -5)),
                                          new ItemForEmeralds(Items::cookedChicken, PriceRange(-8, -6))};
    career.push_back(arrayWithLength<ItemListing*>(butcherTier2, 3));
    profession.push_back(career);
    career.clear();

    static ItemListing* leatherworkerTier1[] = {new EmeraldForItems(Items::leather, PriceRange(9, 12)),
                                                new ItemForEmeralds(Items::leatherLeggings, PriceRange(2, 4))};
    career.push_back(arrayWithLength<ItemListing*>(leatherworkerTier1, 2));

    static ItemListing* leatherworkerTier2[] = {
        new EnchantedItemForEmeralds(Items::leatherChestplate, PriceRange(7, 12))};
    career.push_back(arrayWithLength<ItemListing*>(leatherworkerTier2, 1));

    static ItemListing* leatherworkerTier3[] = {new ItemForEmeralds(Items::saddle, PriceRange(8, 10))};
    career.push_back(arrayWithLength<ItemListing*>(leatherworkerTier3, 1));
    profession.push_back(career);
    career.clear();

    sTrades.push_back(profession);
    profession.clear();

    sTrades.push_back(profession);
}

void Villager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::VILLAGER);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(EntityType::VILLAGER, L"Inventory"));
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new UniqueTypeWalker(EntityIO::getKey(EntityType::VILLAGER)));
}

unsigned long long Villager::GetType()
{
    return EntityType::VILLAGER;
}

void Villager::defineSynchedData()
{
    AgableMob::defineSynchedData();
    mEntityData->createDateItem(DATA_PROFESSION, 0);
    mEntityData->createDateItem(DATA_CAREER, 0);
    mEntityData->createDateItem(DATA_TRADING, 0);
}

int Villager::getProfession()
{
    int profession = mEntityData->get(DATA_PROFESSION) % 6;

    if (profession < 0)
    {
        profession = 0;
    }

    return profession;
}

void Villager::setProfession(int profession)
{
    mEntityData->set(DATA_PROFESSION, profession, true);
}

unsigned int Villager::getCareer()
{
    return mEntityData->get(DATA_CAREER);
}

void Villager::setCareer(int career)
{
    mEntityData->set(DATA_CAREER, career, true);
}

unsigned int Villager::getTrading()
{
    return mEntityData->get(DATA_TRADING);
}

void Villager::setTrading(int trading)
{
    mEntityData->set(DATA_TRADING, trading, true);
}

bool Villager::isTrading()
{
    return mTradingPlayer.lock().get() != NULL;
}

std::wstring Villager::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }

    const wchar_t* name;

    switch (getProfession())
    {
    case 0:
        if (getCareer() == 1)
            name = L10N::GetString(IDS_VILLAGER_FARMER);
        else if (getCareer() == 2)
            name = L10N::GetString(IDS_VILLAGER_FISHERMAN);
        else if (getCareer() == 3)
            name = L10N::GetString(IDS_VILLAGER_SHEPHERD);
        else if (getCareer() == 4)
            name = L10N::GetString(IDS_VILLAGER_FLETCHER);
        else
            name = L10N::GetString(IDS_VILLAGER);
        break;
    case 1:
        if (getCareer() == 1)
            name = L10N::GetString(IDS_VILLAGER_LIBRARIAN);
        else if (getCareer() == 2)
            name = L10N::GetString(IDS_VILLAGER_CARTOGRAPHER);
        else
            name = L10N::GetString(IDS_VILLAGER);
        break;
    case 2:
        name = L10N::GetString(IDS_VILLAGER_CLERIC);
        break;
    case 3:
        if (getCareer() == 1)
            name = L10N::GetString(IDS_VILLAGER_ARMORER);
        else if (getCareer() == 2)
            name = L10N::GetString(IDS_VILLAGER_WEAPONSMITH);
        else if (getCareer() == 3)
            name = L10N::GetString(IDS_VILLAGER_TOOLSMITH);
        else
            name = L10N::GetString(IDS_VILLAGER);
        break;
    case 4:
        if (getCareer() == 1)
            name = L10N::GetString(IDS_VILLAGER_BUTCHER);
        else if (getCareer() == 2)
            name = L10N::GetString(IDS_VILLAGER_LEATHERWORKER);
        else
            name = L10N::GetString(IDS_VILLAGER);
        break;
    case 5:
        name = L10N::GetString(IDS_VILLAGER_NITWIT);
        break;
    default:
        name = L10N::GetString(IDS_VILLAGER);
        break;
    }

    return name;
}

void Villager::setTradingPlayer(const boost::shared_ptr<Player>& player)
{
    setTrading(player.get() != NULL);
    mTradingPlayer = boost::weak_ptr<Player>(player);
}

boost::shared_ptr<Player> Villager::getTradingPlayer()
{
    return mTradingPlayer.lock();
}

void Villager::increaseMerchantCareer()
{
    std::vector<std::vector<arrayWithLength<ItemListing*> > >& careers = sTrades[getProfession()];

    if (getCareer() != 0 && mCareerLevel != 0)
    {
        mCareerLevel++;
    }
    else
    {
        if (careers.empty())
        {
            setCareer(1);
            mCareerLevel = 1;
        }
        else
        {
            setCareer(mRandom->nextInt(careers.size()) + 1);
            mCareerLevel = 1;
        }
    }

    if (mOffers == NULL)
    {
        mOffers = new MerchantRecipeList();
    }

    unsigned int careerIndex = getCareer() - 1;
    unsigned int levelIndex = mCareerLevel - 1;

    if ((int)careerIndex < 0 || careerIndex >= careers.size())
    {
        return;
    }

    std::vector<arrayWithLength<ItemListing*> >& tiers = careers[careerIndex];

    if ((int)levelIndex < 0 || levelIndex >= tiers.size())
    {
        return;
    }

    arrayWithLength<ItemListing*> listings = tiers[levelIndex];

    for (unsigned int i = 0; i < listings.length; i++)
    {
        listings.data[i]->addMerchantRecipe(boost::dynamic_pointer_cast<Merchant>(shared_from_this()), mOffers,
                                            mRandom);
    }
}

MerchantRecipeList* Villager::getOffers(const boost::shared_ptr<Player>&)
{
    if (mOffers == NULL)
    {
        increaseMerchantCareer();
    }

    return mOffers;
}

void Villager::overrideOffers(MerchantRecipeList*)
{
}

void Villager::notifyTrade(MerchantRecipe* recipe)
{
    recipe->increaseUses();
    mAmbientSoundTime = -getAmbientSoundInterval();
    playSound(SoundEvents::VILLAGER_YES, getSoundVolume(), getVoicePitch());
    int experience = mRandom->nextInt(4) + 3;

    if (recipe->getUses() == 1 || mRandom->nextInt(5) == 0)
    {
        mTimeUntilReset = 40;
        mWillingToBreed = true;
        mNeedsInitialization = true;

        boost::shared_ptr<Player> tradingPlayer = mTradingPlayer.lock();

        if (tradingPlayer != NULL)
        {
            mLastTradedPlayerName = tradingPlayer->getName();
        }
        else
        {
            mLastTradedPlayerName = L"";
        }

        experience += 5;
    }

    if (recipe->getBuyAItem()->getItem() == Items::emerald)
    {
        mRiches += recipe->getBuyAItem()->getCount();
    }

    if (recipe->shouldRewardExp())
    {
        boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(mLevel, mX, mY + 0.5, mZ, experience));
        mLevel->addEntity(orb);
    }
}

void Villager::notifyTradeUpdated(const not_null_ptr<ItemInstance>& item)
{
    if (!mLevel->mIsClientSide && mAmbientSoundTime > 20 - getAmbientSoundInterval())
    {
        mAmbientSoundTime = -getAmbientSoundInterval();

        if (item->isEmpty())
        {
            playSound(SoundEvents::VILLAGER_NO, getSoundVolume(), getVoicePitch());
        }
        else
        {
            playSound(SoundEvents::VILLAGER_YES, getSoundVolume(), getVoicePitch());
        }
    }
}

bool Villager::isAvailableToTrade()
{
    return !mRemoved;
}

Level* Villager::getLevel()
{
    return mLevel;
}

BlockPos Villager::getPosition()
{
    return BlockPos(this);
}

MobGroupData* Villager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                         bool randomProfession, const std::wstring&)
{
    data = Mob::finalizeMobSpawn(difficulty, data, L"");

    if (randomProfession)
    {
        setProfession(mLevel->mRandom->nextInt(6));
    }

    loadPersonalAiGoals();
    increaseMerchantCareer();
    return data;
}

MobGroupData* Villager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                         const std::wstring& dataString)
{
    return finalizeMobSpawn(difficulty, data, true, dataString);
}

void Villager::setRewardPlayersInVillage()
{
    mRewardPlayersInVillage = true;
}

bool Villager::countFood(int multiplier)
{
    int profession = getProfession();

    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); slot++)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);

        if (!item->isEmpty())
        {
            if ((item->getItem() == Items::bread && item->getCount() >= multiplier * 3)
                || (item->getItem() == Items::potato && item->getCount() >= multiplier * 12)
                || (item->getItem() == Items::carrot && item->getCount() >= multiplier * 12)
                || (item->getItem() == Items::beetroot && item->getCount() >= multiplier * 12))
            {
                return true;
            }

            if (profession == 0 && item->getItem() == Items::wheat && item->getCount() >= multiplier * 9)
            {
                return true;
            }
        }
    }

    return false;
}

bool Villager::hasExcessFood()
{
    return countFood(2);
}

bool Villager::wantsMoreFood()
{
    bool farmer = getProfession() == 0;

    if (farmer)
    {
        return !countFood(5);
    }

    return !countFood(1);
}

SimpleContainer* Villager::getInventory()
{
    return mInventory;
}

void Villager::loadPersonalAiGoals()
{
    if (!mAdditionalTasksSet)
    {
        mAdditionalTasksSet = true;

        if (isBaby())
        {
            mGoalSelector.addGoal(8, new PlayGoal(this, 0.32), true);
        }
        else if (getProfession() == 0)
        {
            mGoalSelector.addGoal(6, new HarvestFarmBlockGoal(this, 0.6), true);
        }
    }
}

void Villager::grewUp()
{
    if (getProfession() == 0)
    {
        mGoalSelector.addGoal(8, new HarvestFarmBlockGoal(this, 0.6), true);
    }

    AgableMob::grewUp();
}

void Villager::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
}

void Villager::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new AvoidEntityGoal(this, typeid(Zombie), 8.0f, 0.6, 0.6), true);
    mGoalSelector.addGoal(1, new AvoidEntityGoal(this, typeid(EvocationIllager), 12.0f, 0.8, 0.8), true);
    mGoalSelector.addGoal(1, new AvoidEntityGoal(this, typeid(VindicationIllager), 8.0f, 0.8, 0.8), true);
    mGoalSelector.addGoal(1, new AvoidEntityGoal(this, typeid(Vex), 8.0f, 0.6, 0.6), true);
    mGoalSelector.addGoal(1, new TradeWithPlayerGoal(this), true);
    mGoalSelector.addGoal(1, new LookAtTradingPlayerGoal(this), true);
    mGoalSelector.addGoal(2, new PanicGoal(this, 0.6), true);
    mGoalSelector.addGoal(2, new MoveIndoorsGoal(this), true);
    mGoalSelector.addGoal(3, new RestrictOpenDoorGoal(this), true);
    mGoalSelector.addGoal(4, new OpenDoorGoal(this, true), true);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 0.6), true);
    mGoalSelector.addGoal(6, new MakeLoveGoal(this), true);
    mGoalSelector.addGoal(7, new TakeFlowerGoal(this), true);
    mGoalSelector.addGoal(8, new PlayGoal(this, 0.32), true);
    mGoalSelector.addGoal(9, new InteractGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(9, new VillagerInteractGoal(this), true);
    mGoalSelector.addGoal(9, new WaterAvoidingRandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);
}

void Villager::customServerAiStep()
{
    if (--mRandomTickDivider < 1)
    {
        BlockPos pos(shared_from_this());
        mLevel->getVillages()->queryUpdateAround(pos);
        mRandomTickDivider = mRandom->nextInt(50) + 70;
        boost::shared_ptr<Village> village = mLevel->getVillages()->getClosestVillage(pos, 32);
        mVillage = village;

        if (village == NULL)
        {
            clearRestriction();
        }
        else
        {
            restrictTo(*village->getCenter(), static_cast<int>(static_cast<float>(village->getRadius())));

            if (mRewardPlayersInVillage)
            {
                mRewardPlayersInVillage = false;
                village->rewardAllPlayers(5);
            }
        }
    }

    if (!isTrading() && mOffers == NULL && (getCareer() == 0 || mCareerLevel == 0))
    {
        increaseMerchantCareer();
    }

    if (!isTrading() && mTimeUntilReset > 0)
    {
        if (--mTimeUntilReset < 1)
        {
            if (mNeedsInitialization)
            {
                if (mOffers->size() != 0)
                {
                    for (MerchantRecipeList::iterator it = mOffers->begin(); it != mOffers->end(); ++it)
                    {
                        MerchantRecipe* recipe = *it;

                        if (recipe->isDeprecated())
                        {
                            recipe->increaseMaxUses(mRandom->nextInt(6) + mRandom->nextInt(6) + 2);
                        }
                    }
                }

                increaseMerchantCareer();
                mNeedsInitialization = false;

                boost::shared_ptr<Village> village = mVillage.lock();

                if (village != NULL && !mLastTradedPlayerName.empty())
                {
                    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::VILLAGER_HAPPY, 0);
                    village->modifyStanding(mLastTradedPlayerName, 1);
                }
            }

            addEffect(new MobEffectInstance(MobEffects::REGENERATION, 200, 0), boost::shared_ptr<Entity>());
        }
    }

    Mob::customServerAiStep();
}

bool Villager::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (item->getItem() == Items::nameTag)
    {
        item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand);
        return true;
    }

    if (holdingSpawnEggOfClass(item, GetType()) || !isAlive() || isTrading() || isBaby())
    {
        return AgableMob::mobInteract(player, hand);
    }

    if (!mLevel->mIsClientSide)
    {
        if (mOffers == NULL || !mOffers->empty())
        {
            setTradingPlayer(player);

            if (mOffers == NULL)
            {
                increaseMerchantCareer();
            }

            boost::shared_ptr<Merchant> merchant = boost::static_pointer_cast<Villager>(shared_from_this());

            if (hasCustomName())
            {
                player->openTrading(merchant, getCustomName(), getId());
            }
            else
            {
                player->openTrading(merchant, L"", getId());
            }
        }
        else
        {
            return AgableMob::mobInteract(player, hand);
        }
    }

    return true;
}

bool Villager::hurt(const DamageSource* source, float damage)
{
    if (source != NULL && dynamic_cast<const EntityDamageSource*>(source) != NULL)
    {
        boost::shared_ptr<Entity> attacker = source->getDirectEntity();

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType())
            && !boost::static_pointer_cast<Player>(attacker)->isAllowedToAttackAnimals())
        {
            return false;
        }

        if (attacker != NULL && eTYPE_DERIVED_FROM(EntityType::ARROW, attacker->GetType()))
        {
            boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(attacker);

            if (arrow->mOwner != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, arrow->mOwner->GetType())
                && !boost::static_pointer_cast<Player>(arrow->mOwner)->isAllowedToAttackAnimals())
            {
                return false;
            }
        }
    }

    return LivingEntity::hurt(source, damage);
}

unsigned int Villager::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    int itemId = Item::getId(holder.m_pItemInstance->getItem());

    switch (itemId)
    {
    case ItemId::NAME_TAG:
    {
        if (!holder.m_pItemInstance->hasCustomHoverName())
        {
            return IDS_NONE;
        }

        int differs = !hasCustomName() || holder.m_pItemInstance->getHoverName().compare(getCustomName()) != 0;

        return differs ? IDS_TOOLTIPS_NAME : IDS_NONE;
    }
    case ItemId::SPAWN_EGG:
        if (SpawnEggItem::fjGetType(holder.m_pItemInstance) == EntityType::VILLAGER)
        {
            return IDS_TOOLTIPS_PLACE;
        }
        break;
    }

    if (!isBaby() && getProfession() != 5)
    {
        return IDS_TOOLTIPS_TRADE;
    }

    return IDS_NONE;
}

bool Villager::HideUseTooltips()
{
    return getProfession() == 5;
}

void Villager::addAdditonalSaveData(CompoundTag* tag)
{
    AgableMob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Profession"), getProfession());
    tag->putInt(std::wstring(L"Riches"), mRiches);
    tag->putInt(std::wstring(L"Career"), getCareer());
    tag->putInt(std::wstring(L"CareerLevel"), mCareerLevel);
    tag->putBoolean(std::wstring(L"Willing"), mWillingToBreed);

    if (mOffers != NULL)
    {
        tag->put(std::wstring(L"Offers"), mOffers->createTag());
    }

    ListTag* inventory = new ListTag();

    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); slot++)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);

        if (!item->isEmpty())
        {
            inventory->add(item->save(new CompoundTag()));
        }
    }

    tag->put(std::wstring(L"Inventory"), inventory);
}

void Villager::readAdditionalSaveData(CompoundTag* tag)
{
    AgableMob::readAdditionalSaveData(tag);
    setProfession(tag->getInt(L"Profession"));
    mRiches = tag->getInt(L"Riches");
    setCareer(tag->getInt(L"Career"));
    mCareerLevel = tag->getInt(L"CareerLevel");
    mWillingToBreed = tag->getBoolean(L"Willing");

    if (tag->contains(L"Offers"))
    {
        CompoundTag* offers = tag->getCompound(L"Offers");
        delete mOffers;
        mOffers = new MerchantRecipeList(offers);
    }

    ListTag* inventory = tag->fjSafeGetListTag(L"Inventory", 10);

    for (int index = 0; index < inventory->size(); index++)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(inventory->getCompound(index)));

        if (!item->isEmpty())
        {
            mInventory->addItem(item);
        }
    }

    setCanPickUpLoot(true);
    loadPersonalAiGoals();
}

bool Villager::removeWhenFarAway()
{
    return false;
}

const SoundEvent* Villager::getAmbientSound()
{
    if (isTrading())
    {
        return SoundEvents::VILLAGER_TRADING;
    }

    return SoundEvents::VILLAGER_AMBIENT;
}

const SoundEvent* Villager::getHurtSound(const DamageSource*)
{
    return SoundEvents::VILLAGER_HURT;
}

const SoundEvent* Villager::getDeathSound()
{
    return SoundEvents::VILLAGER_DEATH;
}

bool Villager::isInLove()
{
    return mInLove;
}

void Villager::setInLove(bool inLove)
{
    mInLove = inLove;
}

void Villager::setChasing(bool chasing)
{
    mChasing = chasing;
}

bool Villager::isChasing()
{
    return mChasing;
}

void Villager::setLastHurtByMob(boost::shared_ptr<LivingEntity> attacker)
{
    LivingEntity::setLastHurtByMob(attacker);

    boost::shared_ptr<Village> village = mVillage.lock();

    if (village != NULL && attacker != NULL)
    {
        village->addAggressor(attacker);

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
        {
            int amount = -1;

            if (isBaby())
            {
                amount = -3;
            }

            village->modifyStanding(attacker->getName(), amount);

            if (isAlive())
            {
                mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::VILLAGER_ANGRY, 0);
            }
        }
    }
}

void Villager::die(const DamageSource* source)
{
    boost::shared_ptr<Village> village = mVillage.lock();

    if (village != NULL)
    {
        boost::shared_ptr<Entity> killer = source->getEntity();

        if (killer != NULL)
        {
            if (eTYPE_DERIVED_FROM(EntityType::PLAYER, killer->GetType()))
            {
                village->modifyStanding(killer->getName(), -2);
            }
            else if (eTYPE_DERIVED_FROM(EntityType::ENEMY, killer->GetType()))
            {
                village->resetNoBreedTimer();
            }
        }
        else
        {
            boost::shared_ptr<Player> player = mLevel->getNearestPlayer(shared_from_this(), 16.0, -1.0);

            if (player != NULL)
            {
                village->resetNoBreedTimer();
            }
        }
    }

    LivingEntity::die(source);
}

bool Villager::hasEnoughFoodToBreed()
{
    return countFood(1);
}

bool Villager::isWillingToBreed(bool consumeFood)
{
    if (!mWillingToBreed && consumeFood && hasEnoughFoodToBreed())
    {
        for (unsigned int slot = 0; slot < mInventory->getContainerSize(); slot++)
        {
            not_null_ptr<ItemInstance> item = mInventory->getItem(slot);

            if (!item->isEmpty())
            {
                if (item->getItem() == Items::bread && item->getCount() >= 3)
                {
                    mInventory->removeItem(slot, 3);
                }
                else if ((item->getItem() == Items::potato || item->getItem() == Items::carrot)
                         && item->getCount() >= 12)
                {
                    mInventory->removeItem(slot, 12);
                }
                else
                {
                    continue;
                }

                mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::IN_LOVE_HEARTS, 0);
                mWillingToBreed = true;
                break;
            }
        }
    }

    return mWillingToBreed;
}

void Villager::setWillingToBreed(bool willing)
{
    mWillingToBreed = willing;
}

ResourceLocation* Villager::getDefaultLootTable()
{
    return &BuiltInLootTables::VILLAGER;
}

float Villager::getEyeHeight()
{
    if (isBaby())
    {
        return 0.81f;
    }

    return 1.62f;
}

void Villager::addParticlesAroundSelf(const ParticleType* particle)
{
    for (int i = 0; i < 5; i++)
    {
        double xd = mRandom->nextGaussian() * 0.02;
        double yd = mRandom->nextGaussian() * 0.02;
        double zd = mRandom->nextGaussian() * 0.02;
        arrayWithLength<int> params;
        float xo = mRandom->nextFloat();
        float yo = mRandom->nextFloat();
        float zo = mRandom->nextFloat();
        mLevel->addParticle(particle, mX + (double)(xo * mBbWidth * 2.0f) - (double)mBbWidth,
                            mY + 1.0 + (double)(yo * mBbHeight), mZ + (double)(zo * mBbWidth * 2.0f) - (double)mBbWidth,
                            xd, yd, zd, params);
    }
}

void Villager::handleEntityEvent(unsigned char event, int data)
{
    switch (event)
    {
    case EntityEvent::LOVE_HEARTS:
        addParticlesAroundSelf(PARTICLE_HEART);
        break;
    case EntityEvent::VILLAGER_ANGRY:
        addParticlesAroundSelf(PARTICLE_ANGRY_VILLAGER);
        break;
    case EntityEvent::VILLAGER_HAPPY:
        addParticlesAroundSelf(PARTICLE_HAPPY_VILLAGER);
        break;
    default:
        Mob::handleEntityEvent(event, data);
        break;
    }
}

boost::shared_ptr<AgableMob> Villager::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    boost::shared_ptr<Villager> child(new Villager(mLevel));
    child->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(child)), NULL, L"");
    return child;
}

bool Villager::canBeLeashed(const boost::shared_ptr<Player>&)
{
    return false;
}

void Villager::thunderHit(const LightningBolt*)
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    boost::shared_ptr<Witch> witch(new Witch(mLevel));
    witch->moveTo(mX, mY, mZ, mYRot, mXRot);
    witch->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(witch)), NULL, L"");
    witch->setNoAi(isNoAi());

    if (hasCustomName())
    {
        witch->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
        witch->setCustomNameVisible(isCustomNameVisible());
    }

    mLevel->addEntity(witch);
    remove();
}

bool Villager::wantsItem(Item* item)
{
    return item == Items::bread || item == Items::potato || item == Items::carrot || item == Items::wheat
           || item == Items::wheatSeeds || item == Items::beetroot || item == Items::beetrootSeeds;
}

void Villager::pickUpItem(boost::shared_ptr<ItemEntity> itemEntity)
{
    not_null_ptr<ItemInstance> item = itemEntity->getItem();

    if (wantsItem(item->getItem()))
    {
        not_null_ptr<ItemInstance> remainder = mInventory->addItem(item);

        if (remainder->isEmpty())
        {
            itemEntity->remove();
        }
        else
        {
            item->setCount(remainder->getCount());
        }
    }
}

bool Villager::hasFarmSeeds()
{
    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); slot++)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);

        if (!item->isEmpty()
            && (item->getItem() == Items::wheatSeeds || item->getItem() == Items::potato
                || item->getItem() == Items::carrot || item->getItem() == Items::beetrootSeeds))
        {
            return true;
        }
    }

    return false;
}

bool Villager::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    if (Mob::setSlot(slot, item))
    {
        return true;
    }

    int index = slot - 300;

    if (index >= 0 && index < mInventory->getContainerSize())
    {
        mInventory->setItem(index, item);
        return true;
    }

    return false;
}
