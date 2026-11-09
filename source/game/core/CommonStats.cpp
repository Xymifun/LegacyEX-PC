#include "game/core/CommonStats.hpp"
#include "game/core/EntityType.hpp"

#include <cstring>

#include "game/core/Achievement.hpp"
#include "game/core/Achievements.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

CommonStats::~CommonStats()
{
}

Stat* CommonStats::get_stat(int id)
{
    return Stats::get(id);
}

Stat* CommonStats::get_walkOneM()
{
    return Stats::walkOneM;
}

Stat* CommonStats::get_swimOneM()
{
    return Stats::swimOneM;
}

Stat* CommonStats::get_fallOneM()
{
    return Stats::fallOneM;
}

Stat* CommonStats::get_climbOneM()
{
    return Stats::climbOneM;
}

Stat* CommonStats::get_minecartOneM()
{
    return Stats::minecartOneM;
}

Stat* CommonStats::get_boatOneM()
{
    return Stats::boatOneM;
}

Stat* CommonStats::get_pigOneM()
{
    return Stats::pigOneM;
}

Stat* CommonStats::get_portalsCreated()
{
    return Stats::portalsUsed;
}

Stat* CommonStats::get_cowsMilked()
{
    return Stats::cowsMilked;
}

Stat* CommonStats::get_netherLavaCollected()
{
    return Stats::netherLavaCollected;
}

Stat* CommonStats::get_killsZombie()
{
    return Stats::killsZombie;
}

Stat* CommonStats::get_killsSkeleton()
{
    return Stats::killsSkeleton;
}

Stat* CommonStats::get_killsCreeper()
{
    return Stats::killsCreeper;
}

Stat* CommonStats::get_killsSpider()
{
    return Stats::killsSpider;
}

Stat* CommonStats::get_killsSpiderJockey()
{
    return Stats::killsSpiderJockey;
}

Stat* CommonStats::get_killsZombiePigman()
{
    return Stats::killsZombiePigman;
}

Stat* CommonStats::get_killsSlime()
{
    return Stats::killsSlime;
}

Stat* CommonStats::get_killsGhast()
{
    return Stats::killsGhast;
}

Stat* CommonStats::get_killsNetherZombiePigman()
{
    return Stats::killsNetherZombiePigman;
}

Stat* CommonStats::get_breedEntity(unsigned long long entityType)
{
    if (entityType == EntityType::COW)
    {
        return GenericStats::repopulation();
    }
    return NULL;
}

Stat* CommonStats::get_tamedEntity(unsigned long long entityType)
{
    if (entityType == EntityType::OCELOT)
    {
        return GenericStats::lionTamer();
    }
    if (entityType == EntityType::WOLF)
    {
        return Stats::befriendsWolf;
    }
    if (entityType == EntityType::HORSE)
    {
        return Achievements::saddleUp;
    }
    return NULL;
}

Stat* CommonStats::get_craftedEntity(unsigned long long entityType)
{
    if (entityType == EntityType::VILLAGER_GOLEM)
    {
        return GenericStats::bodyGuard();
    }
    if (entityType == EntityType::WITHER_BOSS)
    {
        return GenericStats::spawnWither();
    }
    return NULL;
}

Stat* CommonStats::get_shearedEntity(unsigned long long entityType)
{
    if (entityType == EntityType::SHEEP)
    {
        return GenericStats::haveAShearfulDay();
    }
    return NULL;
}

Stat* CommonStats::get_totalBlocksMined()
{
    return Stats::totalBlocksMined;
}

Stat* CommonStats::get_timePlayed()
{
    return Stats::timePlayed;
}

Stat* CommonStats::get_blocksPlaced(int blockId)
{
#ifdef MINECRAFT_PC
    if (blockId < 0 || blockId >= static_cast<int>(Stats::blockPlacedStats.length))
        return NULL;
#endif
    return Stats::blockPlacedStats.data[blockId];
}

Stat* CommonStats::get_blocksMined(int blockId)
{
    if (blockId < 256)
    {
        return Stats::blockMinedStats.data[blockId];
    }
    return NULL;
}

Stat* CommonStats::get_itemsCollected(int itemId, int auxValue)
{
    if (itemId == BlockId::WOOL)
    {
        return Stats::rainbowCollectionStats.data[auxValue];
    }
    if (itemId != Item::getId(Items::emerald))
    {
        return Stats::itemCollectedStats.data[itemId];
    }
    return NULL;
}

Stat* CommonStats::get_itemsCrafted(int itemId)
{
    return Stats::itemCraftedStats.data[itemId];
}

Stat* CommonStats::get_itemsSmelted(int itemId)
{
    return Stats::itemCraftedStats.data[itemId];
}

Stat* CommonStats::get_itemsUsed(int itemId)
{
    if (itemId == ItemId::COOKED_PORKCHOP || itemId == ItemId::COOKED_RABBIT)
    {
        return Stats::blockPlacedStats.data[itemId];
    }
    if (itemId == ItemId::TOTEM_OF_UNDYING)
    {
        return GenericStats::cheatingDeath();
    }
    return NULL;
}

Stat* CommonStats::get_itemsBought(int itemId)
{
    if (itemId == Item::getId(Items::emerald))
    {
        return Stats::itemCollectedStats.data[itemId];
    }
    return NULL;
}

Stat* CommonStats::get_armourDyed(int itemId)
{
    switch (itemId)
    {
    case 298:
        return Stats::armourDyedStats.data[0];
    case 299:
        return Stats::armourDyedStats.data[1];
    case 300:
        return Stats::armourDyedStats.data[2];
    case 301:
        return Stats::armourDyedStats.data[3];
    }
    return NULL;
}

Stat* CommonStats::get_killsEnderdragon()
{
    return Stats::killsEnderdragon;
}

Stat* CommonStats::get_completeTheEnd()
{
    return Stats::completeTheEnd;
}

Stat* CommonStats::get_enteredBiome(int biomeId)
{
    switch (biomeId)
    {
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        return Stats::biomesVisitedStats.data[0];
    }
    if (biomeId > 40)
    {
        biomeId -= 128;
    }
    return Stats::biomesVisitedStats.data[biomeId];
}

Stat* CommonStats::get_achievement(eAward award)
{
    switch (award)
    {
    case 0:
        return Achievements::openInventory;
    case 1:
        return Achievements::mineWood;
    case 2:
        return Achievements::buildWorkBench;
    case 3:
        return Achievements::buildPickaxe;
    case 4:
        return Achievements::buildFurnace;
    case 5:
        return Achievements::acquireIron;
    case 6:
        return Achievements::buildHoe;
    case 7:
        return Achievements::makeBread;
    case 8:
        return Achievements::bakeCake;
    case 9:
        return Achievements::buildBetterPickaxe;
    case 10:
        return Achievements::cookFish;
    case 11:
        return Achievements::onARail;
    case 12:
        return Achievements::buildSword;
    case 13:
        return Achievements::killEnemy;
    case 14:
        return Achievements::killCow;
    case 15:
        return Achievements::flyPig;
    case 16:
        return Achievements::leaderOfThePack;
    case 17:
        return Achievements::MOARTools;
    case 18:
        return Achievements::dispenseWithThis;
    case 19:
        return Achievements::InToTheNether;
    case 20:
        return Achievements::mine100Blocks;
    case 21:
        return Achievements::kill10Creepers;
    case 22:
        return Achievements::eatPorkChop;
    case 23:
        return Achievements::play100Days;
    case 24:
        return Achievements::arrowKillCreeper;
    case 25:
        return Achievements::socialPost;
    case 26:
        return Achievements::snipeSkeleton;
    case 27:
        return Achievements::diamonds;
    case 29:
        return Achievements::ghast;
    case 30:
        return Achievements::blazeRod;
    case 31:
        return Achievements::potion;
    case 32:
        return Achievements::theEnd;
    case 33:
        return Achievements::theEnd2;
    case 34:
        return Achievements::enchantments;
    case 35:
        return Achievements::overkill;
    case 36:
        return Achievements::bookcase;
    case 37:
        return Achievements::adventuringTime;
    case 38:
        return Achievements::repopulation;
    case 39:
        return Achievements::diamondsToYou;
    case 40:
        return Achievements::theHaggler;
    case 41:
        return Achievements::potPlanter;
    case 42:
        return Achievements::itsASign;
    case 43:
        return Achievements::ironBelly;
    case 44:
        return Achievements::haveAShearfulDay;
    case 45:
        return Achievements::rainbowCollection;
    case 46:
        return Achievements::stayingFrosty;
    case 47:
        return Achievements::chestfulOfCobblestone;
    case 48:
        return Achievements::renewableEnergy;
    case 49:
        return Achievements::musicToMyEars;
    case 50:
        return Achievements::bodyGuard;
    case 51:
        return Achievements::ironMan;
    case 52:
        return Achievements::zombieDoctor;
    case 53:
        return Achievements::lionTamer;
    case 54:
        return Achievements::spawnWither;
    case 55:
        return Achievements::killWither;
    case 56:
        return Achievements::fullBeacon;
    case 57:
        return Achievements::overpowered;
    case 58:
        return Achievements::theDyeOutfit;
    case 59:
        return Achievements::trampoline;
    case 66:
        return Achievements::theEndAgain;
    case 67:
        return Achievements::youNeedAMint;
    case 68:
        return Achievements::superSonic;
    case 69:
        return Achievements::drySpell;
    case 70:
        return Achievements::freeDiver;
    case 71:
        return Achievements::superFuel;
    case 72:
        return Achievements::saddleUp;
    case 73:
        return Achievements::tasteOfYourOwnMedicine;
    case 74:
        return Achievements::beamMeUp;
    case 75:
        return Achievements::mapRoom;
    case 76:
        return Achievements::camouflage;
    case 77:
        return Achievements::backFromTheDead;
    case 78:
        return Achievements::equipYourFists;
    case 79:
        return Achievements::tenaciousBall;
    case 80:
        return Achievements::deadshot;
    case 81:
        return Achievements::suckMySnowballs;
    case 82:
        return Achievements::mountainTop;
    case 83:
        return Achievements::willLikeNoOther;
    case 84:
        return Achievements::theDeepEnd;
    case 85:
        return Achievements::greatViewFromUpHere;
    case 86:
        return Achievements::changeOfSheets;
    case 87:
        return Achievements::cheatingDeath;
    case 88:
        return Achievements::soIGotThatGoingForMe;
    case 89:
        return Achievements::letItGo;
    case 90:
        return Achievements::feelingIll;
    case 60:
        return Achievements::theStudent;
    case 61:
        return Achievements::hasBecomeTheMaster;
    case 62:
        return Achievements::tisButAScratch;
    case 63:
        return Achievements::cupid;
    case 64:
        return Achievements::youWontLikeMeWhenImHungry;
    case 65:
        return Achievements::mine;
    case 91:
        return Achievements::_1234Pickle;
    case 92:
        return Achievements::alternativeFuel;
    case 93:
        return Achievements::moskstraumen;
    case 94:
        return Achievements::castaway;
    case 95:
        return Achievements::sleepWithTheFishes;
    case 96:
        return Achievements::echolocation;
    default:
        return NULL;
    }
}

Stat* CommonStats::get_MG03_Score()
{
    return Stats::mg03Score;
}

Stat* CommonStats::get_MG03_RingCount()
{
    return Stats::mg03RingCount;
}

Stat* CommonStats::get_MG03_BestTimeScoreChallenge()
{
    return Stats::mg03BestTimeScoreChallenge;
}

Stat* CommonStats::get_MG03_BestTime()
{
    return Stats::mg03BestTime;
}

arrayWithLength<unsigned char> CommonStats::makeParam(int value)
{
    arrayWithLength<unsigned char> result(4, true);
    std::memcpy(result.data, &value, 4);
    return result;
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03Score(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03RingCount(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03BestTime(unsigned int value)
{
    arrayWithLength<unsigned char> result(4, true);
    std::memcpy(result.data, &value, 4);
    return result;
}

arrayWithLength<unsigned char> CommonStats::getParam_walkOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_swimOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_fallOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_climbOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_minecartOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_boatOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_pigOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_blocksMined(int blockId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsCollected(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsCrafted(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsSmelted(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsBought(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_time(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_noArgs()
{
    return makeParam(1);
}

unsigned int CommonStats::readParam(arrayWithLength<unsigned char> value)
{
    if (value.length != 4)
    {
        return 1;
    }
    return *reinterpret_cast<unsigned int*>(value.data);
}
