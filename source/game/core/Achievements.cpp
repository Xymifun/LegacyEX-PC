#include "game/core/Achievements.hpp"

#include "game/core/Achievement.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Blocks.hpp"

#include <cstddef>

Achievement* Achievements::openInventory = NULL;
Achievement* Achievements::mineWood = NULL;
Achievement* Achievements::buildWorkBench = NULL;
Achievement* Achievements::buildPickaxe = NULL;
Achievement* Achievements::buildFurnace = NULL;
Achievement* Achievements::acquireIron = NULL;
Achievement* Achievements::buildHoe = NULL;
Achievement* Achievements::makeBread = NULL;
Achievement* Achievements::bakeCake = NULL;
Achievement* Achievements::buildBetterPickaxe = NULL;
Achievement* Achievements::cookFish = NULL;
Achievement* Achievements::onARail = NULL;
Achievement* Achievements::buildSword = NULL;
Achievement* Achievements::killEnemy = NULL;
Achievement* Achievements::killCow = NULL;
Achievement* Achievements::flyPig = NULL;
Achievement* Achievements::snipeSkeleton = NULL;
Achievement* Achievements::diamonds = NULL;
Achievement* Achievements::ghast = NULL;
Achievement* Achievements::blazeRod = NULL;
Achievement* Achievements::potion = NULL;
Achievement* Achievements::theEnd = NULL;
Achievement* Achievements::theEnd2 = NULL;
Achievement* Achievements::enchantments = NULL;
Achievement* Achievements::leaderOfThePack = NULL;
Achievement* Achievements::MOARTools = NULL;
Achievement* Achievements::dispenseWithThis = NULL;
Achievement* Achievements::InToTheNether = NULL;
Achievement* Achievements::socialPost = NULL;
Achievement* Achievements::eatPorkChop = NULL;
Achievement* Achievements::play100Days = NULL;
Achievement* Achievements::arrowKillCreeper = NULL;
Achievement* Achievements::mine100Blocks = NULL;
Achievement* Achievements::kill10Creepers = NULL;
Achievement* Achievements::overkill = NULL;
Achievement* Achievements::bookcase = NULL;
Achievement* Achievements::adventuringTime = NULL;
Achievement* Achievements::repopulation = NULL;
Achievement* Achievements::diamondsToYou = NULL;
Achievement* Achievements::theHaggler = NULL;
Achievement* Achievements::potPlanter = NULL;
Achievement* Achievements::itsASign = NULL;
Achievement* Achievements::ironBelly = NULL;
Achievement* Achievements::haveAShearfulDay = NULL;
Achievement* Achievements::rainbowCollection = NULL;
Achievement* Achievements::stayingFrosty = NULL;
Achievement* Achievements::chestfulOfCobblestone = NULL;
Achievement* Achievements::renewableEnergy = NULL;
Achievement* Achievements::musicToMyEars = NULL;
Achievement* Achievements::bodyGuard = NULL;
Achievement* Achievements::ironMan = NULL;
Achievement* Achievements::zombieDoctor = NULL;
Achievement* Achievements::lionTamer = NULL;
Achievement* Achievements::breedCow = NULL;
Achievement* Achievements::spawnWither = NULL;
Achievement* Achievements::killWither = NULL;
Achievement* Achievements::fullBeacon = NULL;
Achievement* Achievements::exploreAllBiomes = NULL;
Achievement* Achievements::overpowered = NULL;
Achievement* Achievements::theDyeOutfit = NULL;
Achievement* Achievements::trampoline = NULL;
Achievement* Achievements::theStudent = NULL;
Achievement* Achievements::hasBecomeTheMaster = NULL;
Achievement* Achievements::tisButAScratch = NULL;
Achievement* Achievements::cupid = NULL;
Achievement* Achievements::youWontLikeMeWhenImHungry = NULL;
Achievement* Achievements::mine = NULL;
Achievement* Achievements::theEndAgain = NULL;
Achievement* Achievements::youNeedAMint = NULL;
Achievement* Achievements::superSonic = NULL;
Achievement* Achievements::drySpell = NULL;
Achievement* Achievements::freeDiver = NULL;
Achievement* Achievements::superFuel = NULL;
Achievement* Achievements::saddleUp = NULL;
Achievement* Achievements::tasteOfYourOwnMedicine = NULL;
Achievement* Achievements::beamMeUp = NULL;
Achievement* Achievements::mapRoom = NULL;
Achievement* Achievements::camouflage = NULL;
Achievement* Achievements::backFromTheDead = NULL;
Achievement* Achievements::equipYourFists = NULL;
Achievement* Achievements::tenaciousBall = NULL;
Achievement* Achievements::deadshot = NULL;
Achievement* Achievements::suckMySnowballs = NULL;
Achievement* Achievements::mountainTop = NULL;
Achievement* Achievements::willLikeNoOther = NULL;
Achievement* Achievements::theDeepEnd = NULL;
Achievement* Achievements::greatViewFromUpHere = NULL;
Achievement* Achievements::changeOfSheets = NULL;
Achievement* Achievements::cheatingDeath = NULL;
Achievement* Achievements::soIGotThatGoingForMe = NULL;
Achievement* Achievements::letItGo = NULL;
Achievement* Achievements::feelingIll = NULL;
Achievement* Achievements::_1234Pickle = NULL;
Achievement* Achievements::alternativeFuel = NULL;
Achievement* Achievements::moskstraumen = NULL;
Achievement* Achievements::castaway = NULL;
Achievement* Achievements::sleepWithTheFishes = NULL;
Achievement* Achievements::echolocation = NULL;

void Achievements::staticCtor()
{
    Achievement* achievement;

    openInventory =
        (new Achievement(0, L"openInventory", 0, 0, Items::book, NULL))->setAwardLocallyOnly()->postConstruct(true);

    mineWood = (new Achievement(1, L"mineWood", 2, 1, Blocks::log, openInventory))->postConstruct(true);

    buildWorkBench =
        (new Achievement(2, L"buildWorkBench", 4, -1, Blocks::crafting_table, mineWood))->postConstruct(true);

    buildPickaxe =
        (new Achievement(3, L"buildPickaxe", 4, 2, Items::woodenPickaxe, buildWorkBench))->postConstruct(true);

    buildFurnace = (new Achievement(4, L"buildFurnace", 3, 4, Blocks::furnace, buildPickaxe))->postConstruct(true);

    acquireIron = (new Achievement(5, L"acquireIron", 1, 4, Items::ironIngot, buildFurnace))->postConstruct(true);

    buildHoe = (new Achievement(6, L"buildHoe", 2, -3, Items::woodenHoe, buildWorkBench))->postConstruct(true);

    makeBread = (new Achievement(7, L"makeBread", -1, -3, Items::bread, buildHoe))->postConstruct(true);

    bakeCake = (new Achievement(8, L"bakeCake", 0, -5, Items::cake, buildHoe))->postConstruct(true);

    buildBetterPickaxe =
        (new Achievement(9, L"buildBetterPickaxe", 6, 2, Items::stonePickaxe, buildPickaxe))->postConstruct(true);

    cookFish = (new Achievement(10, L"cookFish", 2, 6, Items::cookedFish, buildFurnace))->postConstruct(true);

    achievement = new Achievement(11, L"onARail", 2, 3, Blocks::rail, acquireIron);
    achievement->setGolden();
    onARail = achievement->postConstruct(true);

    buildSword = (new Achievement(12, L"buildSword", 6, -1, Items::woodenSword, buildWorkBench))->postConstruct(true);

    killEnemy = (new Achievement(13, L"killEnemy", 8, -1, Items::bone, buildSword))->postConstruct(true);

    killCow = (new Achievement(14, L"killCow", 7, -3, Items::leather, buildSword))->postConstruct(true);

    achievement = new Achievement(15, L"flyPig", 8, -4, Items::saddle, killCow);
    achievement->setGolden();
    flyPig = achievement->postConstruct(true);

    leaderOfThePack = (new Achievement(16, L"leaderOfThePack", 0, 0, Blocks::log, buildSword))
                          ->setAwardLocallyOnly()
                          ->postConstruct(true);

    MOARTools =
        (new Achievement(17, L"MOARTools", 0, 0, Blocks::log, buildSword))->setAwardLocallyOnly()->postConstruct(true);

    dispenseWithThis = (new Achievement(18, L"dispenseWithThis", 0, 0, Blocks::log, buildSword))->postConstruct(true);

    InToTheNether = (new Achievement(19, L"InToTheNether", 0, 0, Blocks::log, buildSword))->postConstruct(true);

    mine100Blocks = (new Achievement(20, L"mine100Blocks", 0, 0, Blocks::log, buildSword))
                        ->setAwardLocallyOnly()
                        ->postConstruct(true);

    kill10Creepers = (new Achievement(21, L"kill10Creepers", 0, 0, Blocks::log, buildSword))
                         ->setAwardLocallyOnly()
                         ->postConstruct(true);

    eatPorkChop = (new Achievement(22, L"eatPorkChop", 0, 0, Blocks::log, buildSword))
                      ->setAwardLocallyOnly()
                      ->postConstruct(true);

    play100Days = (new Achievement(23, L"play100Days", 0, 0, Blocks::log, buildSword))
                      ->setAwardLocallyOnly()
                      ->postConstruct(true);

    arrowKillCreeper = (new Achievement(24, L"arrowKillCreeper", 0, 0, Blocks::log, buildSword))->postConstruct(true);

    socialPost = (new Achievement(25, L"socialPost", 0, 0, Blocks::log, buildSword))->postConstruct(true);

    achievement = new Achievement(26, L"snipeSkeleton", 7, 0, Items::bow, killEnemy);
    achievement->setGolden();
    snipeSkeleton = achievement->postConstruct(true)->setExtended();

    diamonds =
        (new Achievement(27, L"diamonds", -1, 5, Items::diamond, acquireIron))->postConstruct(true)->setExtended();

    achievement = new Achievement(29, L"ghast", -4, 8, Items::ghastTear, ghast);
    achievement->setGolden();
    ghast = achievement->postConstruct(true)->setExtended();

    blazeRod = (new Achievement(30, L"blazeRod", 0, 9, Items::blazeRod, blazeRod))->postConstruct(true)->setExtended();

    potion = (new Achievement(31, L"potion", 2, 8, Items::potion, potion))->postConstruct(true)->setExtended();

    achievement = new Achievement(32, L"theEnd", 3, 10, Items::enderEye, theEnd);
    achievement->setGolden();
    theEnd = achievement->postConstruct(true)->setExtended();

    achievement = new Achievement(33, L"theEnd2", 4, 13, Blocks::dragon_egg, theEnd2);
    achievement->setGolden();
    theEnd2 = achievement->postConstruct(true)->setExtended();

    enchantments = (new Achievement(34, L"enchantments", -4, 4, Blocks::enchanting_table, enchantments))
                       ->postConstruct(true)
                       ->setExtended();

    achievement = new Achievement(35, L"overkill", -4, 1, Items::diamondSword, enchantments);
    achievement->setGolden();
    overkill = achievement->postConstruct(true)->setExtended();

    bookcase =
        (new Achievement(36, L"bookcase", -3, 6, Blocks::bookshelf, enchantments))->postConstruct(true)->setExtended();

    adventuringTime = (new Achievement(37, L"adventuringTime", 0, 0, Blocks::bookshelf, NULL))
                          ->setAwardLocallyOnly()
                          ->postConstruct(true)
                          ->setExtended();

    repopulation =
        (new Achievement(38, L"repopulation", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    diamondsToYou =
        (new Achievement(39, L"diamondsToYou", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    theHaggler = (new Achievement(40, L"theHaggler", 0, 0, Blocks::bookshelf, NULL))
                     ->setAwardLocallyOnly()
                     ->postConstruct(true)
                     ->setExtended();

    potPlanter = (new Achievement(41, L"potPlanter", 0, 0, Blocks::bookshelf, NULL))
                     ->setAwardLocallyOnly()
                     ->postConstruct(true)
                     ->setExtended();

    itsASign = (new Achievement(42, L"itsASign", 0, 0, Blocks::bookshelf, NULL))
                   ->setAwardLocallyOnly()
                   ->postConstruct(true)
                   ->setExtended();

    ironBelly = (new Achievement(43, L"ironBelly", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    haveAShearfulDay =
        (new Achievement(44, L"haveAShearfulDay", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    rainbowCollection = (new Achievement(45, L"rainbowCollection", 0, 0, Blocks::bookshelf, NULL))
                            ->setAwardLocallyOnly()
                            ->postConstruct(true)
                            ->setExtended();

    stayingFrosty =
        (new Achievement(46, L"stayingFrosty", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    chestfulOfCobblestone = (new Achievement(47, L"chestfulOfCobblestone", 0, 0, Blocks::bookshelf, NULL))
                                ->setAwardLocallyOnly()
                                ->postConstruct(true)
                                ->setExtended();

    renewableEnergy =
        (new Achievement(48, L"renewableEnergy", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    musicToMyEars =
        (new Achievement(49, L"musicToMyEars", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    bodyGuard = (new Achievement(50, L"bodyGuard", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    ironMan = (new Achievement(51, L"ironMan", 0, 0, Blocks::bookshelf, NULL))
                  ->setAwardLocallyOnly()
                  ->postConstruct(true)
                  ->setExtended();

    zombieDoctor =
        (new Achievement(52, L"zombieDoctor", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    lionTamer = (new Achievement(53, L"lionTamer", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    breedCow = repopulation;

    spawnWither =
        (new Achievement(54, L"spawnWither", 0, 0, Blocks::bookshelf, NULL))->postConstruct(true)->setExtended();

    killWither =
        (new Achievement(55, L"killWither", 0, 0, Blocks::bookshelf, spawnWither))->postConstruct(true)->setExtended();

    fullBeacon =
        (new Achievement(56, L"fullBeacon", 0, 0, Blocks::bookshelf, killWither))->postConstruct(true)->setExtended();

    exploreAllBiomes = adventuringTime;

    overpowered = (new Achievement(57, L"overpowered", 0, 0, Blocks::bookshelf, buildBetterPickaxe))
                      ->postConstruct(true)
                      ->setExtended();

    theDyeOutfit = (new Achievement(58, L"theDyeOutfit", 0, 0, Blocks::bookshelf, NULL))
                       ->setAwardLocallyOnly()
                       ->postConstruct(true)
                       ->setExtended();

    trampoline = (new Achievement(59, L"trampoline", 0, 0, Blocks::bookshelf, NULL))
                     ->setAwardLocallyOnly()
                     ->postConstruct(true)
                     ->setExtended();

    theStudent = (new Achievement(60, L"theStudent", 0, 0, Blocks::log, NULL))
                     ->setAwardLocallyOnly()
                     ->postConstruct(true)
                     ->setExtended();

    hasBecomeTheMaster = (new Achievement(61, L"hasBecomeTheMaster", 0, 0, Blocks::log, NULL))
                             ->setAwardLocallyOnly()
                             ->postConstruct(true)
                             ->setExtended();

    tisButAScratch =
        (new Achievement(62, L"tisButAScratch", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    cupid = (new Achievement(63, L"cupid", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    youWontLikeMeWhenImHungry = (new Achievement(64, L"youWontLikeMeWhenImHungry,", 0, 0, Blocks::log, NULL))
                                    ->postConstruct(true)
                                    ->setExtended();

    mine = (new Achievement(65, L"mine,", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    theEndAgain = (new Achievement(66, L"theEndAgain", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    youNeedAMint = (new Achievement(67, L"youNeedAMint", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    superSonic = (new Achievement(68, L"superSonic", 0, 0, Blocks::log, NULL))
                     ->setAwardLocallyOnly()
                     ->postConstruct(true)
                     ->setExtended();

    drySpell = (new Achievement(69, L"drySpell", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    freeDiver = (new Achievement(70, L"freeDiver", 0, 0, Blocks::log, NULL))
                    ->setAwardLocallyOnly()
                    ->postConstruct(true)
                    ->setExtended();

    superFuel = (new Achievement(71, L"superFuel", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    saddleUp = (new Achievement(72, L"saddleUp", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    tasteOfYourOwnMedicine =
        (new Achievement(73, L"tasteOfYourOwnMedicine", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    beamMeUp = (new Achievement(74, L"beamMeUp", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    mapRoom = (new Achievement(75, L"mapRoom", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    camouflage = (new Achievement(76, L"camouflage", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    backFromTheDead =
        (new Achievement(77, L"backFromTheDead", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    equipYourFists =
        (new Achievement(78, L"equipYourFists", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    tenaciousBall =
        (new Achievement(79, L"tenaciousBall", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    deadshot = (new Achievement(80, L"deadshot", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    suckMySnowballs =
        (new Achievement(81, L"suckMySnowballs", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    mountainTop = (new Achievement(82, L"mountainTop", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    willLikeNoOther =
        (new Achievement(83, L"willLikeNoOther", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    theDeepEnd = (new Achievement(84, L"theDeepEnd", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    greatViewFromUpHere =
        (new Achievement(85, L"greatViewFromUpHere", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    changeOfSheets =
        (new Achievement(86, L"changeOfSheets", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    cheatingDeath =
        (new Achievement(87, L"cheatingDeath", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    soIGotThatGoingForMe =
        (new Achievement(88, L"soIGotThatGoingForMe", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    letItGo = (new Achievement(89, L"letItGo", 0, 0, Blocks::log, NULL))
                  ->setAwardLocallyOnly()
                  ->postConstruct(true)
                  ->setExtended();

    feelingIll = (new Achievement(90, L"feelingIll", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    _1234Pickle = (new Achievement(91, L"_1234Pickle", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    alternativeFuel =
        (new Achievement(92, L"alternativeFuel", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    moskstraumen = (new Achievement(93, L"moskstraumen", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    castaway = (new Achievement(94, L"castaway", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    sleepWithTheFishes =
        (new Achievement(95, L"sleepWithTheFishes", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();

    echolocation = (new Achievement(96, L"echolocation", 0, 0, Blocks::log, NULL))->postConstruct(true)->setExtended();
}

void Achievements::init()
{
}
