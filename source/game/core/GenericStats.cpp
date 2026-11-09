#include "game/core/GenericStats.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

Stat* GenericStats::openInventory()
{
    return gStats->get_achievement(eAward_OpenInventory);
}

Stat* GenericStats::mineWood()
{
    return gStats->get_achievement(eAward_MineWood);
}

Stat* GenericStats::buildWorkbench()
{
    return gStats->get_achievement(eAward_BuildWorkbench);
}

Stat* GenericStats::buildPickaxe()
{
    return gStats->get_achievement(eAward_BuildPickaxe);
}

Stat* GenericStats::buildFurnace()
{
    return gStats->get_achievement(eAward_BuildFurnace);
}

Stat* GenericStats::acquireIron()
{
    return gStats->get_achievement(eAward_AcquireIron);
}

Stat* GenericStats::buildHoe()
{
    return gStats->get_achievement(eAward_BuildHoe);
}

Stat* GenericStats::makeBread()
{
    return gStats->get_achievement(eAward_MakeBread);
}

Stat* GenericStats::bakeCake()
{
    return gStats->get_achievement(eAward_BakeCake);
}

Stat* GenericStats::buildBetterPickaxe()
{
    return gStats->get_achievement(eAward_BuildBetterPickaxe);
}

Stat* GenericStats::cookFish()
{
    return gStats->get_achievement(eAward_CookFish);
}

Stat* GenericStats::onARail()
{
    return gStats->get_achievement(eAward_OnARail);
}

Stat* GenericStats::buildSword()
{
    return gStats->get_achievement(eAward_BuildSword);
}

Stat* GenericStats::killEnemy()
{
    return gStats->get_achievement(eAward_KillEnemy);
}

Stat* GenericStats::killCow()
{
    return gStats->get_achievement(eAward_KillCow);
}

Stat* GenericStats::flyPig()
{
    return gStats->get_achievement(eAward_FlyPig);
}

Stat* GenericStats::snipeSkeleton()
{
    return gStats->get_achievement(eAward_SnipeSkeleton);
}

Stat* GenericStats::diamonds()
{
    return gStats->get_achievement(eAward_Diamonds);
}

Stat* GenericStats::ghast()
{
    return gStats->get_achievement(eAward_Ghast);
}

Stat* GenericStats::blazeRod()
{
    return gStats->get_achievement(eAward_BlazeRod);
}

Stat* GenericStats::potion()
{
    return gStats->get_achievement(eAward_Potion);
}

Stat* GenericStats::theEnd()
{
    return gStats->get_achievement(eAward_TheEnd);
}

Stat* GenericStats::winGame()
{
    return gStats->get_achievement(eAward_WinGame);
}

Stat* GenericStats::enchantments()
{
    return gStats->get_achievement(eAward_Enchantments);
}

Stat* GenericStats::overkill()
{
    return gStats->get_achievement(eAward_Overkill);
}

Stat* GenericStats::bookcase()
{
    return gStats->get_achievement(eAward_Bookcase);
}

Stat* GenericStats::leaderOfThePack()
{
    return gStats->get_achievement(eAward_LeaderOfThePack);
}

Stat* GenericStats::MOARTools()
{
    return gStats->get_achievement(eAward_MOARTools);
}

Stat* GenericStats::dispenseWithThis()
{
    return gStats->get_achievement(eAward_DispenseWithThis);
}

Stat* GenericStats::InToTheNether()
{
    return gStats->get_achievement(eAward_InToTheNether);
}

Stat* GenericStats::socialPost()
{
    return gStats->get_achievement(eAward_SocialPost);
}

Stat* GenericStats::mine100Blocks()
{
    return gStats->get_achievement(eAward_Mine100Blocks);
}

Stat* GenericStats::kill10Creepers()
{
    return gStats->get_achievement(eAward_Kill10Creepers);
}

Stat* GenericStats::adventuringTime()
{
    return gStats->get_achievement(eAward_AdventuringTime);
}

Stat* GenericStats::repopulation()
{
    return gStats->get_achievement(eAward_Repopulation);
}

Stat* GenericStats::diamondsToYou()
{
    return gStats->get_achievement(eAward_DiamondsToYou);
}

Stat* GenericStats::porkChop()
{
    return gStats->get_achievement(eAward_PorkChop);
}

Stat* GenericStats::passingTheTime()
{
    return gStats->get_achievement(eAward_PassingTheTime);
}

Stat* GenericStats::archer()
{
    return gStats->get_achievement(eAward_Archer);
}

Stat* GenericStats::theHaggler()
{
    return gStats->get_achievement(eAward_TheHaggler);
}

Stat* GenericStats::potPlanter()
{
    return gStats->get_achievement(eAward_PotPlanter);
}

Stat* GenericStats::itsASign()
{
    return gStats->get_achievement(eAward_ItsASign);
}

Stat* GenericStats::ironBelly()
{
    return gStats->get_achievement(eAward_IronBelly);
}

Stat* GenericStats::haveAShearfulDay()
{
    return gStats->get_achievement(eAward_HaveAShearfulDay);
}

Stat* GenericStats::rainbowCollection()
{
    return gStats->get_achievement(eAward_RainbowCollection);
}

Stat* GenericStats::stayinFrosty()
{
    return gStats->get_achievement(eAward_StayinFrosty);
}

Stat* GenericStats::chestfulOfCobblestone()
{
    return gStats->get_achievement(eAward_ChestfulOfCobblestone);
}

Stat* GenericStats::renewableEnergy()
{
    return gStats->get_achievement(eAward_RenewableEnergy);
}

Stat* GenericStats::musicToMyEars()
{
    return gStats->get_achievement(eAward_MusicToMyEars);
}

Stat* GenericStats::bodyGuard()
{
    return gStats->get_achievement(eAward_BodyGuard);
}

Stat* GenericStats::ironMan()
{
    return gStats->get_achievement(eAward_IronMan);
}

Stat* GenericStats::zombieDoctor()
{
    return gStats->get_achievement(eAward_ZombieDoctor);
}

Stat* GenericStats::lionTamer()
{
    return gStats->get_achievement(eAward_LionTamer);
}

Stat* GenericStats::spawnWither()
{
    return gStats->get_achievement(eAward_SpawnWither);
}

Stat* GenericStats::killWither()
{
    return gStats->get_achievement(eAward_KillWither);
}

Stat* GenericStats::fullBeacon()
{
    return gStats->get_achievement(eAward_FullBeacon);
}

Stat* GenericStats::overpowered()
{
    return gStats->get_achievement(eAward_Overpowered);
}

Stat* GenericStats::tieDyeOutfit()
{
    return gStats->get_achievement(eAward_TieDyeOutfit);
}

Stat* GenericStats::trampoline()
{
    return gStats->get_achievement(eAward_Trampoline);
}

Stat* GenericStats::theStudent()
{
    return gStats->get_achievement(eAward_TheStudent);
}

Stat* GenericStats::hasBecomeTheMaster()
{
    return gStats->get_achievement(eAward_HasBecomeTheMaster);
}

Stat* GenericStats::tisButAScratch()
{
    return gStats->get_achievement(eAward_TisButAScratch);
}

Stat* GenericStats::cupid()
{
    return gStats->get_achievement(eAward_Cupid);
}

Stat* GenericStats::youWontLikeMeWhenImHungry()
{
    return gStats->get_achievement(eAward_YouWontLikeMeWhenImHungry);
}

Stat* GenericStats::mine()
{
    return gStats->get_achievement(eAward_Mine);
}

Stat* GenericStats::theEndAgain()
{
    return gStats->get_achievement(eAward_TheEndAgain);
}

Stat* GenericStats::youNeedAMint()
{
    return gStats->get_achievement(eAward_YouNeedAMint);
}

Stat* GenericStats::superSonic()
{
    return gStats->get_achievement(eAward_SuperSonic);
}

Stat* GenericStats::drySpell()
{
    return gStats->get_achievement(eAward_DrySpell);
}

Stat* GenericStats::freeDiver()
{
    return gStats->get_achievement(eAward_FreeDiver);
}

Stat* GenericStats::superFuel()
{
    return gStats->get_achievement(eAward_SuperFuel);
}

Stat* GenericStats::saddleUp()
{
    return gStats->get_achievement(eAward_SaddleUp);
}

Stat* GenericStats::tasteOfYourOwnMedicine()
{
    return gStats->get_achievement(eAward_TasteOfYourOwnMedicine);
}

Stat* GenericStats::beamMeUp()
{
    return gStats->get_achievement(eAward_BeamMeUp);
}

Stat* GenericStats::mapRoom()
{
    return gStats->get_achievement(eAward_MapRoom);
}

Stat* GenericStats::camouflage()
{
    return gStats->get_achievement(eAward_Camouflage);
}

Stat* GenericStats::backFromTheDead()
{
    return gStats->get_achievement(eAward_BackFromTheDead);
}

Stat* GenericStats::equipYourFists()
{
    return gStats->get_achievement(eAward_EquipYourFists);
}

Stat* GenericStats::tenaciousBall()
{
    return gStats->get_achievement(eAward_TenaciousBall);
}

Stat* GenericStats::deadshot()
{
    return gStats->get_achievement(eAward_Deadshot);
}

Stat* GenericStats::suckMySnowballs()
{
    return gStats->get_achievement(eAward_SuckMySnowballs);
}

Stat* GenericStats::mountainTop()
{
    return gStats->get_achievement(eAward_MountainTop);
}

Stat* GenericStats::willLikeNoOther()
{
    return gStats->get_achievement(eAward_WillLikeNoOther);
}

Stat* GenericStats::theDeepEnd()
{
    return gStats->get_achievement(eAward_TheDeepEnd);
}

Stat* GenericStats::greatViewFromUpHere()
{
    return gStats->get_achievement(eAward_GreatViewFromUpHere);
}

Stat* GenericStats::changeOfSheets()
{
    return gStats->get_achievement(eAward_ChangeOfSheets);
}

Stat* GenericStats::cheatingDeath()
{
    return gStats->get_achievement(eAward_CheatingDeath);
}

Stat* GenericStats::soIGotThatGoingForMe()
{
    return gStats->get_achievement(eAward_SoIGotThatGoingForMe);
}

Stat* GenericStats::letItGo()
{
    return gStats->get_achievement(eAward_LetItGo);
}

Stat* GenericStats::feelingIll()
{
    return gStats->get_achievement(eAward_FeelingIll);
}

Stat* GenericStats::_1234Pickle()
{
    return gStats->get_achievement(eAward_1234Pickle);
}

Stat* GenericStats::alternativeFuel()
{
    return gStats->get_achievement(eAward_AlternativeFuel);
}

Stat* GenericStats::moskstraumen()
{
    return gStats->get_achievement(eAward_Moskstraumen);
}

Stat* GenericStats::castaway()
{
    return gStats->get_achievement(eAward_Castaway);
}

Stat* GenericStats::sleepWithTheFishes()
{
    return gStats->get_achievement(eAward_SleepWithTheFishes);
}

Stat* GenericStats::echolocation()
{
    return gStats->get_achievement(eAward_Echolocation);
}

arrayWithLength<unsigned char> GenericStats::param_MG03Score(int value)
{
    return gStats->getParam_MG03Score(value);
}

arrayWithLength<unsigned char> GenericStats::param_MG03BestTime(unsigned int value)
{
    return gStats->getParam_MG03BestTime(value);
}

arrayWithLength<unsigned char> GenericStats::param_MG03RingCount(int value)
{
    return gStats->getParam_MG03RingCount(value);
}

arrayWithLength<unsigned char> GenericStats::param_walk(int value)
{
    return gStats->getParam_walkOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_swim(int value)
{
    return gStats->getParam_swimOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_fall(int value)
{
    return gStats->getParam_fallOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_climb(int value)
{
    return gStats->getParam_climbOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_minecart(int value)
{
    return gStats->getParam_minecartOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_boat(int value)
{
    return gStats->getParam_boatOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_pig(int value)
{
    return gStats->getParam_pigOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_cowsMilked()
{
    return gStats->getParam_cowsMilked();
}

arrayWithLength<unsigned char> GenericStats::param_blocksPlaced(const BlockState* state, int count)
{
    int id = Block::getId(state->getBlock());
    int data = state->getBlock()->convertBlockStateToLegacyData(state);
    return gStats->getParam_blocksPlaced(id, data, count);
}

arrayWithLength<unsigned char> GenericStats::param_blocksMined(const BlockState* state, int count)
{
    int id = Block::getId(state->getBlock());
    int data = state->getBlock()->convertBlockStateToLegacyData(state);
    return gStats->getParam_blocksMined(id, data, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsCollected(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsCollected(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsCrafted(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsCrafted(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsSmelted(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsSmelted(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsUsed(boost::shared_ptr<Player> player,
                                                             not_null_ptr<ItemInstance> itemInstance)
{
    if (player && !itemInstance->isEmpty())
    {
        return gStats->getParam_itemsUsed(player, itemInstance);
    }

    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_itemsBought(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsBought(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_mobKill(boost::shared_ptr<Player> player, boost::shared_ptr<Mob> mob,
                                                           const DamageSource* damageSource)
{
    if (player && mob)
    {
        return gStats->getParam_mobKill(player, mob, damageSource);
    }

    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_breedEntity(unsigned long long entityType)
{
    return gStats->getParam_breedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_tamedEntity(unsigned long long entityType)
{
    return gStats->getParam_tamedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_curedEntity(unsigned long long entityType)
{
    return gStats->getParam_curedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_craftedEntity(unsigned long long entityType)
{
    return gStats->getParam_craftedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_shearedEntity(unsigned long long entityType)
{
    return gStats->getParam_shearedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_time(int value)
{
    return gStats->getParam_time(value);
}

arrayWithLength<unsigned char> GenericStats::param_changedDimension(int fromDimension, int toDimension)
{
    return gStats->getParam_changedDimension(fromDimension, toDimension);
}

arrayWithLength<unsigned char> GenericStats::param_enteredBiome(int value)
{
    return gStats->getParam_enteredBiome(value);
}

arrayWithLength<unsigned char> GenericStats::param_noArgs()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_openInventory()
{
    return gStats->getParam_achievement(eAward_OpenInventory);
}

arrayWithLength<unsigned char> GenericStats::param_mineWood()
{
    return gStats->getParam_achievement(eAward_MineWood);
}

arrayWithLength<unsigned char> GenericStats::param_buildWorkbench()
{
    return gStats->getParam_achievement(eAward_BuildWorkbench);
}

arrayWithLength<unsigned char> GenericStats::param_buildPickaxe()
{
    return gStats->getParam_achievement(eAward_BuildPickaxe);
}

arrayWithLength<unsigned char> GenericStats::param_buildFurnace()
{
    return gStats->getParam_achievement(eAward_BuildFurnace);
}

arrayWithLength<unsigned char> GenericStats::param_acquireIron()
{
    return gStats->getParam_achievement(eAward_AcquireIron);
}

arrayWithLength<unsigned char> GenericStats::param_buildHoe()
{
    return gStats->getParam_achievement(eAward_BuildHoe);
}

arrayWithLength<unsigned char> GenericStats::param_makeBread()
{
    return gStats->getParam_achievement(eAward_MakeBread);
}

arrayWithLength<unsigned char> GenericStats::param_bakeCake()
{
    return gStats->getParam_achievement(eAward_BakeCake);
}

arrayWithLength<unsigned char> GenericStats::param_buildBetterPickaxe()
{
    return gStats->getParam_achievement(eAward_BuildBetterPickaxe);
}

arrayWithLength<unsigned char> GenericStats::param_cookFish()
{
    return gStats->getParam_achievement(eAward_CookFish);
}

arrayWithLength<unsigned char> GenericStats::param_onARail(int value)
{
    return gStats->getParam_onARail(value);
}

arrayWithLength<unsigned char> GenericStats::param_buildSword()
{
    return gStats->getParam_achievement(eAward_BuildSword);
}

arrayWithLength<unsigned char> GenericStats::param_killEnemy()
{
    return gStats->getParam_achievement(eAward_KillEnemy);
}

arrayWithLength<unsigned char> GenericStats::param_killCow()
{
    return gStats->getParam_achievement(eAward_KillCow);
}

arrayWithLength<unsigned char> GenericStats::param_flyPig()
{
    return gStats->getParam_achievement(eAward_FlyPig);
}

arrayWithLength<unsigned char> GenericStats::param_snipeSkeleton()
{
    return gStats->getParam_achievement(eAward_SnipeSkeleton);
}

arrayWithLength<unsigned char> GenericStats::param_diamonds()
{
    return gStats->getParam_achievement(eAward_Diamonds);
}

arrayWithLength<unsigned char> GenericStats::param_ghast()
{
    return gStats->getParam_achievement(eAward_Ghast);
}

arrayWithLength<unsigned char> GenericStats::param_blazeRod()
{
    return gStats->getParam_achievement(eAward_BlazeRod);
}

arrayWithLength<unsigned char> GenericStats::param_potion()
{
    return gStats->getParam_achievement(eAward_Potion);
}

arrayWithLength<unsigned char> GenericStats::param_theEnd()
{
    return gStats->getParam_achievement(eAward_TheEnd);
}

arrayWithLength<unsigned char> GenericStats::param_winGame()
{
    return gStats->getParam_achievement(eAward_WinGame);
}

arrayWithLength<unsigned char> GenericStats::param_enchantments()
{
    return gStats->getParam_achievement(eAward_Enchantments);
}

arrayWithLength<unsigned char> GenericStats::param_overkill(int value)
{
    return gStats->getParam_overkill(value);
}

arrayWithLength<unsigned char> GenericStats::param_bookcase()
{
    return gStats->getParam_achievement(eAward_Bookcase);
}

arrayWithLength<unsigned char> GenericStats::param_leaderOfThePack()
{
    return gStats->getParam_achievement(eAward_LeaderOfThePack);
}

arrayWithLength<unsigned char> GenericStats::param_MOARTools()
{
    return gStats->getParam_achievement(eAward_MOARTools);
}

arrayWithLength<unsigned char> GenericStats::param_dispenseWithThis()
{
    return gStats->getParam_achievement(eAward_DispenseWithThis);
}

arrayWithLength<unsigned char> GenericStats::param_InToTheNether()
{
    return gStats->getParam_achievement(eAward_InToTheNether);
}

arrayWithLength<unsigned char> GenericStats::param_socialPost()
{
    return gStats->getParam_achievement(eAward_SocialPost);
}

arrayWithLength<unsigned char> GenericStats::param_mine100Blocks()
{
    return gStats->getParam_achievement(eAward_Mine100Blocks);
}

arrayWithLength<unsigned char> GenericStats::param_kill10Creepers()
{
    return gStats->getParam_achievement(eAward_Kill10Creepers);
}

arrayWithLength<unsigned char> GenericStats::param_adventuringTime()
{
    return gStats->getParam_achievement(eAward_AdventuringTime);
}

arrayWithLength<unsigned char> GenericStats::param_repopulation()
{
    return gStats->getParam_achievement(eAward_Repopulation);
}

arrayWithLength<unsigned char> GenericStats::param_porkChop()
{
    return gStats->getParam_achievement(eAward_PorkChop);
}

arrayWithLength<unsigned char> GenericStats::param_diamondsToYou()
{
    return gStats->getParam_achievement(eAward_DiamondsToYou);
}

arrayWithLength<unsigned char> GenericStats::param_passingTheTime()
{
    return gStats->getParam_achievement(eAward_PassingTheTime);
}

arrayWithLength<unsigned char> GenericStats::param_archer()
{
    return gStats->getParam_achievement(eAward_Archer);
}

arrayWithLength<unsigned char> GenericStats::param_theHaggler()
{
    return gStats->getParam_achievement(eAward_TheHaggler);
}

arrayWithLength<unsigned char> GenericStats::param_potPlanter()
{
    return gStats->getParam_achievement(eAward_PotPlanter);
}

arrayWithLength<unsigned char> GenericStats::param_itsASign()
{
    return gStats->getParam_achievement(eAward_ItsASign);
}

arrayWithLength<unsigned char> GenericStats::param_ironBelly()
{
    return gStats->getParam_achievement(eAward_IronBelly);
}

arrayWithLength<unsigned char> GenericStats::param_haveAShearfulDay()
{
    return gStats->getParam_achievement(eAward_HaveAShearfulDay);
}

arrayWithLength<unsigned char> GenericStats::param_rainbowCollection()
{
    return gStats->getParam_achievement(eAward_RainbowCollection);
}

arrayWithLength<unsigned char> GenericStats::param_stayinFrosty()
{
    return gStats->getParam_achievement(eAward_StayinFrosty);
}

arrayWithLength<unsigned char> GenericStats::param_chestfulOfCobblestone(int value)
{
    return gStats->getParam_chestfulOfCobblestone(value);
}

arrayWithLength<unsigned char> GenericStats::param_renewableEnergy()
{
    return gStats->getParam_achievement(eAward_RenewableEnergy);
}

arrayWithLength<unsigned char> GenericStats::param_musicToMyEars(int value)
{
    return gStats->getParam_musicToMyEars(value);
}

arrayWithLength<unsigned char> GenericStats::param_bodyGuard()
{
    return gStats->getParam_achievement(eAward_BodyGuard);
}

arrayWithLength<unsigned char> GenericStats::param_ironMan()
{
    return gStats->getParam_achievement(eAward_IronMan);
}

arrayWithLength<unsigned char> GenericStats::param_zombieDoctor()
{
    return gStats->getParam_achievement(eAward_ZombieDoctor);
}

arrayWithLength<unsigned char> GenericStats::param_lionTamer()
{
    return gStats->getParam_achievement(eAward_LionTamer);
}

arrayWithLength<unsigned char> GenericStats::param_tieDyeOutfit(int value)
{
    return gStats->getParam_tieDyeOutfit(value);
}

arrayWithLength<unsigned char> GenericStats::param_trampoline(int value)
{
    return gStats->getParam_trampoline(value);
}

arrayWithLength<unsigned char> GenericStats::param_spawnWither()
{
    return gStats->getParam_spawnWither();
}

arrayWithLength<unsigned char> GenericStats::param_killWither()
{
    return gStats->getParam_killWither();
}

arrayWithLength<unsigned char> GenericStats::param_fullBeacon()
{
    return gStats->getParam_fullBeacon();
}

arrayWithLength<unsigned char> GenericStats::param_overpowered()
{
    return gStats->getParam_overpowered();
}

arrayWithLength<unsigned char> GenericStats::param_smeltEverything()
{
    return gStats->getParam_smeltEverything();
}

arrayWithLength<unsigned char> GenericStats::param_freightStation()
{
    return gStats->getParam_freightStation();
}

arrayWithLength<unsigned char> GenericStats::param_theStudent()
{
    return gStats->getParam_theStudent();
}

arrayWithLength<unsigned char> GenericStats::param_hasBecomeTheMaster()
{
    return gStats->getParam_hasBecomeTheMaster();
}

arrayWithLength<unsigned char> GenericStats::param_tisButAScratch()
{
    return gStats->getParam_tisButAScratch();
}

arrayWithLength<unsigned char> GenericStats::param_cupid()
{
    return gStats->getParam_cupid();
}

arrayWithLength<unsigned char> GenericStats::param_youWontLikeMeWhenImHungry()
{
    return gStats->getParam_youWontLikeMeWhenImHungry();
}

arrayWithLength<unsigned char> GenericStats::param_mine()
{
    return gStats->getParam_mine();
}

arrayWithLength<unsigned char> GenericStats::param_theEndAgain()
{
    return gStats->getParam_achievement(eAward_TheEndAgain);
}

arrayWithLength<unsigned char> GenericStats::param_youNeedAMint()
{
    return gStats->getParam_achievement(eAward_YouNeedAMint);
}

arrayWithLength<unsigned char> GenericStats::param_superSonic(int value)
{
    return gStats->getParam_superSonic(value);
}

arrayWithLength<unsigned char> GenericStats::param_drySpell()
{
    return gStats->getParam_achievement(eAward_DrySpell);
}

arrayWithLength<unsigned char> GenericStats::param_freeDiver(int value)
{
    return gStats->getParam_freeDiver(value);
}

arrayWithLength<unsigned char> GenericStats::param_superFuel()
{
    return gStats->getParam_achievement(eAward_SuperFuel);
}

arrayWithLength<unsigned char> GenericStats::param_saddleUp()
{
    return gStats->getParam_achievement(eAward_SaddleUp);
}

arrayWithLength<unsigned char> GenericStats::param_tasteOfYourOwnMedicine()
{
    return gStats->getParam_achievement(eAward_TasteOfYourOwnMedicine);
}

arrayWithLength<unsigned char> GenericStats::param_beamMeUp(int value)
{
    return gStats->getParam_beamMeUp(value);
}

arrayWithLength<unsigned char> GenericStats::param_mapRoom(int value)
{
    return gStats->getParam_mapRoom(value);
}

arrayWithLength<unsigned char> GenericStats::param_camouflage()
{
    return gStats->getParam_achievement(eAward_Camouflage);
}

arrayWithLength<unsigned char> GenericStats::param_backFromTheDead()
{
    return gStats->getParam_achievement(eAward_BackFromTheDead);
}

arrayWithLength<unsigned char> GenericStats::param_equipYourFists()
{
    return gStats->getParam_achievement(eAward_EquipYourFists);
}

arrayWithLength<unsigned char> GenericStats::param_tenaciousBall(int value)
{
    return gStats->getParam_tenaciousBall(value);
}

arrayWithLength<unsigned char> GenericStats::param_deadshot()
{
    return gStats->getParam_achievement(eAward_Deadshot);
}

arrayWithLength<unsigned char> GenericStats::param_suckMySnowballs(int value)
{
    return gStats->getParam_suckMySnowballs(value);
}

arrayWithLength<unsigned char> GenericStats::param_mountainTop()
{
    return gStats->getParam_achievement(eAward_MountainTop);
}

arrayWithLength<unsigned char> GenericStats::param_willLikeNoOther()
{
    return gStats->getParam_achievement(eAward_WillLikeNoOther);
}

arrayWithLength<unsigned char> GenericStats::param_theDeepEnd()
{
    return gStats->getParam_achievement(eAward_TheDeepEnd);
}

arrayWithLength<unsigned char> GenericStats::param_greatViewFromUpHere(int value)
{
    return gStats->getParam_greatViewFromUpHere(value);
}

arrayWithLength<unsigned char> GenericStats::param_soIGotThatGoingForMe(int value)
{
    return gStats->getParam_soIGotThatGoingForMe(value);
}

arrayWithLength<unsigned char> GenericStats::param_letItGo()
{
    return gStats->getParam_achievement(eAward_LetItGo);
}

arrayWithLength<unsigned char> GenericStats::param_feelingIll()
{
    return gStats->getParam_achievement(eAward_FeelingIll);
}

arrayWithLength<unsigned char> GenericStats::param_changeOfSheets()
{
    return gStats->getParam_achievement(eAward_ChangeOfSheets);
}

arrayWithLength<unsigned char> GenericStats::param_1234Pickle()
{
    return gStats->getParam_achievement(eAward_1234Pickle);
}

arrayWithLength<unsigned char> GenericStats::param_alternativeFuel()
{
    return gStats->getParam_achievement(eAward_AlternativeFuel);
}

arrayWithLength<unsigned char> GenericStats::param_moskstraumen()
{
    return gStats->getParam_achievement(eAward_Moskstraumen);
}

arrayWithLength<unsigned char> GenericStats::param_castaway()
{
    return gStats->getParam_achievement(eAward_Castaway);
}

arrayWithLength<unsigned char> GenericStats::param_sleepWithTheFishes()
{
    return gStats->getParam_achievement(eAward_SleepWithTheFishes);
}

arrayWithLength<unsigned char> GenericStats::param_echolocation()
{
    return gStats->getParam_achievement(eAward_Echolocation);
}

Stat* GenericStats::get_achievement(eAward award)
{
    return NULL;
}

Stat* GenericStats::get_walkOneM()
{
    return NULL;
}

Stat* GenericStats::get_swimOneM()
{
    return NULL;
}

Stat* GenericStats::get_fallOneM()
{
    return NULL;
}

Stat* GenericStats::get_climbOneM()
{
    return NULL;
}

Stat* GenericStats::get_minecartOneM()
{
    return NULL;
}

Stat* GenericStats::get_boatOneM()
{
    return NULL;
}

Stat* GenericStats::get_pigOneM()
{
    return NULL;
}

Stat* GenericStats::get_portalsCreated()
{
    return NULL;
}

Stat* GenericStats::get_cowsMilked()
{
    return NULL;
}

Stat* GenericStats::get_netherLavaCollected()
{
    return NULL;
}

Stat* GenericStats::get_killMob()
{
    return NULL;
}

Stat* GenericStats::get_killsZombie()
{
    return NULL;
}

Stat* GenericStats::get_killsSkeleton()
{
    return NULL;
}

Stat* GenericStats::get_killsCreeper()
{
    return NULL;
}

Stat* GenericStats::get_killsSpider()
{
    return NULL;
}

Stat* GenericStats::get_killsSpiderJockey()
{
    return NULL;
}

Stat* GenericStats::get_killsZombiePigman()
{
    return NULL;
}

Stat* GenericStats::get_killsSlime()
{
    return NULL;
}

Stat* GenericStats::get_killsGhast()
{
    return NULL;
}

Stat* GenericStats::get_killsNetherZombiePigman()
{
    return NULL;
}

Stat* GenericStats::get_breedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_tamedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_curedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_craftedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_shearedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_totalBlocksMined()
{
    return NULL;
}

Stat* GenericStats::get_timePlayed()
{
    return NULL;
}

Stat* GenericStats::get_blocksPlaced(int value)
{
    return NULL;
}

Stat* GenericStats::get_blocksMined(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsCollected(int value, int auxValue)
{
    return NULL;
}

Stat* GenericStats::get_itemsCrafted(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsSmelted(int value)
{
    return get_itemsCrafted(value);
}

Stat* GenericStats::get_itemsUsed(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsBought(int value)
{
    return NULL;
}

Stat* GenericStats::get_armourDyed(int value)
{
    return NULL;
}

Stat* GenericStats::get_killsEnderdragon()
{
    return NULL;
}

Stat* GenericStats::get_completeTheEnd()
{
    return NULL;
}

Stat* GenericStats::get_changedDimension(int value, int auxValue)
{
    return NULL;
}

Stat* GenericStats::get_enteredBiome(int value)
{
    return NULL;
}

Stat* GenericStats::get_MG03_Score()
{
    return NULL;
}

Stat* GenericStats::get_MG03_RingCount()
{
    return NULL;
}

Stat* GenericStats::get_MG03_BestTimeScoreChallenge()
{
    return NULL;
}

Stat* GenericStats::get_MG03_BestTime()
{
    return NULL;
}

arrayWithLength<unsigned char> GenericStats::getParam_walkOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_swimOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_fallOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_climbOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_minecartOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_boatOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_pigOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_cowsMilked()
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_blocksPlaced(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_blocksMined(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsCollected(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsCrafted(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsSmelted(int value, int auxValue, int count)
{
    return getParam_itemsCrafted(value, auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsUsed(boost::shared_ptr<Player> player,
                                                                not_null_ptr<ItemInstance> itemInstance)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsBought(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_mobKill(boost::shared_ptr<Player> player,
                                                              boost::shared_ptr<Mob> mob,
                                                              const DamageSource* damageSource)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_breedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_tamedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_curedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_craftedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_shearedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_time(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_changedDimension(int value, int auxValue)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_enteredBiome(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_achievement(eAward award)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_onARail(int value)
{
    return getParam_achievement(eAward_OnARail);
}

arrayWithLength<unsigned char> GenericStats::getParam_overkill(int value)
{
    return getParam_achievement(eAward_Overkill);
}

arrayWithLength<unsigned char> GenericStats::getParam_openInventory(int value)
{
    return getParam_achievement(eAward_OpenInventory);
}

arrayWithLength<unsigned char> GenericStats::getParam_chestfulOfCobblestone(int value)
{
    return getParam_achievement(eAward_ChestfulOfCobblestone);
}

arrayWithLength<unsigned char> GenericStats::getParam_musicToMyEars(int value)
{
    return getParam_achievement(eAward_MusicToMyEars);
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03Score(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03RingCount(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03BestTime(unsigned int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_tieDyeOutfit(int value)
{
    return getParam_achievement(eAward_TieDyeOutfit);
}

arrayWithLength<unsigned char> GenericStats::getParam_trampoline(int value)
{
    return getParam_achievement(eAward_Trampoline);
}

arrayWithLength<unsigned char> GenericStats::getParam_spawnWither()
{
    return getParam_achievement(eAward_SpawnWither);
}

arrayWithLength<unsigned char> GenericStats::getParam_killWither()
{
    return getParam_achievement(eAward_KillWither);
}

arrayWithLength<unsigned char> GenericStats::getParam_fullBeacon()
{
    return getParam_achievement(eAward_FullBeacon);
}

arrayWithLength<unsigned char> GenericStats::getParam_overpowered()
{
    return getParam_achievement(eAward_Overpowered);
}

arrayWithLength<unsigned char> GenericStats::getParam_smeltEverything()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_camouflage()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_freightStation()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_theStudent()
{
    return getParam_achievement(eAward_TheStudent);
}

arrayWithLength<unsigned char> GenericStats::getParam_hasBecomeTheMaster()
{
    return getParam_achievement(eAward_HasBecomeTheMaster);
}

arrayWithLength<unsigned char> GenericStats::getParam_tisButAScratch()
{
    return getParam_achievement(eAward_TisButAScratch);
}

arrayWithLength<unsigned char> GenericStats::getParam_cupid()
{
    return getParam_achievement(eAward_Cupid);
}

arrayWithLength<unsigned char> GenericStats::getParam_youWontLikeMeWhenImHungry()
{
    return getParam_achievement(eAward_YouWontLikeMeWhenImHungry);
}

arrayWithLength<unsigned char> GenericStats::getParam_mine()
{
    return getParam_achievement(eAward_Mine);
}

arrayWithLength<unsigned char> GenericStats::getParam_superSonic(int value)
{
    return getParam_achievement(eAward_SuperSonic);
}

arrayWithLength<unsigned char> GenericStats::getParam_freeDiver(int value)
{
    return getParam_achievement(eAward_FreeDiver);
}

arrayWithLength<unsigned char> GenericStats::getParam_beamMeUp(int value)
{
    return getParam_achievement(eAward_BeamMeUp);
}

arrayWithLength<unsigned char> GenericStats::getParam_mapRoom(int value)
{
    return getParam_achievement(eAward_MapRoom);
}

arrayWithLength<unsigned char> GenericStats::getParam_tenaciousBall(int value)
{
    return getParam_achievement(eAward_TenaciousBall);
}

arrayWithLength<unsigned char> GenericStats::getParam_suckMySnowballs(int value)
{
    return getParam_achievement(eAward_SuckMySnowballs);
}

arrayWithLength<unsigned char> GenericStats::getParam_greatViewFromUpHere(int value)
{
    return getParam_achievement(eAward_GreatViewFromUpHere);
}

arrayWithLength<unsigned char> GenericStats::getParam_soIGotThatGoingForMe(int value)
{
    return getParam_achievement(eAward_SoIGotThatGoingForMe);
}

arrayWithLength<unsigned char> GenericStats::getParam_noArgs()
{
    return arrayWithLength<unsigned char>();
}

Stat* GenericStats::itemsCollected(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    int auxValue = itemInstance->getAuxValue();
    return gStats->get_itemsCollected(Item::VALIDgetId(item), auxValue);
}

arrayWithLength<unsigned char> GenericStats::param_itemsCollected(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    int auxValue = itemInstance->getAuxValue();
    int count = itemInstance->getCount();
    return param_itemsCollected(item, auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsBought(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    int auxValue = itemInstance->getAuxValue();
    int count = itemInstance->getCount();
    return param_itemsBought(item, auxValue, count);
}
