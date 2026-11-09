#include "game/minigame/MiniGameMedals.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/FastestTimeAndHightestScoreInARoundCondition.hpp"
#include "game/core/FinishCourseWithDamageTakenCondition.hpp"
#include "game/core/FinishedLastRoundCondition.hpp"
#include "game/core/ImprovedCondition.hpp"
#include "game/core/LastInFirstRoundCondition.hpp"
#include "game/core/NotFastestTimeButWonAnywayCondition.hpp"
#include "game/core/NotWinnerCondition.hpp"
#include "game/core/ReachEndTimeCondition.hpp"
#include "game/core/TimeSpentCondition.hpp"
#include "game/core/TopSpeedCondition.hpp"
#include "game/core/TotalDamageTakenCondition.hpp"
#include "game/core/TotalDeathsCondition.hpp"
#include "game/core/TotalPointsCondition.hpp"
#include "game/core/TotalThermalsEnteredCondition.hpp"
#include "game/core/VariantCondition.hpp"
#include "game/core/WonByAmountCondition.hpp"
#include "game/entity/projectile/HitsFromProjectileMedal.hpp"
#include "game/entity/projectile/ProjectileDirectHitMedal.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/AmphibiousMedal.hpp"
#include "game/minigame/ClimbingMedal.hpp"
#include "game/minigame/ConsolationMedal.hpp"
#include "game/minigame/DamageDealtMedal.hpp"
#include "game/minigame/DamageSourceMedal.hpp"
#include "game/minigame/DistanceMedal.hpp"
#include "game/minigame/FallKillMedal.hpp"
#include "game/minigame/FirstToBottomAndWonMedal.hpp"
#include "game/minigame/FirstToBottomMedal.hpp"
#include "game/minigame/FirstToDieMedal.hpp"
#include "game/minigame/FoodMedal.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/HoarderMedal.hpp"
#include "game/minigame/ImprovementMedal.hpp"
#include "game/minigame/JumpsMedal.hpp"
#include "game/minigame/LeastKillsMedal.hpp"
#include "game/minigame/LifeSpanMedal.hpp"
#include "game/minigame/LongWayDownMedal.hpp"
#include "game/minigame/MedalAdder.hpp"
#include "game/minigame/MissedSnowballsMedal.hpp"
#include "game/minigame/MultiConditionMedal.hpp"
#include "game/minigame/MostKillsMedal.hpp"
#include "game/minigame/RingsMedal.hpp"
#include "game/minigame/SneakingMedal.hpp"
#include "game/minigame/SniperMedal.hpp"
#include "game/minigame/TankMedal.hpp"
#include "game/minigame/TreasureHunterMedal.hpp"
#include "game/minigame/UnderwaterMedal.hpp"
#include "game/minigame/WeaponMedal.hpp"
#include "game/world/block/BlockDestroyedMedal.hpp"
#include "game/world/block/BlockPlacedMedal.hpp"
#include "game/world/block/BlockPlacedOverTime.hpp"
#include "game/world/block/BlocksDestroyedFromBelowMedal.hpp"
#include "game/world/block/FirstDeathWithoutBlockDestructionMedal.hpp"
#include "game/world/block/PodiumPlaceMostBlocksDestroyedMedal.hpp"
#include "game/world/block/WinnerWithoutBlockDestructionMedal.hpp"
#include "game/world/generation/PlayerCountCondition.hpp"
#include "game/world/generation/TimeInLayerCentreMedal.hpp"
#include "game/world/generation/TimeSpentOnLayerMedal.hpp"
#include "game/world/generation/WinningOnTopLayerMedal.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/HasSpawnedAtAllCheckpointsCondition.hpp"
#include "game/item/Item.hpp"
#include "game/item/PotionMedal.hpp"
#include "game/item/SplashPotionMedal.hpp"

#include <cfloat>
#include <cstddef>
#include <vector>

std::vector<const GameMedal*> gAllMiniGameMedals;
std::vector<const GameMedal*> gBattleMedals;
std::vector<const GameMedal*> gTumbleMedals;
std::vector<const GameMedal*> gGlideMedals;
std::vector<const GameMedal*> gBuildMedals;

int gBattleMedalCount;
int gTumbleMedalCount;
int gGlideMedalCount;
int gBuildMedalCount;

void MiniGameMedals::staticCtor()
{
    int nextMedalId = 0;

    MedalAdder battleMedals(&nextMedalId, &gBattleMedals, &gAllMiniGameMedals);

    battleMedals.addMedal(
        new MostKillsMedal(IDS_MEDAL_MOST_KILLS, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(
        new LeastKillsMedal(IDS_MEDAL_LEAST_KILLS, 15.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new SniperMedal(0xFA877DCF, 15.0f, 625));
    battleMedals.addMedal(new AmphibiousMedal(0x5BA330E9, 1.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    GameMedal* medal = new FallKillMedal(0x3990407B, 1.0f, 3125);
    MultiConditionMedal* conditionMedal;
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);
    battleMedals.addMedal(new WeaponMedal(0x46558545, 10.0f, 125, (Item*)NULL));
    battleMedals.addMedal(new DamageDealtMedal(0x02F32BA3, 20.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DamageDealtMedal(0x0CF03A49, 10.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    battleMedals.addMedal(new WeaponMedal(0xAAF401B3, 10.0f, 125, 1));
    battleMedals.addMedal(new WeaponMedal(0x9FDE2D4C, 10.0f, 125, Items::bow));

    int weaponIds[] = {10, 8, 7, 9};
    battleMedals.addMedal(new WeaponMedal(0x86DFA3A0, 3.0f, 625, new std::vector<int>(weaponIds, weaponIds + 4)));

    Scoreboard::EDamageSource damageSources[] = {Scoreboard::DAMAGE_SOURCE_LAVA, Scoreboard::DAMAGE_SOURCE_FIRE};
    battleMedals.addMedal(new DamageSourceMedal(
        0x118D7CB1, 10.0f, 625, new std::vector<Scoreboard::EDamageSource>(damageSources, damageSources + 2)));

    battleMedals.addMedal(new DamageSourceMedal(0x9B18B91B, 5.0f, 125, Scoreboard::DAMAGE_SOURCE_DROWN));
    battleMedals.addMedal(new DamageSourceMedal(0xA847BB24, 5.0f, 125, Scoreboard::DAMAGE_SOURCE_FALL));
    battleMedals.addMedal(new DamageSourceMedal(0xEDE3A3FA, 10.0f, 25, Scoreboard::DAMAGE_SOURCE_ARROW));
    battleMedals.addMedal(new TankMedal(0x3F27FDC6, 40.0f, 625));
    battleMedals.addMedal(new FoodMedal(0x69158380, 2.0f, 125, (Item*)NULL));

    Item* fishFoods[] = {Items::cookedFish, Items::fish};
    battleMedals.addMedal(new FoodMedal(0x481026AB, 2.0f, 125, new std::vector<Item*>(fishFoods, fishFoods + 2)));

    battleMedals.addMedal(new PotionMedal(0xE04899EF, 2.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new FoodMedal(0x777DF067, 2.0f, 125, Items::bread));

    Item* meatFoods[] = {Items::cookedBeef, Items::beef,          Items::cookedPorkchop,
                         Items::porkchop,   Items::cookedChicken, Items::chicken};
    battleMedals.addMedal(new FoodMedal(0x410B9DD2, 2.0f, 125, new std::vector<Item*>(meatFoods, meatFoods + 6)));

    battleMedals.addMedal(new HoarderMedal(0xBB274286, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new TreasureHunterMedal(0x483C96F0, 3.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new SplashPotionMedal(0x02C0E252, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(
        new SneakingMedal(IDS_MEDAL_MOST_SNEAKING, 2.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new UnderwaterMedal(0xE268EF55, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DistanceMedal(0x15F4E5E1, 100.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new ClimbingMedal(0xF14B91A1, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DistanceMedal(0x8AF1D99B, 100.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    battleMedals.addMedal(new LifeSpanMedal(IDS_MEDAL_SHORTEST_LIFE, 120.0f, 25, GameMedal::OPERATOR_AT_MOST));
    medal = new ConsolationMedal(IDS_MEDAL_CONSOLATION_1, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);
    medal = new ConsolationMedal(IDS_MEDAL_CONSOLATION_2, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);
    medal = new ConsolationMedal(0x6772BD71, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);
    medal = new ConsolationMedal(0x257F217E, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);
    medal = new ConsolationMedal(0x4B819BE7, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    battleMedals.addMedal(medal);

    MedalAdder tumbleMedals(&nextMedalId, &gTumbleMedals, &gAllMiniGameMedals);

    tumbleMedals.addMedal(new BlockDestroyedMedal(0x2BB44FEE, 0.0f, 125, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new BlockDestroyedMedal(0xDF79BDD4, 100.0f, 125, GameMedal::OPERATOR_AT_MOST));
    tumbleMedals.addMedal(
        new ProjectileDirectHitMedal(0x70D4F647, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(
        new HitsFromProjectileMedal(0xDBB3DF9C, 25.0f, 125, GameMedal::OPERATOR_AT_MOST, true, false));
    medal = new WinnerWithoutBlockDestructionMedal(0x8A2B0AC2, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false);
    medal->SetCanMultiAward(true);
    tumbleMedals.addMedal(medal);
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_FIRST,
                                                                  0x03AD6CD5, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_SECOND,
                                                                  0xDA364E83, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_THIRD,
                                                                  0x2493FAEE, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new JumpsMedal(IDS_MEDAL_MOST_JUMPS, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    medal = new FirstDeathWithoutBlockDestructionMedal(0xB64A8AB1, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true,
                                                       false);
    medal->SetCanMultiAward(true);
    tumbleMedals.addMedal(medal);
    tumbleMedals.addMedal(new LongWayDownMedal(0x57745C04, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeSpentOnLayerMedal(TimeSpentOnLayerMedal::LAYER_MEDAL_TOP, 0x75F9EAFC, 20.0f, 625,
                                                    GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new TimeSpentOnLayerMedal(TimeSpentOnLayerMedal::LAYER_MEDAL_BOTTOM, 0xFDF26520, 20.0f, 625,
                                                    GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new LifeSpanMedal(0x415F73D7, 30.0f, 25, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new LifeSpanMedal(0xD01D07DF, 300.0f, 125, GameMedal::OPERATOR_AT_MOST));
    tumbleMedals.addMedal(
        new BlocksDestroyedFromBelowMedal(0x18F9F129, 5.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    medal = new WinningOnTopLayerMedal(0x8C688A64, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false);
    medal->SetCanMultiAward(true);
    tumbleMedals.addMedal(medal);
    tumbleMedals.addMedal(new MissedSnowballsMedal(0xD5E3FB2E, 20.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new FirstToDieMedal(0xE75F245D, 0.9f, 125));
    tumbleMedals.addMedal(new ImprovementMedal(0x3179D63E, 1.0f, 3125));
    tumbleMedals.addMedal(new FirstToBottomMedal(0xD3DA7ABA, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeInLayerCentreMedal(0xC507906D, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeInLayerCentreMedal(0xCC2971F7, 150.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    tumbleMedals.addMedal(
        new FirstToBottomAndWonMedal(0xBD402445, 1.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    medal = new ConsolationMedal(0xD97A2CAC, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    tumbleMedals.addMedal(medal);
    medal = new ConsolationMedal(0xFF7CA715, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    tumbleMedals.addMedal(medal);
    tumbleMedals.addMedal(new DistanceMedal(0x15F4E5E1, 25.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new DistanceMedal(0x8AF1D99B, 100.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    tumbleMedals.addMedal(
        new SneakingMedal(IDS_MEDAL_MOST_SNEAKING, 2.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));

    MedalAdder glideMedals(&nextMedalId, &gGlideMedals, &gAllMiniGameMedals);

    glideMedals.addMedal((new MultiConditionMedal(IDS_MG03_MEDAL_DEATHS, 25,
                                                  new TotalDeathsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal(
        (new MultiConditionMedal(0x79CA8ED4, 625, new TotalDeathsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
            ->addExtraCondition(new WinnerCondition())
            ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    conditionMedal = (new MultiConditionMedal(IDS_MG03_MEDAL_POINTS, 625,
                                              new TotalPointsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
                         ->addExtraCondition(new VariantCondition(eMiniGameVariantId_ScoreAttack))
                         ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    conditionMedal = new MultiConditionMedal(0xEAA09456, 625, new HasSpawnedAtAllCheckpointsCondition());
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    glideMedals.addMedal(
        (new MultiConditionMedal(IDS_MG03_MEDAL_DAMAGE, 125,
                                 new TotalDamageTakenCondition(10.0f, GameMedal::OPERATOR_AT_MOST, true)))
            ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal(
        (new MultiConditionMedal(IDS_MG03_MEDAL_THERMALS, 25,
                                 new TotalThermalsEnteredCondition(0.0f, GameMedal::OPERATOR_AT_MOST, true)))
            ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(
                              0xC6338D98, 625, new TotalDamageTakenCondition(10.0f, GameMedal::OPERATOR_AT_MOST, true)))
                             ->addExtraCondition(new WinnerCondition())
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    conditionMedal = new MultiConditionMedal(0xE4CE5068, 3125, new FinishCourseWithDamageTakenCondition(0.0f));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    conditionMedal = new MultiConditionMedal(IDS_MG03_MEDAL_1_DAMAGE, 625,
                                             new FinishCourseWithDamageTakenCondition(1.0f));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    conditionMedal = new MultiConditionMedal(IDS_MG03_MEDAL_2_DAMAGE, 625,
                                             new FinishCourseWithDamageTakenCondition(2.0f));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);

    RingsMedal::eRingMedalAmount ringAmounts1[] = {RingsMedal::eRingAmountMost, RingsMedal::eRingAmountMost,
                                                   RingsMedal::eRingAmountAll};
    medal = new RingsMedal(IDS_MG03_MEDAL_RINGS_LARGE, 125, ringAmounts1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    RingsMedal::eRingMedalAmount ringAmounts2[] = {RingsMedal::eRingAmountMost, RingsMedal::eRingAmountAll,
                                                   RingsMedal::eRingAmountMost};
    medal = new RingsMedal(0xCC9401CF, 625, ringAmounts2, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    RingsMedal::eRingMedalAmount ringAmounts3[] = {RingsMedal::eRingAmountAll, RingsMedal::eRingAmountMost,
                                                   RingsMedal::eRingAmountMost};
    medal = new RingsMedal(IDS_MG03_MEDAL_RINGS_SMALL, 3125, ringAmounts3, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    RingsMedal::eRingMedalAmount ringAmounts4[] = {RingsMedal::eRingAmountNone, RingsMedal::eRingAmountNone,
                                                   RingsMedal::eRingAmountSome};
    medal = new RingsMedal(0xDA6B1EBC, 625, ringAmounts4, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    RingsMedal::eRingMedalAmount ringAmounts5[] = {RingsMedal::eRingAmountNone, RingsMedal::eRingAmountNone,
                                                   RingsMedal::eRingAmountNone};
    medal = new RingsMedal(0x1B041C00, 125, ringAmounts5, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    conditionMedal = (new MultiConditionMedal(0x6B80D29C, 625,
                                              new ReachEndTimeCondition(FLT_MAX, true, GameMedal::OPERATOR_AT_MOST)))
                         ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    glideMedals.addMedal(
        (new MultiConditionMedal(0x0932AB28, 125, new TopSpeedCondition(0.0f, GameMedal::OPERATOR_AT_LEAST)))
            ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    conditionMedal =
        (new MultiConditionMedal(0x47708614, 25, new ImprovedCondition(1.0f, GameMedal::OPERATOR_AT_LEAST)))
            ->addExtraCondition(new FinishedLastRoundCondition());
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    glideMedals.addMedal((new MultiConditionMedal(0x6EDA42F4, 25,
                                                  new TimeSpentCondition(1.0f, eTimeSpentCategory_BetweenCheckpoints,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xC9E72AE0, 25,
                                                  new TimeSpentCondition(1000.0f, eTimeSpentCategory_GoingBackwards,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(IDS_MG03_MEDAL_WATER, 25,
                                                  new TimeSpentCondition(1.0f, eTimeSpentCategory_InWater,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xC8D305C9, 25,
                                                  new TimeSpentCondition(1.0f, eTimeSpentCategory_NearBlocks,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0x89F55709, 3125, new LastInFirstRoundCondition()))
                             ->addExtraCondition(new WinnerCondition())
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    conditionMedal = (new MultiConditionMedal(0x18A608DA, 625, new FastestTimeAndHightestScoreInARoundCondition()))
                         ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
                         ->addExtraCondition(new VariantCondition(eMiniGameVariantId_ScoreAttack));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    conditionMedal = (new MultiConditionMedal(IDS_MG03_MEDAL_CLOSE, 3125,
                                              new WonByAmountCondition(1000.0f, GameMedal::OPERATOR_AT_MOST)))
                         ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    conditionMedal =
        (new MultiConditionMedal(0xC9D3B088, 625, new WonByAmountCondition(10000.0f, GameMedal::OPERATOR_AT_LEAST)))
            ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST));
    conditionMedal->SetCanMultiAward(true);
    glideMedals.addMedal(conditionMedal);
    glideMedals.addMedal((new MultiConditionMedal(0x13F33157, 125,
                                                  new TimeSpentCondition(1000.0f, eTimeSpentCategory_GoingBackwards,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
                             ->addExtraCondition(new WinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0xADD38DE6, 125,
                                                  new TimeSpentCondition(15.0f, eTimeSpentCategory_Leading,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
                             ->addExtraCondition(new NotWinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0xAB7835CB, 625,
                                                  new TimeSpentCondition(15.0f, eTimeSpentCategory_Trailing,
                                                                         GameMedal::OPERATOR_AT_LEAST, true)))
                             ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
                             ->addExtraCondition(new WinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0x8FDDCE64, 25, new NotFastestTimeButWonAnywayCondition()))
                             ->addExtraCondition(new VariantCondition(eMiniGameVariantId_ScoreAttack)));
    medal = new ConsolationMedal(IDS_MG03_MEDAL_CONSOLATION_1, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);
    medal = new ConsolationMedal(IDS_MG03_MEDAL_CONSOLATION_2, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);
    medal = new ConsolationMedal(0x91C693F7, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);
    medal = new ConsolationMedal(0x87B53B18, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST);
    medal->SetCanMultiAward(true);
    glideMedals.addMedal(medal);

    MedalAdder buildMedals(&nextMedalId, &gBuildMedals, &gAllMiniGameMedals);

    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 20.0f, 125, BlockPlacedOverTime::START_DELAY,
                                                 GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 10.0f, 125, BlockPlacedOverTime::IN_FIRST_10SECS,
                                                 GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 20.0f, 125, BlockPlacedOverTime::TIME_LEFTOVER,
                                                 GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 10.0f, 125, BlockPlacedOverTime::IN_LAST_10SECS,
                                                 GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedMedal(0x777DF067, 10.0f, 625, GameMedal::OPERATOR_AT_MOST));
    buildMedals.addMedal(new BlockPlacedMedal(0x777DF067, 100.0f, 125, GameMedal::OPERATOR_AT_LEAST));

    gBattleMedalCount = gBattleMedals.size();
    gTumbleMedalCount = gTumbleMedals.size();
    gGlideMedalCount = gGlideMedals.size();
    gBuildMedalCount = gBuildMedals.size();
}

const GameMedal* MiniGameMedals::getMedalById(int medalId)
{
    if (medalId >= 0 && (unsigned int)medalId < gAllMiniGameMedals.size())
    {
        return gAllMiniGameMedals.at(medalId);
    }

    return NULL;
}
