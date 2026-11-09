#include "game/minigame/FullTutorial.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/AreaConstraint.hpp"
#include "game/core/AreaHint.hpp"
#include "game/core/AreaTask.hpp"
#include "game/core/ChangeStateConstraint.hpp"
#include "game/core/ChoiceTask.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ControllerTask.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameType.hpp"
#include "game/core/InfoTask.hpp"
#include "game/core/PickupTask.hpp"
#include "game/core/ProcedureCompoundTask.hpp"
#include "game/core/ProgressFlagTask.hpp"
#include "game/core/StateChangeTask.hpp"
#include "game/item/CompleteUsingItemTask.hpp"
#include "game/item/CraftTask.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/RepairItemTask.hpp"
#include "game/item/EquipItemTask.hpp"
#include "game/item/EffectChangedTask.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/item/UseItemTask.hpp"
#include "game/minigame/FullTutorialActiveTask.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/UICraftingTask.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/UseBlockTask.hpp"
#include "game/util/Vec3.hpp"

namespace TutorialIds
{
static const unsigned int AdditionalPlanksCraft = IDS_TUTORIAL_TASK_CRAFT_CREATE_PLANKS;
static const unsigned int AnvilChoice = IDS_TUTORIAL_TASK_ANVIL_OVERVIEW;
static const unsigned int AnvilChoicePrompt = IDS_TUTORIAL_PROMPT_ANVIL_OVERVIEW;
static const unsigned int AnvilInfo1 = IDS_TUTORIAL_TASK_ANVIL_SUMMARY;
static const unsigned int AnvilInfo2 = IDS_TUTORIAL_TASK_ANVIL_ENCHANTED_BOOKS;
static const unsigned int AnvilInfo3 = IDS_TUTORIAL_TASK_ANVIL_COST;
static const unsigned int AnvilInfo4 = 0x8d2afeff;
static const unsigned int AnvilInfo5 = IDS_TUTORIAL_TASK_ANVIL_RENAMING;
static const unsigned int AnvilInfo6 = 0x5f44dd70;
static const unsigned int AxeSetCraft = 0xfac53756;
static const unsigned int BannerChoice = IDS_TUTORIAL_TASK_BANNER_OVERVIEW;
static const unsigned int BannerChoicePrompt = IDS_TUTORIAL_PROMPT_BANNER_OVERVIEW;
static const unsigned int BannerInfo1 = IDS_TUTORIAL_TASK_BANNER_CRAFTING;
static const unsigned int BannerInfo2 = 0x7893e960;
static const unsigned int BannerInfo3 = IDS_TUTORIAL_TASK_BANNER_PATTERNS_1;
static const unsigned int BannerInfo4 = 0x3f94c9d;
static const unsigned int BannerInfo5 = IDS_TUTORIAL_TASK_BANNER_PATTERNS_2;
static const unsigned int BannerInfo6 = 0x82eeddb;
static const unsigned int BeaconChoice = IDS_TUTORIAL_TASK_BEACON_OVERVIEW;
static const unsigned int BeaconChoicePrompt = IDS_TUTORIAL_PROMPT_BEACON_OVERVIEW;
static const unsigned int BeaconInfo1 = 0x702925f0;
static const unsigned int BeaconInfo2 = 0x68035ac;
static const unsigned int BeaconInfo3 = 0x54620c0d;
static const unsigned int BeaconInfo4 = 0xd4e2892d;
static const unsigned int BoatArea = IDS_TUTORIAL_HINT_BOAT;
static const unsigned int BreedingChoice = IDS_TUTORIAL_BREEDING_OVERVIEW;
static const unsigned int BreedingChoicePrompt = IDS_TUTORIAL_PROMPT_BREEDING_OVERVIEW;
static const unsigned int BreedingInfo1 = IDS_TUTORIAL_TASK_BREEDING_FEED;
static const unsigned int BreedingInfo10 = IDS_TUTORIAL_TASK_BREEDING_COMPLETE;
static const unsigned int BreedingInfo2 = 0x2602e491;
static const unsigned int BreedingInfo3 = 0xa6bf646e;
static const unsigned int BreedingInfo4 = 0x4cd6cb;
static const unsigned int BreedingInfo5 = IDS_TUTORIAL_TASK_BREEDING_FOLLOW;
static const unsigned int BreedingInfo6 = 0xc49c0d63;
static const unsigned int BreedingInfo7 = 0x12a10fb;
static const unsigned int BreedingInfo8 = IDS_TUTORIAL_TASK_BREEDING_WOLF_COLLAR;
static const unsigned int BreedingInfo9 = 0xb6fadd57;
static const unsigned int BrewingBottlePickup = 0xfdd3b930;
static const unsigned int BrewingChoice = IDS_TUTORIAL_TASK_BREWING_OVERVIEW;
static const unsigned int BrewingChoicePrompt = 0xada8412;
static const unsigned int BrewingFireResistanceEffect = 0xf7c6caa1;
static const unsigned int BrewingFireResistancePickup = 0xa57e40af;
static const unsigned int BrewingInfo1 = 0x43a035a4;
static const unsigned int BrewingInfo2 = IDS_TUTORIAL_TASK_BREWING_USE_POTION;
static const unsigned int BrewingInfo3 = 0x7c858498;
static const unsigned int BrewingPotionPickup = 0x64e7f921;
static const unsigned int ChallengeIntroInfo = 0x3f434d73;
static const unsigned int CoalCraft = IDS_TUTORIAL_TASK_CREATE_CHARCOAL;
static const unsigned int CoalCraftCreative = 0xb9ad8818;
static const unsigned int CobblestonePickup = IDS_TUTORIAL_TASK_MINE_STONE;
static const unsigned int ConduitChoice = IDS_TUTORIAL_TASK_CONDUIT_OVERVIEW;
static const unsigned int ConduitChoicePrompt = IDS_TUTORIAL_PROMPT_CONDUIT_OVERVIEW;
static const unsigned int ConduitInfo1 = 0x4a786472;
static const unsigned int ConduitInfo2 = IDS_TUTORIAL_TASK_CONDUIT_CRAFTING;
static const unsigned int ConduitInfo3 = IDS_TUTORIAL_TASK_CONDUIT_UNDERWATER;
static const unsigned int ConduitInfo4 = IDS_TUTORIAL_TASK_CONDUIT_FRAME;
static const unsigned int ConduitInfo5 = IDS_TUTORIAL_TASK_CONDUIT_PLACE;
static const unsigned int ControllerAction = IDS_TUTORIAL_TASK_JUMP;
static const unsigned int ControllerActionSecondary = IDS_TUTORIAL_TASK_MINE;
static const unsigned int ControllerInventory = IDS_TUTORIAL_TASK_CRAFTING;
static const unsigned int ControllerJump = IDS_TUTORIAL_TASK_INVENTORY;
static const unsigned int ControllerLook = 0xd50ac0c8;
static const unsigned int ControllerLookMove = IDS_TUTORIAL_TASK_MOVE;
static const unsigned int ControllerMove = IDS_TUTORIAL_TASK_LOOK;
static const unsigned int CraftingBasicsInfo = IDS_TUTORIAL_TASK_USE;
static const unsigned int CraftingChoice = 0x28c71b69;
static const unsigned int CraftingChoiceFollowup = 0x791088b0;
static const unsigned int CraftingInfo = 0x59983743;
static const unsigned int CraftingInfoAdvanced = IDS_TUTORIAL_TASK_NEARBY_SHELTER;
static const unsigned int CraftingTableCraft = 0x2e35e925;
static const unsigned int CraftingTableInfo = IDS_TUTORIAL_TASK_COLLECT_RESOURCES;
static const unsigned int CraftingTableInfoAdvanced = 0xacd522e4;
static const unsigned int CraftingTableUi = 0xc8a27304;
static const unsigned int CraftingTableUse = 0x7de96d85;
static const unsigned int CraftingTableUseInfo = 0x88ac7bfd;
static const unsigned int CraftingTableUseItem = 0xe42098c2;
static const unsigned int CreativeController = IDS_TUTORIAL_TASK_OPEN_CREATIVE_INVENTORY;
static const unsigned int CreativeExitArea = IDS_TUTORIAL_TASK_CREATIVE_EXIT;
static const unsigned int CreativeExitInfo = IDS_TUTORIAL_TASK_CREATIVE_COMPLETE;
static const unsigned int CreativeInfo = IDS_TUTORIAL_TASK_FLY;
static const unsigned int CreativeInventoryInfo = IDS_TUTORIAL_TASK_CREATIVE_MODE;
static const unsigned int CreativeModeChoice = IDS_TUTORIAL_CREATIVE_OVERVIEW;
static const unsigned int CreativeModeChoiceFollowup = 0xd1ebba23;
static const unsigned int DefaultMessageGroup = 0x3f9463f;
static const unsigned int DoorSetCraft = 0x74135c27;
static const unsigned int DoorUse = IDS_TUTORIAL_TASK_PLACE_DOOR;
static const unsigned int ElytraArea = 0x6fcb60c3;
static const unsigned int ElytraEquip = 0x8fcc51e8;
static const unsigned int ElytraRepair = 0x1ceb28a5;
static const unsigned int ElytraChoice = 0xd741e933;
static const unsigned int ElytraChoicePrompt = 0xb906b2f2;
static const unsigned int ElytraInfo1 = 0x8992fd0c;
static const unsigned int ElytraInfo2 = 0x58852eda;
static const unsigned int ElytraInfo3 = 0xdc0fe933;
static const unsigned int ElytraSecondChoice = 0x7f2abff;
static const unsigned int ElytraSecondChoicePrompt = 0x48cf39c6;
static const unsigned int EnchantingChoice = IDS_TUTORIAL_TASK_ENCHANTING_OVERVIEW;
static const unsigned int EnchantingChoicePrompt = IDS_TUTORIAL_PROMPT_ENCHANTING_OVERVIEW;
static const unsigned int EnchantingInfo1 = IDS_TUTORIAL_TASK_ENCHANTING_SUMMARY;
static const unsigned int EnchantingInfo2 = IDS_TUTORIAL_TASK_ENCHANTING_BOOKS;
static const unsigned int EnchantingInfo3 = 0xa3d895f;
static const unsigned int EnchantingInfo4 = IDS_TUTORIAL_TASK_ENCHANTING_EXPERIENCE;
static const unsigned int EnchantingInfo5 = 0xc52e1;
static const unsigned int EnchantingInfo6 = 0xfcd038df;
static const unsigned int EnderChestChoice = 0xe6bf898;
static const unsigned int EnderChestChoicePrompt = 0x372b359;
static const unsigned int EnderChestInfo1 = 0x393eb8ef;
static const unsigned int EnderChestInfo2 = 0xcb555ab;
static const unsigned int EnderChestInfo3 = 0x2b2d9551;
static const unsigned int FarmingChoice = IDS_TUTORIAL_FARMING_OVERVIEW;
static const unsigned int FarmingChoicePrompt = IDS_TUTORIAL_PROMPT_FARMING_OVERVIEW;
static const unsigned int FarmingInfo1 = 0xcd48e76;
static const unsigned int FarmingInfo10 = IDS_TUTORIAL_TASK_FARMING_COMPLETE;
static const unsigned int FarmingInfo2 = 0xb101b8d;
static const unsigned int FarmingInfo3 = IDS_TUTORIAL_TASK_FARMING_WHEAT;
static const unsigned int FarmingInfo4 = 0xc3230db0;
static const unsigned int FarmingInfo5 = 0x773032a4;
static const unsigned int FarmingInfo6 = 0x5944041;
static const unsigned int FarmingInfo7 = 0x361de5e9;
static const unsigned int FarmingInfo8 = IDS_TUTORIAL_TASK_FARMING_MUSHROOM;
static const unsigned int FarmingInfo9 = IDS_TUTORIAL_TASK_FARMING_BONEMEAL;
static const unsigned int FireworksChoice = 0x432ea6f8;
static const unsigned int FireworksChoicePrompt = 0xd357b889;
static const unsigned int FireworksInfo1 = 0xba3e693d;
static const unsigned int FireworksInfo2 = 0xe6996247;
static const unsigned int FireworksInfo3 = IDS_TUTORIAL_TASK_FIREWORK_CRAFTING;
static const unsigned int FishingArea = IDS_TUTORIAL_HINT_FISHING;
static const unsigned int FoodUse = 0x355eb7d9;
static const unsigned int FurnaceCraft = 0xe7cad766;
static const unsigned int FurnaceCraftCreative = IDS_TUTORIAL_TASK_CRAFT_CREATE_FURNACE;
static const unsigned int FurnaceUse = 0xf1d4d67e;
static const unsigned int GlassCraft = IDS_TUTORIAL_TASK_CREATE_GLASS;
static const unsigned int GlassCraftCreative = 0xa9ca4693;
static const unsigned int GolemChoice = IDS_TUTORIAL_GOLEM_OVERVIEW;
static const unsigned int GolemChoicePrompt = IDS_TUTORIAL_PROMPT_GOLEM_OVERVIEW;
static const unsigned int GolemInfo1 = IDS_TUTORIAL_TASK_GOLEM_PUMPKIN;
static const unsigned int GolemInfo2 = IDS_TUTORIAL_TASK_GOLEM_SNOW;
static const unsigned int GolemInfo3 = IDS_TUTORIAL_TASK_GOLEM_IRON;
static const unsigned int GolemInfo4 = IDS_TUTORIAL_TASK_GOLEM_IRON_VILLAGE;
static const unsigned int HopperChoice = IDS_TUTORIAL_TASK_HOPPER_OVERVIEW;
static const unsigned int HopperChoicePrompt = IDS_TUTORIAL_PROMPT_HOPPER_OVERVIEW;
static const unsigned int HopperInfo1 = 0x31bcd75c;
static const unsigned int HopperInfo2 = 0x985f1d96;
static const unsigned int HopperInfo3 = 0xc398648f;
static const unsigned int HopperInfo4 = IDS_TUTORIAL_TASK_HOPPER_REDSTONE;
static const unsigned int HopperInfo5 = IDS_TUTORIAL_TASK_HOPPER_OUTPUT;
static const unsigned int HopperInfo6 = IDS_TUTORIAL_TASK_HOPPER_AREA;
static const unsigned int InventoryInfo = 0x642c1bb4;
static const unsigned int LogPickup = IDS_TUTORIAL_TASK_CHOP_WOOD;
static const unsigned int MinecartArea = IDS_TUTORIAL_HINT_MINECART;
static const unsigned int MovementForwardInfo = 0xd756c6a6;
static const unsigned int MovementInfo = IDS_TUTORIAL_TASK_SPRINT;
static const unsigned int MovementLookInfo = IDS_TUTORIAL_TASK_FOOD_BAR_DEPLETE;
static const unsigned int PickaxeSetCraft = 0x19cdd9a4;
static const unsigned int PistonBridgeArea = 0xf258c0ad;
static const unsigned int PistonChoice = IDS_TUTORIAL_REDSTONE_OVERVIEW;
static const unsigned int PistonChoicePrompt = IDS_TUTORIAL_PROMPT_REDSTONE_OVERVIEW;
static const unsigned int PistonInfo1 = 0xc1453d8c;
static const unsigned int PistonInfo2 = 0xe197530c;
static const unsigned int PistonInfo3 = 0x2aa68e1a;
static const unsigned int PistonInfo4 = IDS_TUTORIAL_TASK_REDSTONE_DUST;
static const unsigned int PistonInfo5 = 0x8a39641a;
static const unsigned int PistonInfo6 = IDS_TUTORIAL_TASK_PISTONS;
static const unsigned int PistonInfo7 = 0x1e3960b0;
static const unsigned int PlanksCraft = 0x4d449d7d;
static const unsigned int PortalChoice = IDS_TUTORIAL_PORTAL_OVERVIEW;
static const unsigned int PortalChoicePrompt = IDS_TUTORIAL_PROMPT_PORTAL_OVERVIEW;
static const unsigned int PortalInfo1 = 0xbdf81a00;
static const unsigned int PortalInfo2 = IDS_TUTORIAL_TASK_ACTIVATE_PORTAL;
static const unsigned int PortalInfo3 = IDS_TUTORIAL_TASK_USE_PORTAL;
static const unsigned int PortalInfo4 = IDS_TUTORIAL_TASK_NETHER;
static const unsigned int PortalInfo5 = IDS_TUTORIAL_TASK_NETHER_FAST_TRAVEL;
static const unsigned int ShovelSetCraft = 0xf9cc6772;
static const unsigned int SlimeChoice = IDS_TUTORIAL_TASK_SLIME_OVERVIEW;
static const unsigned int SlimeChoicePrompt = IDS_TUTORIAL_PROMPT_SLIME_OVERVIEW;
static const unsigned int SlimeInfo1 = 0xd56bb829;
static const unsigned int SlimeInfo2 = 0x853766e0;
static const unsigned int SmeltingBasicsInfo = 0x5fce503f;
static const unsigned int SmeltingInfo = 0x4c729fc4;
static const unsigned int StickCraft = 0x56bee93b;
static const unsigned int StructuresUi = IDS_TUTORIAL_TASK_CRAFT_SELECT_STRUCTURES;
static const unsigned int SurvivalBasicsInfo = 0x12eeee51;
static const unsigned int ToolBasicsInfo = 0x63c8c01a;
static const unsigned int ToolsUi = IDS_TUTORIAL_TASK_CRAFT_SELECT_TOOLS;
static const unsigned int TorchCraft = 0xd15ac;
static const unsigned int TradingChoice = IDS_TUTORIAL_TASK_TRADING_OVERVIEW;
static const unsigned int TradingChoicePrompt = IDS_TUTORIAL_PROMPT_TRADING_OVERVIEW;
static const unsigned int TradingInfo1 = IDS_TUTORIAL_TASK_TRADING_SUMMARY;
static const unsigned int TradingInfo2 = IDS_TUTORIAL_TASK_TRADING_TRADES;
static const unsigned int TradingInfo3 = 0x2c97e467;
static const unsigned int TradingInfo4 = 0x1886cb59;
static const unsigned int TradingInfo5 = 0x239bee27;
static const unsigned int TridentChoice = IDS_TUTORIAL_TASK_TRIDENT_OVERVIEW;
static const unsigned int TridentChoicePrompt = IDS_TUTORIAL_PROMPT_TRIDENT_OVERVIEW;
static const unsigned int TridentInfo1 = 0x75f41b48;
static const unsigned int TridentInfo2 = IDS_TUTORIAL_TASK_TRIDENT_THROWING;
static const unsigned int TridentInfo3 = 0x270d5c69;
static const unsigned int TridentInfo4 = 0x31524f6d;
static const unsigned int TridentInfo5 = IDS_TUTORIAL_TASK_TRIDENT_RIPTIDE;
static const unsigned int TridentInfo6 = 0x1d32cd30;
static const unsigned int TutorialArea = 0x53717f71;
static const unsigned int TutorialStartChoice = 0xdf59fc3f;
static const unsigned int TutorialStartTask = IDS_TUTORIAL_TASK_OVERVIEW;
static const unsigned int WelcomeMessage = IDS_TUTORIAL_COMPLETED;
static const unsigned int WoodenShovelUi = 0x76c32bbb;
}

FullTutorial::FullTutorial(int playerId, GameModeConstraints* constraints, bool enabled)
    : Tutorial(playerId, constraints, true, false),
      mEnabledForPlayer(enabled)
{
    overridesGameHostOptions = true;
    mProgressFlags = 0;
    for (int i = 0; i < 44; ++i)
    {
        mStateCompleted[i] = 0;
    }
    addMessage(TutorialIds::WelcomeMessage, false, 3);

    if (mEnabledForPlayer || getCompleted(0x183) != 0)
    {
        addTask(eTutorial_State_Gameplay,
                new ChoiceTask(this, TutorialIds::TutorialStartTask, TutorialIds::TutorialStartChoice, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateExceptLastTask,
                               eTelemetryChallenges_TutorialStart));
    }
    else
    {
        addTask(eTutorial_State_Gameplay,
                new InfoTask(this, TutorialIds::TutorialStartTask, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
    }

    int controllerMappingsA[4] = {30, 31, 28, 29};
    int controllerMappingsB[4] = {24, 25, 26, 27};
    int controllerCompletionIdsA[4] = {10, 9, 6, 5};

    addTask(eTutorial_State_Gameplay,
            new ControllerTask(this, TutorialIds::ControllerMove, false, false, controllerMappingsA, 4,
                               controllerCompletionIdsA, 4, controllerMappingsB, 4));
    addTask(eTutorial_State_Gameplay,
            new ControllerTask(this, TutorialIds::ControllerLookMove, false, false, controllerMappingsB, 4,
                               controllerCompletionIdsA, 4, controllerMappingsA, 4));
    addTask(eTutorial_State_Gameplay, new InfoTask(this, TutorialIds::MovementInfo, TutorialIds::DefaultMessageGroup,
                                                   true, 0, eTelemetryChallenges_None));

    int mappingSingle32[1] = {32};
    addTask(eTutorial_State_Gameplay,
            new ControllerTask(this, TutorialIds::ControllerAction, false, true, mappingSingle32, 1, NULL, 0, NULL, 0));
    int mappingSingle34[1] = {34};
    addTask(eTutorial_State_Gameplay, new ControllerTask(this, TutorialIds::ControllerActionSecondary, false, true,
                                                         mappingSingle34, 1, NULL, 0, NULL, 0));

    Item* logPickupItems[8] = {Item::byBlock(Blocks::log),
                               Item::byBlock(Blocks::log2),
                               Item::byBlock(Blocks::stripped_log_dark_oak),
                               Item::byBlock(Blocks::stripped_log_acacia),
                               Item::byBlock(Blocks::stripped_log_birch),
                               Item::byBlock(Blocks::stripped_log_jungle),
                               Item::byBlock(Blocks::stripped_log_oak),
                               Item::byBlock(Blocks::stripped_log_spruce)};
    addTask(eTutorial_State_Gameplay, new PickupTask(logPickupItems, 8, 4, -1, this, TutorialIds::LogPickup, true, NULL,
                                                     false, true, true, NULL));

    int mappingsPair1[2] = {35, 36};
    int completionPair1[2] = {2, 1};
    addTask(eTutorial_State_Gameplay, new ControllerTask(this, TutorialIds::ControllerLook, false, false, mappingsPair1,
                                                         2, completionPair1, 2, NULL, 0));
    int mappingSingle37[1] = {37};
    addTask(eTutorial_State_Gameplay,
            new ControllerTask(this, TutorialIds::ControllerJump, false, false, mappingSingle37, 1, NULL, 0, NULL, 0));

    addTask(eTutorial_State_Gameplay,
            new StateChangeTask(eTutorial_State_InventoryMenu, this, -1, false, NULL, false, true, true));

    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::MovementLookInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::MovementForwardInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay, new InfoTask(this, TutorialIds::InventoryInfo, TutorialIds::DefaultMessageGroup,
                                                   true, 0, eTelemetryChallenges_None));

    Item* completeUsingFoodItems[15] = {
        Items::mushroomStew, Items::apple,         Items::bread,      Items::porkchop, Items::cookedPorkchop,
        Items::goldenApple,  Items::fish,          Items::cookedFish, Items::cookie,   Items::cookedBeef,
        Items::beef,         Items::cookedChicken, Items::chicken,    Items::melon,    Items::rottenFlesh};
    addTask(eTutorial_State_Gameplay,
            new CompleteUsingItemTask(this, TutorialIds::FoodUse, completeUsingFoodItems, 15, true));

    int mappingSingle40[1] = {40};
    addTask(eTutorial_State_Gameplay, new ControllerTask(this, TutorialIds::ControllerInventory, false, false,
                                                         mappingSingle40, 1, NULL, 0, NULL, 0));

    addTask(eTutorial_State_Gameplay,
            new ProgressFlagTask(this, &mProgressFlags, 1, ProgressFlagTask::eProgressFlagAction_Set));

    addTask(eTutorial_State_Gameplay, new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this,
                                                    TutorialIds::PlanksCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_Gameplay, new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this,
                                                    TutorialIds::CraftingTableCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::CraftingBasicsInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay,
            new UseItemTask(Item::byBlock(Blocks::crafting_table), this, TutorialIds::CraftingTableUseItem, true, NULL,
                            false, true, true));
    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::CraftingTableUseInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::CraftingInfoAdvanced, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay,
            new InfoTask(this, TutorialIds::CraftingTableInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay,
            new ChoiceTask(this, TutorialIds::CraftingChoice, TutorialIds::CraftingChoiceFollowup, true, 0, 1,
                           eTutorial_CompletionAction_CompleteStateExceptLastTask,
                           eTelemetryChallenges_CraftingChoice));
    addTask(eTutorial_State_Gameplay, new UseBlockTask(Blocks::crafting_table, this, TutorialIds::CraftingTableUse,
                                                       false, NULL, false, true, true));

    addTask(eTutorial_State_Gameplay,
            new ProgressFlagTask(this, &mProgressFlags, 2, ProgressFlagTask::eProgressFlagAction_Set));

    addTask(eTutorial_State_Gameplay,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));

    Item* shovelSetItems[5] = {Items::woodenShovel, Items::stoneShovel, Items::ironShovel, Items::goldenHorseArmor,
                               Items::diamondShovel};
    int shovelSetAux[5] = {-1, -1, -1, -1, -1};
    addTask(eTutorial_State_Gameplay, new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this,
                                                    TutorialIds::ShovelSetCraft, true, NULL, false, true, true));

    Item* axeSetItems[5] = {Items::woodenAxe, Items::stoneAxe, Items::ironAxe, Items::goldenAxe, Items::diamondAxe};
    int axeSetAux[5] = {-1, -1, -1, -1, -1};
    addTask(eTutorial_State_Gameplay,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true, true));

    Item* pickaxeSetItems[5] = {Items::woodenPickaxe, Items::stonePickaxe, Items::ironPickaxe, Items::goldenPickaxe,
                                Items::diamondPickaxe};
    int pickaxeSetAux[5] = {-1, -1, -1, -1, -1};
    addTask(eTutorial_State_Gameplay, new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this,
                                                    TutorialIds::PickaxeSetCraft, true, NULL, false, true, true));

    Item* cobblestonePickupItems[1] = {Item::byBlock(Blocks::cobblestone)};
    addTask(eTutorial_State_Gameplay,
            new PickupTask(cobblestonePickupItems, 1, 8, -1, this, TutorialIds::CobblestonePickup, true, NULL, false,
                           true, true, NULL));

    addTask(eTutorial_State_Gameplay,
            new ProgressFlagTask(this, &mProgressFlags, 4, ProgressFlagTask::eProgressFlagAction_Set));

    addTask(eTutorial_State_Gameplay, new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this,
                                                    TutorialIds::FurnaceCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_Gameplay,
            new UseBlockTask(Blocks::furnace, this, TutorialIds::FurnaceUse, false, NULL, false, true, true));

    addTask(eTutorial_State_Gameplay,
            new ProgressFlagTask(this, &mProgressFlags, 8, ProgressFlagTask::eProgressFlagAction_Set));

    addTask(eTutorial_State_Gameplay,
            new StateChangeTask(eTutorial_State_FurnaceMenu, this, -1, false, NULL, false, true, true));

    addTask(eTutorial_State_Gameplay,
            new CraftTask(Items::coal, -1, 1, this, TutorialIds::CoalCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_Gameplay, new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this, TutorialIds::GlassCraft,
                                                    true, NULL, false, true, true));

    Item* doorSetItems[6] = {Items::woodenDoor,  Items::acaciaDoor, Items::birchDoor,
                             Items::darkOakDoor, Items::jungleDoor, Items::spruceDoor};
    int doorSetAux[6] = {-1, -1, -1, -1, -1, -1};
    addTask(eTutorial_State_Gameplay, new CraftTask(doorSetItems, doorSetAux, 6, 1, this, TutorialIds::DoorSetCraft,
                                                    true, NULL, false, true, true));
    addTask(eTutorial_State_Gameplay,
            new UseItemTask(doorSetItems, 6, this, TutorialIds::DoorUse, false, NULL, false, true, true));

    addTask(eTutorial_State_Gameplay, new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, TutorialIds::TorchCraft,
                                                    true, NULL, false, true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaTutorial = gApp.getGameRuleDefinitions()->getNamedArea(L"tutorialArea");
        if (areaTutorial != NULL)
        {
            std::vector<TutorialConstraint*>* tutorialAreaConstraints = new std::vector<TutorialConstraint*>();
            tutorialAreaConstraints->push_back(new AreaConstraint(
                TutorialIds::TutorialArea, areaTutorial->mMinX, areaTutorial->mMinY, areaTutorial->mMinZ,
                areaTutorial->mMaxX, areaTutorial->mMaxY, areaTutorial->mMaxZ, true, true, false, 2));
            addTask(eTutorial_State_Gameplay, new AreaTask(eTutorial_State_Gameplay, this, tutorialAreaConstraints, -1,
                                                           AreaTask::CompletionStateOnStateChange));
        }
    }

    ProcedureCompoundTask* compound0 = new ProcedureCompoundTask(this);
    compound0->AddTask(new InfoTask(this, TutorialIds::WelcomeMessage, TutorialIds::DefaultMessageGroup, true, 0,
                                    eTelemetryChallenges_TutorialWelcome));
    compound0->AddTask(new InfoTask(this, TutorialIds::CraftingInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                    eTelemetryChallenges_None));
    compound0->AddTask(new InfoTask(this, TutorialIds::CraftingTableInfoAdvanced, TutorialIds::DefaultMessageGroup,
                                    true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Gameplay, compound0);

    addTask(eTutorial_State_InventoryMenu, new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_InventoryMenu,
            new InfoTask(this, TutorialIds::ChallengeIntroInfo, -1, false, 1, eTelemetryChallenges_None));

    addTask(eTutorial_State_CraftingMenu, new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_CraftingMenu,
            new ProgressFlagTask(this, &mProgressFlags, 1, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_CraftingMenu, new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_CraftingMenu,
            new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this, TutorialIds::AdditionalPlanksCraft, true, NULL,
                          false, true, true));

    ProcedureCompoundTask* compound1 = new ProcedureCompoundTask(this);
    compound1->AddTask(new UICraftingTask(this, TutorialIds::StructuresUi, Recipe::eGroupType_Structures, false, NULL,
                                          false, true, true));
    compound1->AddTask(new UICraftingTask(this, TutorialIds::CraftingTableUi, Item::byBlock(Blocks::crafting_table),
                                          false, NULL, false, true, true));
    compound1->AddTask(new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this,
                                     TutorialIds::CraftingTableCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_CraftingMenu, compound1);

    addTask(eTutorial_State_CraftingMenu,
            new InfoTask(this, TutorialIds::SurvivalBasicsInfo, -1, false, 1, eTelemetryChallenges_None));

    addTask(eTutorial_State_CraftingTableMenu,
            new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_CraftingTableMenu,
            new ProgressFlagTask(this, &mProgressFlags, 2, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_CraftingTableMenu,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));

    ProcedureCompoundTask* compound2 = new ProcedureCompoundTask(this);
    compound2->AddTask(
        new UICraftingTask(this, TutorialIds::ToolsUi, Recipe::eGroupType_Tools, false, NULL, false, true, true));
    compound2->AddTask(
        new UICraftingTask(this, TutorialIds::WoodenShovelUi, Items::woodenShovel, false, NULL, false, true, true));
    compound2->AddTask(new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, TutorialIds::ShovelSetCraft, true, NULL,
                                     false, true, true));
    addTask(eTutorial_State_CraftingTableMenu, compound2);

    addTask(eTutorial_State_CraftingTableMenu,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_CraftingTableMenu,
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, TutorialIds::PickaxeSetCraft, true, NULL, false,
                          true, true));
    addTask(eTutorial_State_CraftingTableMenu,
            new InfoTask(this, TutorialIds::ToolBasicsInfo, -1, false, 1, eTelemetryChallenges_None));
    addTask(eTutorial_State_CraftingTableMenu,
            new ProgressFlagTask(this, &mProgressFlags, 4, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_CraftingTableMenu,
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, TutorialIds::FurnaceCraftCreative, true, NULL,
                          false, true, true));
    addTask(eTutorial_State_CraftingTableMenu,
            new InfoTask(this, TutorialIds::SmeltingBasicsInfo, -1, false, 1, eTelemetryChallenges_None));
    addTask(eTutorial_State_CraftingTableMenu, new CraftTask(doorSetItems, doorSetAux, 6, 1, this,
                                                             TutorialIds::DoorSetCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_CraftingTableMenu, new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this,
                                                             TutorialIds::TorchCraft, true, NULL, false, true, true));

    addTask(eTutorial_State_SurvivalCrafting,
            new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_SurvivalCrafting,
            new ProgressFlagTask(this, &mProgressFlags, 2, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_SurvivalCrafting,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_SurvivalCrafting,
            new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, TutorialIds::ShovelSetCraft, true, NULL, false,
                          true, true));
    addTask(eTutorial_State_SurvivalCrafting,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_SurvivalCrafting,
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, TutorialIds::PickaxeSetCraft, true, NULL, false,
                          true, true));
    addTask(eTutorial_State_SurvivalCrafting,
            new InfoTask(this, TutorialIds::ToolBasicsInfo, -1, false, 1, eTelemetryChallenges_None));
    addTask(eTutorial_State_SurvivalCrafting,
            new ProgressFlagTask(this, &mProgressFlags, 4, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_SurvivalCrafting,
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, TutorialIds::FurnaceCraftCreative, true, NULL,
                          false, true, true));
    addTask(eTutorial_State_SurvivalCrafting,
            new InfoTask(this, TutorialIds::SmeltingBasicsInfo, -1, false, 1, eTelemetryChallenges_None));
    addTask(eTutorial_State_SurvivalCrafting, new CraftTask(doorSetItems, doorSetAux, 6, 1, this,
                                                            TutorialIds::DoorSetCraft, true, NULL, false, true, true));
    addTask(eTutorial_State_SurvivalCrafting, new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this,
                                                            TutorialIds::TorchCraft, true, NULL, false, true, true));

    addTask(eTutorial_State_FurnaceMenu, new FullTutorialActiveTask(this, eTutorial_CompletionAction_CompleteState));
    addTask(eTutorial_State_FurnaceMenu,
            new ProgressFlagTask(this, &mProgressFlags, 8, ProgressFlagTask::eProgressFlagAction_Check));
    addTask(eTutorial_State_FurnaceMenu,
            new CraftTask(Items::coal, -1, 1, this, TutorialIds::CoalCraftCreative, true, NULL, false, true, true));
    addTask(eTutorial_State_FurnaceMenu, new InfoTask(this, TutorialIds::SmeltingInfo, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FurnaceMenu, new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this,
                                                       TutorialIds::GlassCraftCreative, true, NULL, false, true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaMinecart = gApp.getGameRuleDefinitions()->getNamedArea(L"minecartArea");
        if (areaMinecart != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_RidingMinecart,
                                 TutorialIds::MinecartArea, areaMinecart->mMinX, areaMinecart->mMinY,
                                 areaMinecart->mMinZ, areaMinecart->mMaxX, areaMinecart->mMaxY, areaMinecart->mMaxZ,
                                 true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaBoat = gApp.getGameRuleDefinitions()->getNamedArea(L"boatArea");
        if (areaBoat != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_RidingBoat,
                                 TutorialIds::BoatArea, areaBoat->mMinX, areaBoat->mMinY, areaBoat->mMinZ,
                                 areaBoat->mMaxX, areaBoat->mMaxY, areaBoat->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaFishing = gApp.getGameRuleDefinitions()->getNamedArea(L"fishingArea");
        if (areaFishing != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_Fishing,
                                 TutorialIds::FishingArea, areaFishing->mMinX, areaFishing->mMinY, areaFishing->mMinZ,
                                 areaFishing->mMaxX, areaFishing->mMaxY, areaFishing->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaPistonBridge = gApp.getGameRuleDefinitions()->getNamedArea(L"pistonBridgeArea");
        if (areaPistonBridge != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_Invalid,
                                 TutorialIds::PistonBridgeArea, areaPistonBridge->mMinX, areaPistonBridge->mMinY,
                                 areaPistonBridge->mMinZ, areaPistonBridge->mMaxX, areaPistonBridge->mMaxY,
                                 areaPistonBridge->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaEmeraldElytra = gApp.getGameRuleDefinitions()->getNamedArea(L"emeraldElytraArea");
        AABB* areaGoldElytra = gApp.getGameRuleDefinitions()->getNamedArea(L"goldElytraArea");
        AABB* areaDiamondElytra = gApp.getGameRuleDefinitions()->getNamedArea(L"diamondElytraArea");

        if (areaEmeraldElytra != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_Invalid,
                                 TutorialIds::ElytraArea, areaEmeraldElytra->mMinX, areaEmeraldElytra->mMinY,
                                 areaEmeraldElytra->mMinZ, areaEmeraldElytra->mMaxX, areaEmeraldElytra->mMaxY,
                                 areaEmeraldElytra->mMaxZ, true, true));
        }
        if (areaGoldElytra != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_Invalid,
                                 TutorialIds::ElytraArea, areaGoldElytra->mMinX, areaGoldElytra->mMinY,
                                 areaGoldElytra->mMinZ, areaGoldElytra->mMaxX, areaGoldElytra->mMaxY,
                                 areaGoldElytra->mMaxZ, true, true));
        }
        if (areaDiamondElytra != NULL)
        {
            addHint(eTutorial_State_Gameplay,
                    new AreaHint(eTutorial_Hint_General, this, eTutorial_State_Gameplay, eTutorial_State_Invalid,
                                 TutorialIds::ElytraArea, areaDiamondElytra->mMinX, areaDiamondElytra->mMinY,
                                 areaDiamondElytra->mMinZ, areaDiamondElytra->mMaxX, areaDiamondElytra->mMaxY,
                                 areaDiamondElytra->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaCreative = gApp.getGameRuleDefinitions()->getNamedArea(L"creativeArea");
        if (areaCreative != NULL)
        {
            eTutorial_State creativeExitStates[1] = {eTutorial_State_Gameplay};
            gameModeConstraints->AddGlobalConstraint(new ChangeStateConstraint(
                this, eTutorial_State_CreativeMode, creativeExitStates, 1, areaCreative->mMinX, areaCreative->mMinY,
                areaCreative->mMinZ, areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true,
                GameType::CREATIVE));

            addTask(eTutorial_State_CreativeMode,
                    new ChoiceTask(this, TutorialIds::CreativeModeChoice, TutorialIds::CreativeModeChoiceFollowup, true,
                                   0, 1, eTutorial_CompletionAction_CompleteStateExceptLastTask,
                                   eTelemetryChallenges_CreativeModeChoice));
            addTask(eTutorial_State_CreativeMode,
                    new InfoTask(this, TutorialIds::CreativeInventoryInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                 eTelemetryChallenges_None));
            addTask(eTutorial_State_CreativeMode,
                    new InfoTask(this, TutorialIds::CreativeInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                 eTelemetryChallenges_None));

            int creativeMapping[1] = {40};
            addTask(eTutorial_State_CreativeMode, new ControllerTask(this, TutorialIds::CreativeController, false,
                                                                     false, creativeMapping, 1, NULL, 0, NULL, 0));

            addTask(eTutorial_State_CreativeMode, new StateChangeTask(eTutorial_State_CreativeInventoryMenu, this, -1,
                                                                      false, NULL, false, true, true));

            ProcedureCompoundTask* compound3 = new ProcedureCompoundTask(this);

            AABB* areaCreativeExit = gApp.getGameRuleDefinitions()->getNamedArea(L"creativeExitArea");
            if (areaCreativeExit != NULL)
            {
                std::vector<TutorialConstraint*>* creativeExitConstraints = new std::vector<TutorialConstraint*>();
                creativeExitConstraints->push_back(new AreaConstraint(
                    -1, areaCreativeExit->mMinX, areaCreativeExit->mMinY, areaCreativeExit->mMinZ,
                    areaCreativeExit->mMaxX, areaCreativeExit->mMaxY, areaCreativeExit->mMaxZ, true, false, false, 2));
                compound3->AddTask(new AreaTask(eTutorial_State_CreativeMode, this, creativeExitConstraints,
                                                TutorialIds::CreativeExitArea, AreaTask::CompletionStateOnConstraints));
            }

            std::vector<TutorialConstraint*>* creativeAreaConstraints = new std::vector<TutorialConstraint*>();
            creativeAreaConstraints->push_back(new AreaConstraint(
                TutorialIds::TutorialArea, areaCreative->mMinX, areaCreative->mMinY, areaCreative->mMinZ,
                areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true, false, 2));
            compound3->AddTask(new AreaTask(eTutorial_State_CreativeMode, this, creativeAreaConstraints, -1,
                                            AreaTask::CompletionStateOnStateChange));

            compound3->AddTask(new InfoTask(this, TutorialIds::CreativeExitInfo, TutorialIds::DefaultMessageGroup, true,
                                            0, eTelemetryChallenges_None));

            addTask(eTutorial_State_CreativeMode, compound3);
        }
    }

    AddTutorialArea(eTutorial_State_Piston);
    AddTutorialArea(eTutorial_State_Portal);
    AddTutorialArea(eTutorial_State_Brewing);
    AddTutorialArea(eTutorial_State_Enchanting);
    AddTutorialArea(eTutorial_State_Anvil);
    AddTutorialArea(eTutorial_State_Trading);
    AddTutorialArea(eTutorial_State_Fireworks);
    AddTutorialArea(eTutorial_State_Beacon);
    AddTutorialArea(eTutorial_State_Hopper);
    AddTutorialArea(eTutorial_State_EnderChest);
    AddTutorialArea(eTutorial_State_Farming);
    AddTutorialArea(eTutorial_State_Breeding);
    AddTutorialArea(eTutorial_State_Golem);
    AddTutorialArea(eTutorial_State_Banner);
    AddTutorialArea(eTutorial_State_Slime);
    AddTutorialArea(eTutorial_State_Elytra);
    AddTutorialArea(eTutorial_State_Trident);
    AddTutorialArea(eTutorial_State_Conduit);
}

static std::wstring s_tutorialAreaNames[44] = {L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"",
                                               L"pistonArea",
                                               L"portalArea",
                                               L"",
                                               L"",
                                               L"",
                                               L"brewingArea",
                                               L"",
                                               L"enchantingArea",
                                               L"",
                                               L"farmingArea",
                                               L"breedingArea",
                                               L"golemArea",
                                               L"tradingArea",
                                               L"",
                                               L"anvilArea",
                                               L"",
                                               L"enderchestArea",
                                               L"",
                                               L"",
                                               L"hopperArea",
                                               L"",
                                               L"beaconArea",
                                               L"",
                                               L"fireworksArea",
                                               L"",
                                               L"",
                                               L"bannerArea",
                                               L"slimeArea",
                                               L"elytraArea",
                                               L"",
                                               L"tridentArea",
                                               L"conduitArea"};

void FullTutorial::AddTutorialArea(eTutorial_State state)
{
    std::wstring areaName = s_tutorialAreaNames[state];
    std::wstring secondAreaName(L"");
    if (gApp.getGameRuleDefinitions() == NULL || areaName == L"")
        return;

    if (state == eTutorial_State_Portal)
    {
        secondAreaName = areaName;
        areaName += L"1";
        secondAreaName += L"2";
    }

    AABB* area = gApp.getGameRuleDefinitions()->getNamedArea(areaName);
    if (area == NULL)
        return;

    if (!gameModeConstraints->HasGlobalConstraint(state))
    {
        eTutorial_State states[1] = {eTutorial_State_Gameplay};
        gameModeConstraints->AddGlobalConstraint(
            new ChangeStateConstraint(this, state, states, 1, area->mMinX, area->mMinY, area->mMinZ, area->mMaxX,
                                      area->mMaxY, area->mMaxZ, true, false, NULL));

        AABB* secondArea = gApp.getGameRuleDefinitions()->getNamedArea(secondAreaName);
        if (secondArea != NULL)
        {
            gameModeConstraints->AddGlobalConstraint(new ChangeStateConstraint(
                this, state, states, 1, secondArea->mMinX, secondArea->mMinY, secondArea->mMinZ, secondArea->mMaxX,
                secondArea->mMaxY, secondArea->mMaxZ, true, false, NULL));
        }
    }

    switch (state)
    {
    case eTutorial_State_Piston:
        addTask(eTutorial_State_Piston,
                new ChoiceTask(this, TutorialIds::PistonChoice, TutorialIds::PistonChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               eTelemetryChallenges_PistonChoice));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo4, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo5, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo6, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Piston, new InfoTask(this, TutorialIds::PistonInfo7, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Portal:
        addTask(eTutorial_State_Portal,
                new ChoiceTask(this, TutorialIds::PortalChoice, TutorialIds::PortalChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(12)));
        addTask(eTutorial_State_Portal, new InfoTask(this, TutorialIds::PortalInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Portal, new InfoTask(this, TutorialIds::PortalInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Portal, new InfoTask(this, TutorialIds::PortalInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Portal, new InfoTask(this, TutorialIds::PortalInfo4, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Portal, new InfoTask(this, TutorialIds::PortalInfo5, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Brewing:
        addTask(eTutorial_State_Brewing,
                new ChoiceTask(this, TutorialIds::BrewingChoice, TutorialIds::BrewingChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(22)));
        {
            ProcedureCompoundTask* fillBottle = new ProcedureCompoundTask(this);
            Item* bottle = Items::glassBottle;
            fillBottle->AddTask(new PickupTask(&bottle, 1, 1, -1, this, TutorialIds::BrewingBottlePickup, true, NULL,
                                               false, true, true, NULL));
            Item* potion = Items::potion;
            fillBottle->AddTask(new PickupTask(&potion, 1, 1, 0, this, TutorialIds::BrewingPotionPickup, true, NULL,
                                               false, true, true, NULL));
            addTask(eTutorial_State_Brewing, fillBottle);
            addTask(eTutorial_State_Brewing,
                    new InfoTask(this, TutorialIds::BrewingInfo1, TutorialIds::DefaultMessageGroup, true, 0,
                                 eTelemetryChallenges_None));
            addTask(eTutorial_State_Brewing,
                    new ProgressFlagTask(this, &mProgressFlags, 0x10, ProgressFlagTask::eProgressFlagAction_Set));
            addTask(eTutorial_State_Brewing,
                    new PickupTask(&potion, 1, 1, 0, this, TutorialIds::BrewingFireResistancePickup, true, NULL, false,
                                   true, true, MobEffects::FIRE_RESISTANCE));
        }
        addTask(eTutorial_State_Brewing, new InfoTask(this, TutorialIds::BrewingInfo2, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Brewing,
                new EffectChangedTask(this, TutorialIds::BrewingFireResistanceEffect, MobEffects::FIRE_RESISTANCE, true,
                                      true, false, true, true));
        addTask(eTutorial_State_Brewing, new InfoTask(this, TutorialIds::BrewingInfo3, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Enchanting:
        addTask(eTutorial_State_Enchanting,
                new ChoiceTask(this, TutorialIds::EnchantingChoice, TutorialIds::EnchantingChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(23)));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo1, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo2, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo3, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo4, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo5, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Enchanting,
                new InfoTask(this, TutorialIds::EnchantingInfo6, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        break;
    case eTutorial_State_Farming:
        addTask(eTutorial_State_Farming,
                new ChoiceTask(this, TutorialIds::FarmingChoice, TutorialIds::FarmingChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(24)));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo1, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo2, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo3, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo4, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo5, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo6, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo7, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo8, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming, new InfoTask(this, TutorialIds::FarmingInfo9, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Farming,
                new InfoTask(this, TutorialIds::FarmingInfo10, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        break;
    case eTutorial_State_Breeding:
        addTask(eTutorial_State_Breeding,
                new ChoiceTask(this, TutorialIds::BreedingChoice, TutorialIds::BreedingChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(43)));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo1, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo2, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo3, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo4, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo5, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo6, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo7, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo8, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo9, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Breeding,
                new InfoTask(this, TutorialIds::BreedingInfo10, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        break;
    case eTutorial_State_Golem:
        addTask(eTutorial_State_Golem,
                new ChoiceTask(this, TutorialIds::GolemChoice, TutorialIds::GolemChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(44)));
        addTask(eTutorial_State_Golem, new InfoTask(this, TutorialIds::GolemInfo1, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Golem, new InfoTask(this, TutorialIds::GolemInfo2, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Golem, new InfoTask(this, TutorialIds::GolemInfo3, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Golem, new InfoTask(this, TutorialIds::GolemInfo4, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Trading:
        addTask(eTutorial_State_Trading,
                new ChoiceTask(this, TutorialIds::TradingChoice, TutorialIds::TradingChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(47)));
        addTask(eTutorial_State_Trading, new InfoTask(this, TutorialIds::TradingInfo1, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trading, new InfoTask(this, TutorialIds::TradingInfo2, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trading, new InfoTask(this, TutorialIds::TradingInfo3, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trading, new InfoTask(this, TutorialIds::TradingInfo4, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trading, new InfoTask(this, TutorialIds::TradingInfo5, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Anvil:
        addTask(eTutorial_State_Anvil,
                new ChoiceTask(this, TutorialIds::AnvilChoice, TutorialIds::AnvilChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(45)));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo1, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo2, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo3, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo4, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo5, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Anvil, new InfoTask(this, TutorialIds::AnvilInfo6, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_EnderChest:
        addTask(eTutorial_State_EnderChest,
                new ChoiceTask(this, TutorialIds::EnderChestChoice, TutorialIds::EnderChestChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(49)));
        addTask(eTutorial_State_EnderChest,
                new InfoTask(this, TutorialIds::EnderChestInfo1, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_EnderChest,
                new InfoTask(this, TutorialIds::EnderChestInfo2, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_EnderChest,
                new InfoTask(this, TutorialIds::EnderChestInfo3, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        break;
    case eTutorial_State_Hopper:
        addTask(eTutorial_State_Hopper,
                new ChoiceTask(this, TutorialIds::HopperChoice, TutorialIds::HopperChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(56)));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo4, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo5, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Hopper, new InfoTask(this, TutorialIds::HopperInfo6, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Beacon:
        addTask(eTutorial_State_Beacon,
                new ChoiceTask(this, TutorialIds::BeaconChoice, TutorialIds::BeaconChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(54)));
        addTask(eTutorial_State_Beacon, new InfoTask(this, TutorialIds::BeaconInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Beacon, new InfoTask(this, TutorialIds::BeaconInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Beacon, new InfoTask(this, TutorialIds::BeaconInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Beacon, new InfoTask(this, TutorialIds::BeaconInfo4, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Fireworks:
        addTask(eTutorial_State_Fireworks,
                new ChoiceTask(this, TutorialIds::FireworksChoice, TutorialIds::FireworksChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(47)));
        addTask(eTutorial_State_Fireworks,
                new InfoTask(this, TutorialIds::FireworksInfo1, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Fireworks,
                new InfoTask(this, TutorialIds::FireworksInfo2, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        addTask(eTutorial_State_Fireworks,
                new InfoTask(this, TutorialIds::FireworksInfo3, TutorialIds::DefaultMessageGroup, true, 0,
                             eTelemetryChallenges_None));
        break;
    case eTutorial_State_Banner:
        addTask(eTutorial_State_Banner,
                new ChoiceTask(this, TutorialIds::BannerChoice, TutorialIds::BannerChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(76)));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo4, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo5, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Banner, new InfoTask(this, TutorialIds::BannerInfo6, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Slime:
        addTask(eTutorial_State_Slime,
                new ChoiceTask(this, TutorialIds::SlimeChoice, TutorialIds::SlimeChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(77)));
        addTask(eTutorial_State_Slime, new InfoTask(this, TutorialIds::SlimeInfo1, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Slime, new InfoTask(this, TutorialIds::SlimeInfo2, TutorialIds::DefaultMessageGroup,
                                                    true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Elytra:
        addTask(eTutorial_State_Elytra,
                new ChoiceTask(this, TutorialIds::ElytraChoice, TutorialIds::ElytraChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(78)));
        addTask(eTutorial_State_Elytra,
                new EquipItemTask(Items::elytra, -1, this, TutorialIds::ElytraEquip, false, NULL, false, true, true));
        addTask(eTutorial_State_Elytra, new InfoTask(this, TutorialIds::ElytraInfo1, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Elytra,
                new RepairItemTask(Items::elytra, -1, this, TutorialIds::ElytraRepair, false, NULL, false, true, true));
        addTask(eTutorial_State_Elytra,
                new ChoiceTask(this, TutorialIds::ElytraSecondChoice, TutorialIds::ElytraSecondChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(78)));
        addTask(eTutorial_State_Elytra, new InfoTask(this, TutorialIds::ElytraInfo2, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Elytra, new InfoTask(this, TutorialIds::ElytraInfo3, TutorialIds::DefaultMessageGroup,
                                                     true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Trident:
        addTask(eTutorial_State_Trident,
                new ChoiceTask(this, TutorialIds::TridentChoice, TutorialIds::TridentChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(85)));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo1, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo2, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo3, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo4, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo5, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Trident, new InfoTask(this, TutorialIds::TridentInfo6, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        break;
    case eTutorial_State_Conduit:
        addTask(eTutorial_State_Conduit,
                new ChoiceTask(this, TutorialIds::ConduitChoice, TutorialIds::ConduitChoicePrompt, true, 0, 1,
                               eTutorial_CompletionAction_CompleteStateAndConstraints,
                               static_cast<ETelemetryChallenges>(86)));
        addTask(eTutorial_State_Conduit, new InfoTask(this, TutorialIds::ConduitInfo1, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Conduit, new InfoTask(this, TutorialIds::ConduitInfo2, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Conduit, new InfoTask(this, TutorialIds::ConduitInfo3, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Conduit, new InfoTask(this, TutorialIds::ConduitInfo4, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_Conduit, new InfoTask(this, TutorialIds::ConduitInfo5, TutorialIds::DefaultMessageGroup,
                                                      true, 0, eTelemetryChallenges_None));
        break;
    default:
        break;
    }
}

void FullTutorial::PopulateChest(eTutorial_State state)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == NULL)
    {
        return;
    }

    GameRuleDefinition* definition = ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_OnInitialiseWorld);
    if (definition == NULL)
    {
        return;
    }

    ServerLevel* level = gMinecraftServer->getLevel(0);

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children, ConsoleGameRules::eGameRuleType_PopulateContainer);

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        PopulateContainerRuleDefinition* container = static_cast<PopulateContainerRuleDefinition*>(*it);

        int x;
        int y;
        int z;
        container->GetPos(x, y, z);

        if (state == eTutorial_State_Completed
            || gameModeConstraints->InConstraintArea(Vec3::newTemp(x, y, z), static_cast<int>(state)))
        {
            container->evaluateRule(level, ruleset, *MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true));
        }
    }
}

void FullTutorial::tick()
{
    if (!completedStates.empty())
    {
        std::vector<eTutorial_State>::iterator state = completedStates.begin();
        while (state != completedStates.end())
        {
            eTutorial_State current = *state;
            if (gameModeConstraints->NotInConstraintArea(playerId, static_cast<int>(current)))
            {
                mStateCompleted[static_cast<int>(current)] = 0;
                AddTutorialArea(current);
                PopulateChest(current);
                state = completedStates.erase(state);
            }
            else
            {
                ++state;
            }
        }
    }
    Tutorial::tick();
}

unsigned char FullTutorial::isStateCompleted(eTutorial_State state)
{
    return mStateCompleted[static_cast<int>(state)];
}

void FullTutorial::setStateCompleted(eTutorial_State state)
{
    mStateCompleted[static_cast<int>(state)] = 1;
    Tutorial::setStateCompleted(state);
}
