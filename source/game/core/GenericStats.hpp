#ifndef GAME_CORE_GENERICSTATS_HPP
#define GAME_CORE_GENERICSTATS_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Block.hpp"

class BlockState;
class DamageSource;
class Item;
class ItemInstance;
class Mob;
class Player;
class Stat;

enum eAward
{
    eAward_None = -1,
    eAward_OpenInventory = 0,
    eAward_MineWood = 1,
    eAward_BuildWorkbench = 2,
    eAward_BuildPickaxe = 3,
    eAward_BuildFurnace = 4,
    eAward_AcquireIron = 5,
    eAward_BuildHoe = 6,
    eAward_MakeBread = 7,
    eAward_BakeCake = 8,
    eAward_BuildBetterPickaxe = 9,
    eAward_CookFish = 10,
    eAward_OnARail = 11,
    eAward_BuildSword = 12,
    eAward_KillEnemy = 13,
    eAward_KillCow = 14,
    eAward_FlyPig = 15,
    eAward_LeaderOfThePack = 16,
    eAward_MOARTools = 17,
    eAward_DispenseWithThis = 18,
    eAward_InToTheNether = 19,
    eAward_Mine100Blocks = 20,
    eAward_Kill10Creepers = 21,
    eAward_PorkChop = 22,
    eAward_PassingTheTime = 23,
    eAward_Archer = 24,
    eAward_SocialPost = 25,
    eAward_SnipeSkeleton = 26,
    eAward_Diamonds = 27,
    eAward_Ghast = 29,
    eAward_BlazeRod = 30,
    eAward_Potion = 31,
    eAward_TheEnd = 32,
    eAward_WinGame = 33,
    eAward_Enchantments = 34,
    eAward_Overkill = 35,
    eAward_Bookcase = 36,
    eAward_AdventuringTime = 37,
    eAward_Repopulation = 38,
    eAward_DiamondsToYou = 39,
    eAward_TheHaggler = 40,
    eAward_PotPlanter = 41,
    eAward_ItsASign = 42,
    eAward_IronBelly = 43,
    eAward_HaveAShearfulDay = 44,
    eAward_RainbowCollection = 45,
    eAward_StayinFrosty = 46,
    eAward_ChestfulOfCobblestone = 47,
    eAward_RenewableEnergy = 48,
    eAward_MusicToMyEars = 49,
    eAward_BodyGuard = 50,
    eAward_IronMan = 51,
    eAward_ZombieDoctor = 52,
    eAward_LionTamer = 53,
    eAward_SpawnWither = 54,
    eAward_KillWither = 55,
    eAward_FullBeacon = 56,
    eAward_Overpowered = 57,
    eAward_TieDyeOutfit = 58,
    eAward_Trampoline = 59,
    eAward_TheStudent = 60,
    eAward_HasBecomeTheMaster = 61,
    eAward_TisButAScratch = 62,
    eAward_Cupid = 63,
    eAward_YouWontLikeMeWhenImHungry = 64,
    eAward_Mine = 65,
    eAward_TheEndAgain = 66,
    eAward_YouNeedAMint = 67,
    eAward_SuperSonic = 68,
    eAward_DrySpell = 69,
    eAward_FreeDiver = 70,
    eAward_SuperFuel = 71,
    eAward_SaddleUp = 72,
    eAward_TasteOfYourOwnMedicine = 73,
    eAward_BeamMeUp = 74,
    eAward_MapRoom = 75,
    eAward_Camouflage = 76,
    eAward_BackFromTheDead = 77,
    eAward_EquipYourFists = 78,
    eAward_TenaciousBall = 79,
    eAward_Deadshot = 80,
    eAward_SuckMySnowballs = 81,
    eAward_MountainTop = 82,
    eAward_WillLikeNoOther = 83,
    eAward_TheDeepEnd = 84,
    eAward_GreatViewFromUpHere = 85,
    eAward_ChangeOfSheets = 86,
    eAward_CheatingDeath = 87,
    eAward_SoIGotThatGoingForMe = 88,
    eAward_LetItGo = 89,
    eAward_FeelingIll = 90,
    eAward_1234Pickle = 91,
    eAward_AlternativeFuel = 92,
    eAward_Moskstraumen = 93,
    eAward_Castaway = 94,
    eAward_SleepWithTheFishes = 95,
    eAward_Echolocation = 96
};

class GenericStats;
extern GenericStats* gStats;

class GenericStats
{
public:
    virtual ~GenericStats() {}
    virtual Stat* get_stat(int id) = 0;

    static Stat* openInventory();
    static Stat* mineBlock() { return gStats->get_totalBlocksMined(); }
    static Stat* tamedEntity(unsigned long long entityType) { return gStats->get_tamedEntity(entityType); }
    static Stat* shearedEntity(unsigned long long entityType) { return gStats->get_shearedEntity(entityType); }
    static Stat* itemsUsed(Item* item) { return gStats->get_itemsUsed(Item::VALIDgetId(item)); }
    static Stat* fallOneM() { return gStats->get_fallOneM(); }
    static Stat* minecartOneM() { return gStats->get_minecartOneM(); }
    static Stat* boatOneM() { return gStats->get_boatOneM(); }
    static Stat* pigOneM() { return gStats->get_pigOneM(); }
    static Stat* cowsMilked() { return gStats->get_cowsMilked(); }
    static Stat* netherLavaCollected() { return gStats->get_netherLavaCollected(); }
    static Stat* breedEntity(unsigned long long entityType) { return gStats->get_breedEntity(entityType); }
    static Stat* craftedEntity(unsigned long long entityType) { return gStats->get_craftedEntity(entityType); }
    static Stat* timePlayed() { return gStats->get_timePlayed(); }
    static Stat* blocksPlaced(Block* block) { return gStats->get_blocksPlaced(Block::getId(block)); }
    static Stat* blocksMined(Block* block) { return gStats->get_blocksMined(Block::getId(block)); }
    static Stat* itemsCrafted(Item* item) { return gStats->get_itemsCrafted(Item::VALIDgetId(item)); }
    static Stat* itemsBought(Item* item) { return gStats->get_itemsBought(Item::VALIDgetId(item)); }
    static Stat* armourDyed(Item* item) { return gStats->get_armourDyed(Item::VALIDgetId(item)); }
    static Stat* completeTheEnd() { return gStats->get_completeTheEnd(); }
    static Stat* enteredBiome(int biomeId) { return gStats->get_enteredBiome(biomeId); }
    static Stat* mineWood();
    static Stat* buildWorkbench();
    static Stat* buildPickaxe();
    static Stat* buildFurnace();
    static Stat* acquireIron();
    static Stat* buildHoe();
    static Stat* makeBread();
    static Stat* bakeCake();
    static Stat* buildBetterPickaxe();
    static Stat* cookFish();
    static Stat* onARail();
    static Stat* buildSword();
    static Stat* killEnemy();
    static Stat* killCow();
    static Stat* flyPig();
    static Stat* snipeSkeleton();
    static Stat* diamonds();
    static Stat* ghast();
    static Stat* blazeRod();
    static Stat* potion();
    static Stat* theEnd();
    static Stat* winGame();
    static Stat* enchantments();
    static Stat* overkill();
    static Stat* bookcase();
    static Stat* leaderOfThePack();
    static Stat* MOARTools();
    static Stat* dispenseWithThis();
    static Stat* InToTheNether();
    static Stat* socialPost();
    static Stat* mine100Blocks();
    static Stat* kill10Creepers();
    static Stat* adventuringTime();
    static Stat* repopulation();
    static Stat* diamondsToYou();
    static Stat* porkChop();
    static Stat* passingTheTime();
    static Stat* archer();
    static Stat* theHaggler();
    static Stat* potPlanter();
    static Stat* itsASign();
    static Stat* ironBelly();
    static Stat* haveAShearfulDay();
    static Stat* rainbowCollection();
    static Stat* stayinFrosty();
    static Stat* chestfulOfCobblestone();
    static Stat* renewableEnergy();
    static Stat* musicToMyEars();
    static Stat* bodyGuard();
    static Stat* ironMan();
    static Stat* zombieDoctor();
    static Stat* lionTamer();
    static Stat* spawnWither();
    static Stat* killWither();
    static Stat* fullBeacon();
    static Stat* overpowered();
    static Stat* tieDyeOutfit();
    static Stat* trampoline();
    static Stat* theStudent();
    static Stat* hasBecomeTheMaster();
    static Stat* tisButAScratch();
    static Stat* cupid();
    static Stat* youWontLikeMeWhenImHungry();
    static Stat* mine();
    static Stat* theEndAgain();
    static Stat* youNeedAMint();
    static Stat* superSonic();
    static Stat* drySpell();
    static Stat* freeDiver();
    static Stat* superFuel();
    static Stat* saddleUp();
    static Stat* tasteOfYourOwnMedicine();
    static Stat* beamMeUp();
    static Stat* mapRoom();
    static Stat* camouflage();
    static Stat* backFromTheDead();
    static Stat* equipYourFists();
    static Stat* tenaciousBall();
    static Stat* deadshot();
    static Stat* suckMySnowballs();
    static Stat* mountainTop();
    static Stat* willLikeNoOther();
    static Stat* theDeepEnd();
    static Stat* greatViewFromUpHere();
    static Stat* changeOfSheets();
    static Stat* cheatingDeath();
    static Stat* soIGotThatGoingForMe();
    static Stat* letItGo();
    static Stat* feelingIll();
    static Stat* _1234Pickle();
    static Stat* alternativeFuel();
    static Stat* moskstraumen();
    static Stat* castaway();
    static Stat* sleepWithTheFishes();
    static Stat* echolocation();
    static arrayWithLength<unsigned char> param_MG03Score(int value);
    static arrayWithLength<unsigned char> param_MG03BestTime(unsigned int value);
    static arrayWithLength<unsigned char> param_MG03RingCount(int value);
    static arrayWithLength<unsigned char> param_walk(int value);
    static arrayWithLength<unsigned char> param_swim(int value);
    static arrayWithLength<unsigned char> param_fall(int value);
    static arrayWithLength<unsigned char> param_climb(int value);
    static arrayWithLength<unsigned char> param_minecart(int value);
    static arrayWithLength<unsigned char> param_boat(int value);
    static arrayWithLength<unsigned char> param_pig(int value);
    static arrayWithLength<unsigned char> param_cowsMilked();
    static arrayWithLength<unsigned char> param_blocksPlaced(const BlockState* state, int count);
    static arrayWithLength<unsigned char> param_blocksMined(const BlockState* state, int count);
    static arrayWithLength<unsigned char> param_itemsCollected(Item* item, int auxValue, int count);
    static arrayWithLength<unsigned char> param_itemsCrafted(Item* item, int auxValue, int count);
    static arrayWithLength<unsigned char> param_itemsSmelted(Item* item, int auxValue, int count);
    static arrayWithLength<unsigned char> param_itemsUsed(boost::shared_ptr<Player> player,
                                                          not_null_ptr<ItemInstance> itemInstance);
    static arrayWithLength<unsigned char> param_itemsBought(Item* item, int auxValue, int count);
    static arrayWithLength<unsigned char> param_mobKill(boost::shared_ptr<Player> player, boost::shared_ptr<Mob> mob,
                                                        const DamageSource* damageSource);
    static arrayWithLength<unsigned char> param_breedEntity(unsigned long long entityType);
    static arrayWithLength<unsigned char> param_tamedEntity(unsigned long long entityType);
    static arrayWithLength<unsigned char> param_curedEntity(unsigned long long entityType);
    static arrayWithLength<unsigned char> param_craftedEntity(unsigned long long entityType);
    static arrayWithLength<unsigned char> param_shearedEntity(unsigned long long entityType);
    static arrayWithLength<unsigned char> param_time(int value);
    static arrayWithLength<unsigned char> param_changedDimension(int fromDimension, int toDimension);
    static arrayWithLength<unsigned char> param_enteredBiome(int value);
    static arrayWithLength<unsigned char> param_noArgs();
    static arrayWithLength<unsigned char> param_openInventory();
    static arrayWithLength<unsigned char> param_mineWood();
    static arrayWithLength<unsigned char> param_buildWorkbench();
    static arrayWithLength<unsigned char> param_buildPickaxe();
    static arrayWithLength<unsigned char> param_buildFurnace();
    static arrayWithLength<unsigned char> param_acquireIron();
    static arrayWithLength<unsigned char> param_buildHoe();
    static arrayWithLength<unsigned char> param_makeBread();
    static arrayWithLength<unsigned char> param_bakeCake();
    static arrayWithLength<unsigned char> param_buildBetterPickaxe();
    static arrayWithLength<unsigned char> param_cookFish();
    static arrayWithLength<unsigned char> param_onARail(int value);
    static arrayWithLength<unsigned char> param_buildSword();
    static arrayWithLength<unsigned char> param_killEnemy();
    static arrayWithLength<unsigned char> param_killCow();
    static arrayWithLength<unsigned char> param_flyPig();
    static arrayWithLength<unsigned char> param_snipeSkeleton();
    static arrayWithLength<unsigned char> param_diamonds();
    static arrayWithLength<unsigned char> param_ghast();
    static arrayWithLength<unsigned char> param_blazeRod();
    static arrayWithLength<unsigned char> param_potion();
    static arrayWithLength<unsigned char> param_theEnd();
    static arrayWithLength<unsigned char> param_winGame();
    static arrayWithLength<unsigned char> param_enchantments();
    static arrayWithLength<unsigned char> param_overkill(int value);
    static arrayWithLength<unsigned char> param_bookcase();
    static arrayWithLength<unsigned char> param_leaderOfThePack();
    static arrayWithLength<unsigned char> param_MOARTools();
    static arrayWithLength<unsigned char> param_dispenseWithThis();
    static arrayWithLength<unsigned char> param_InToTheNether();
    static arrayWithLength<unsigned char> param_socialPost();
    static arrayWithLength<unsigned char> param_mine100Blocks();
    static arrayWithLength<unsigned char> param_kill10Creepers();
    static arrayWithLength<unsigned char> param_adventuringTime();
    static arrayWithLength<unsigned char> param_repopulation();
    static arrayWithLength<unsigned char> param_porkChop();
    static arrayWithLength<unsigned char> param_diamondsToYou();
    static arrayWithLength<unsigned char> param_passingTheTime();
    static arrayWithLength<unsigned char> param_archer();
    static arrayWithLength<unsigned char> param_theHaggler();
    static arrayWithLength<unsigned char> param_potPlanter();
    static arrayWithLength<unsigned char> param_itsASign();
    static arrayWithLength<unsigned char> param_ironBelly();
    static arrayWithLength<unsigned char> param_haveAShearfulDay();
    static arrayWithLength<unsigned char> param_rainbowCollection();
    static arrayWithLength<unsigned char> param_stayinFrosty();
    static arrayWithLength<unsigned char> param_chestfulOfCobblestone(int value);
    static arrayWithLength<unsigned char> param_renewableEnergy();
    static arrayWithLength<unsigned char> param_musicToMyEars(int value);
    static arrayWithLength<unsigned char> param_bodyGuard();
    static arrayWithLength<unsigned char> param_ironMan();
    static arrayWithLength<unsigned char> param_zombieDoctor();
    static arrayWithLength<unsigned char> param_lionTamer();
    static arrayWithLength<unsigned char> param_tieDyeOutfit(int value);
    static arrayWithLength<unsigned char> param_trampoline(int value);
    static arrayWithLength<unsigned char> param_spawnWither();
    static arrayWithLength<unsigned char> param_killWither();
    static arrayWithLength<unsigned char> param_fullBeacon();
    static arrayWithLength<unsigned char> param_overpowered();
    static arrayWithLength<unsigned char> param_smeltEverything();
    static arrayWithLength<unsigned char> param_freightStation();
    static arrayWithLength<unsigned char> param_theStudent();
    static arrayWithLength<unsigned char> param_hasBecomeTheMaster();
    static arrayWithLength<unsigned char> param_tisButAScratch();
    static arrayWithLength<unsigned char> param_cupid();
    static arrayWithLength<unsigned char> param_youWontLikeMeWhenImHungry();
    static arrayWithLength<unsigned char> param_mine();
    static arrayWithLength<unsigned char> param_theEndAgain();
    static arrayWithLength<unsigned char> param_youNeedAMint();
    static arrayWithLength<unsigned char> param_superSonic(int value);
    static arrayWithLength<unsigned char> param_drySpell();
    static arrayWithLength<unsigned char> param_freeDiver(int value);
    static arrayWithLength<unsigned char> param_superFuel();
    static arrayWithLength<unsigned char> param_saddleUp();
    static arrayWithLength<unsigned char> param_tasteOfYourOwnMedicine();
    static arrayWithLength<unsigned char> param_beamMeUp(int value);
    static arrayWithLength<unsigned char> param_mapRoom(int value);
    static arrayWithLength<unsigned char> param_camouflage();
    static arrayWithLength<unsigned char> param_backFromTheDead();
    static arrayWithLength<unsigned char> param_equipYourFists();
    static arrayWithLength<unsigned char> param_tenaciousBall(int value);
    static arrayWithLength<unsigned char> param_deadshot();
    static arrayWithLength<unsigned char> param_suckMySnowballs(int value);
    static arrayWithLength<unsigned char> param_mountainTop();
    static arrayWithLength<unsigned char> param_willLikeNoOther();
    static arrayWithLength<unsigned char> param_theDeepEnd();
    static arrayWithLength<unsigned char> param_greatViewFromUpHere(int value);
    static arrayWithLength<unsigned char> param_soIGotThatGoingForMe(int value);
    static arrayWithLength<unsigned char> param_letItGo();
    static arrayWithLength<unsigned char> param_feelingIll();
    static arrayWithLength<unsigned char> param_changeOfSheets();
    static arrayWithLength<unsigned char> param_1234Pickle();
    static arrayWithLength<unsigned char> param_alternativeFuel();
    static arrayWithLength<unsigned char> param_moskstraumen();
    static arrayWithLength<unsigned char> param_castaway();
    static arrayWithLength<unsigned char> param_sleepWithTheFishes();
    static arrayWithLength<unsigned char> param_echolocation();
    virtual Stat* get_achievement(eAward award);
    virtual Stat* get_walkOneM();
    virtual Stat* get_swimOneM();
    virtual Stat* get_fallOneM();
    virtual Stat* get_climbOneM();
    virtual Stat* get_minecartOneM();
    virtual Stat* get_boatOneM();
    virtual Stat* get_pigOneM();
    virtual Stat* get_portalsCreated();
    virtual Stat* get_cowsMilked();
    virtual Stat* get_netherLavaCollected();
    virtual Stat* get_killMob();
    virtual Stat* get_killsZombie();
    virtual Stat* get_killsSkeleton();
    virtual Stat* get_killsCreeper();
    virtual Stat* get_killsSpider();
    virtual Stat* get_killsSpiderJockey();
    virtual Stat* get_killsZombiePigman();
    virtual Stat* get_killsSlime();
    virtual Stat* get_killsGhast();
    virtual Stat* get_killsNetherZombiePigman();
    virtual Stat* get_breedEntity(unsigned long long entityType);
    virtual Stat* get_tamedEntity(unsigned long long entityType);
    virtual Stat* get_curedEntity(unsigned long long entityType);
    virtual Stat* get_craftedEntity(unsigned long long entityType);
    virtual Stat* get_shearedEntity(unsigned long long entityType);
    virtual Stat* get_totalBlocksMined();
    virtual Stat* get_timePlayed();
    virtual Stat* get_blocksPlaced(int blockId);
    virtual Stat* get_blocksMined(int blockId);
    virtual Stat* get_itemsCollected(int itemId, int auxValue);
    virtual Stat* get_itemsCrafted(int itemId);
    virtual Stat* get_itemsSmelted(int itemId);
    virtual Stat* get_itemsUsed(int itemId);
    virtual Stat* get_itemsBought(int itemId);
    virtual Stat* get_armourDyed(int itemId);
    virtual Stat* get_killsEnderdragon();
    virtual Stat* get_completeTheEnd();
    virtual Stat* get_changedDimension(int fromDimension, int toDimension);
    virtual Stat* get_enteredBiome(int biomeId);
    virtual Stat* get_MG03_Score();
    virtual Stat* get_MG03_RingCount();
    virtual Stat* get_MG03_BestTimeScoreChallenge();
    virtual Stat* get_MG03_BestTime();
    virtual arrayWithLength<unsigned char> getParam_walkOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_swimOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_fallOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_climbOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_minecartOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_boatOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_pigOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_cowsMilked();
    virtual arrayWithLength<unsigned char> getParam_blocksPlaced(int blockId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_blocksMined(int blockId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsCollected(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsCrafted(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsSmelted(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsUsed(boost::shared_ptr<Player> player,
                                                              not_null_ptr<ItemInstance> itemInstance);
    virtual arrayWithLength<unsigned char> getParam_itemsBought(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_mobKill(boost::shared_ptr<Player> player,
                                                            boost::shared_ptr<Mob> mob,
                                                            const DamageSource* damageSource);
    virtual arrayWithLength<unsigned char> getParam_breedEntity(unsigned long long entityType);
    virtual arrayWithLength<unsigned char> getParam_tamedEntity(unsigned long long entityType);
    virtual arrayWithLength<unsigned char> getParam_curedEntity(unsigned long long entityType);
    virtual arrayWithLength<unsigned char> getParam_craftedEntity(unsigned long long entityType);
    virtual arrayWithLength<unsigned char> getParam_shearedEntity(unsigned long long entityType);
    virtual arrayWithLength<unsigned char> getParam_time(int value);
    virtual arrayWithLength<unsigned char> getParam_changedDimension(int fromDimension, int toDimension);
    virtual arrayWithLength<unsigned char> getParam_enteredBiome(int biomeId);
    virtual arrayWithLength<unsigned char> getParam_achievement(eAward award);
    virtual arrayWithLength<unsigned char> getParam_onARail(int value);
    virtual arrayWithLength<unsigned char> getParam_overkill(int value);
    virtual arrayWithLength<unsigned char> getParam_openInventory(int value);
    virtual arrayWithLength<unsigned char> getParam_chestfulOfCobblestone(int value);
    virtual arrayWithLength<unsigned char> getParam_musicToMyEars(int value);
    virtual arrayWithLength<unsigned char> getParam_MG03Score(int value);
    virtual arrayWithLength<unsigned char> getParam_MG03RingCount(int value);
    virtual arrayWithLength<unsigned char> getParam_MG03BestTime(unsigned int value);
    virtual arrayWithLength<unsigned char> getParam_tieDyeOutfit(int value);
    virtual arrayWithLength<unsigned char> getParam_trampoline(int value);
    virtual arrayWithLength<unsigned char> getParam_spawnWither();
    virtual arrayWithLength<unsigned char> getParam_killWither();
    virtual arrayWithLength<unsigned char> getParam_fullBeacon();
    virtual arrayWithLength<unsigned char> getParam_overpowered();
    virtual arrayWithLength<unsigned char> getParam_smeltEverything();
    virtual arrayWithLength<unsigned char> getParam_camouflage();
    virtual arrayWithLength<unsigned char> getParam_freightStation();
    virtual arrayWithLength<unsigned char> getParam_theStudent();
    virtual arrayWithLength<unsigned char> getParam_hasBecomeTheMaster();
    virtual arrayWithLength<unsigned char> getParam_tisButAScratch();
    virtual arrayWithLength<unsigned char> getParam_cupid();
    virtual arrayWithLength<unsigned char> getParam_youWontLikeMeWhenImHungry();
    virtual arrayWithLength<unsigned char> getParam_mine();
    virtual arrayWithLength<unsigned char> getParam_superSonic(int value);
    virtual arrayWithLength<unsigned char> getParam_freeDiver(int value);
    virtual arrayWithLength<unsigned char> getParam_beamMeUp(int value);
    virtual arrayWithLength<unsigned char> getParam_mapRoom(int value);
    virtual arrayWithLength<unsigned char> getParam_tenaciousBall(int value);
    virtual arrayWithLength<unsigned char> getParam_suckMySnowballs(int value);
    virtual arrayWithLength<unsigned char> getParam_greatViewFromUpHere(int value);
    virtual arrayWithLength<unsigned char> getParam_soIGotThatGoingForMe(int value);
    virtual arrayWithLength<unsigned char> getParam_noArgs();
    static Stat* itemsCollected(not_null_ptr<ItemInstance> itemInstance);
    static arrayWithLength<unsigned char> param_itemsCollected(not_null_ptr<ItemInstance> itemInstance);
    static arrayWithLength<unsigned char> param_itemsBought(not_null_ptr<ItemInstance> itemInstance);
    static Stat* breedCow();
    static Stat* exploreAllBiomes();
    static Stat* smeltEverything();
    static Stat* freightStation();
    static arrayWithLength<unsigned char> param_breedCow();
    static arrayWithLength<unsigned char> param_exploreAllBiomes();
};

#endif
