#ifndef GAME_MINIGAME_TUTORIAL_HPP
#define GAME_MINIGAME_TUTORIAL_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/item/not_null_ptr.hpp"

class Block;
class BlockPos;
class BlockState;
class Entity;
class GameModeConstraints;
class ItemInstance;
class Level;
class MobEffect;
class Player;
class TutorialHint;
class TutorialMessage;
class TutorialTask;
class UIScene;

enum eTutorial_Hint
{
    eTutorial_Hint_None = 0,
    eTutorial_Hint_General = 44,
    eTutorial_Hint_HoldToMine,
    eTutorial_Hint_ToolDamaged,
    eTutorial_Hint_SwimUp,
    eTutorial_Hint_Stone = 57,
    eTutorial_Hint_Cobblestone,
    eTutorial_Hint_Planks,
    eTutorial_Hint_Sapling,
    eTutorial_Hint_Bedrock,
    eTutorial_Hint_Water,
    eTutorial_Hint_Lava,
    eTutorial_Hint_Magma,
    eTutorial_Hint_Sand,
    eTutorial_Hint_Gravel,
    eTutorial_Hint_GoldOre,
    eTutorial_Hint_IronOre,
    eTutorial_Hint_CoalOre,
    eTutorial_Hint_Log,
    eTutorial_Hint_Leaves,
    eTutorial_Hint_Glass,
    eTutorial_Hint_LapisOre,
    eTutorial_Hint_LapisBlock,
    eTutorial_Hint_Dispenser,
    eTutorial_Hint_Sandstone,
    eTutorial_Hint_Noteblock,
    eTutorial_Hint_GoldenRail,
    eTutorial_Hint_DetectorRail,
    eTutorial_Hint_Tallgrass,
    eTutorial_Hint_Wool,
    eTutorial_Hint_RedFlower,
    eTutorial_Hint_RedMushroom,
    eTutorial_Hint_GoldBlock,
    eTutorial_Hint_IronBlock,
    eTutorial_Hint_StoneSlab,
    eTutorial_Hint_BrickBlock,
    eTutorial_Hint_Tnt,
    eTutorial_Hint_Bookshelf,
    eTutorial_Hint_MossyCobblestone,
    eTutorial_Hint_Obsidian,
    eTutorial_Hint_Torch,
    eTutorial_Hint_MobSpawner,
    eTutorial_Hint_Chest,
    eTutorial_Hint_RedstoneWire,
    eTutorial_Hint_DiamondOre,
    eTutorial_Hint_DiamondBlock,
    eTutorial_Hint_CraftingTable,
    eTutorial_Hint_Wheat,
    eTutorial_Hint_Beetroots,
    eTutorial_Hint_Farmland,
    eTutorial_Hint_Furnace,
    eTutorial_Hint_WallSign,
    eTutorial_Hint_WoodenDoor,
    eTutorial_Hint_Ladder,
    eTutorial_Hint_StoneStairs,
    eTutorial_Hint_Rail,
    eTutorial_Hint_Lever,
    eTutorial_Hint_StonePressurePlate,
    eTutorial_Hint_IronDoor,
    eTutorial_Hint_RedstoneOre,
    eTutorial_Hint_RedstoneTorch,
    eTutorial_Hint_StoneButton,
    eTutorial_Hint_Snow,
    eTutorial_Hint_Ice,
    eTutorial_Hint_Cactus,
    eTutorial_Hint_Clay,
    eTutorial_Hint_Reeds,
    eTutorial_Hint_Jukebox,
    eTutorial_Hint_Pumpkin,
    eTutorial_Hint_Netherrack,
    eTutorial_Hint_SoulSand,
    eTutorial_Hint_Glowstone,
    eTutorial_Hint_Portal,
    eTutorial_Hint_LitPumpkin,
    eTutorial_Hint_Cake,
    eTutorial_Hint_PoweredRepeater,
    eTutorial_Hint_Trapdoor,
    eTutorial_Hint_Piston,
    eTutorial_Hint_StickyPiston,
    eTutorial_Hint_MonsterEgg,
    eTutorial_Hint_Stonebrick,
    eTutorial_Hint_BrownMushroomBlock,
    eTutorial_Hint_IronBars,
    eTutorial_Hint_GlassPane,
    eTutorial_Hint_MelonBlock,
    eTutorial_Hint_Vine,
    eTutorial_Hint_FenceGate,
    eTutorial_Hint_Mycelium,
    eTutorial_Hint_Waterlily,
    eTutorial_Hint_NetherBrick,
    eTutorial_Hint_NetherBrickFence,
    eTutorial_Hint_NetherWart,
    eTutorial_Hint_EnchantingTable,
    eTutorial_Hint_BrewingStand,
    eTutorial_Hint_Cauldron,
    eTutorial_Hint_EndPortal,
    eTutorial_Hint_EndPortalFrame,
    eTutorial_Hint_ChorusFlower,
    eTutorial_Hint_ChorusPlant,
    eTutorial_Hint_EndGateway,
    eTutorial_Hint_EndRod,
    eTutorial_Hint_PurpurBlock,
    eTutorial_Hint_Grass,
    eTutorial_Hint_Dirt,
    eTutorial_Hint_CoarseDirt,
    eTutorial_Hint_Podzol,
    eTutorial_Hint_Fence,
    eTutorial_Hint_PackedIce,
    eTutorial_Hint_IronTrapdoor,
    eTutorial_Hint_Skull,
    eTutorial_Hint_Deadbush,
    eTutorial_Hint_Web,
    eTutorial_Hint_Squid,
    eTutorial_Hint_Cow,
    eTutorial_Hint_Sheep,
    eTutorial_Hint_Chicken,
    eTutorial_Hint_Pig,
    eTutorial_Hint_Wolf,
    eTutorial_Hint_Creeper,
    eTutorial_Hint_Skeleton,
    eTutorial_Hint_Spider,
    eTutorial_Hint_Zombie,
    eTutorial_Hint_PigZombie,
    eTutorial_Hint_Ghast,
    eTutorial_Hint_Slime,
    eTutorial_Hint_EnderMan,
    eTutorial_Hint_Silverfish,
    eTutorial_Hint_CaveSpider,
    eTutorial_Hint_MushroomCow,
    eTutorial_Hint_SnowMan,
    eTutorial_Hint_VillagerGolem,
    eTutorial_Hint_EnderDragon,
    eTutorial_Hint_Blaze,
    eTutorial_Hint_MagmaCube,
    eTutorial_Hint_Ocelot,
    eTutorial_Hint_Villager,
    eTutorial_Hint_WitherBoss,
    eTutorial_Hint_WitherSkeleton,
    eTutorial_Hint_Witch,
    eTutorial_Hint_Bat,
    eTutorial_Hint_Horse,
    eTutorial_Hint_Guardian,
    eTutorial_Hint_Rabbit,
    eTutorial_Hint_Endermite,
    eTutorial_Hint_PolarBear,
    eTutorial_Hint_Shulker,
    eTutorial_Hint_Stray,
    eTutorial_Hint_Husk,
    eTutorial_Hint_SkeletonHorse,
    eTutorial_Hint_Painting,
    eTutorial_Hint_ItemFrame,
    eTutorial_Hint_MinecartRideable,
    eTutorial_Hint_MinecartChest,
    eTutorial_Hint_MinecartFurnace,
    eTutorial_Hint_MinecartHopper,
    eTutorial_Hint_MinecartTnt,
    eTutorial_Hint_Boat,
    eTutorial_Hint_ArmorStand,
    eTutorial_Hint_LeashFenceKnotEntity,
    eTutorial_Hint_EnderCrystal,
    eTutorial_Hint_Shovel,
    eTutorial_Hint_Axe,
    eTutorial_Hint_Pickaxe,
    eTutorial_Hint_EndStone = 278,
    eTutorial_Hint_DragonEgg,
    eTutorial_Hint_RedstoneLamp,
    eTutorial_Hint_Cocoa,
    eTutorial_Hint_EmeraldOre,
    eTutorial_Hint_EmeraldBlock,
    eTutorial_Hint_EnderChest,
    eTutorial_Hint_TripwireHook,
    eTutorial_Hint_Tripwire,
    eTutorial_Hint_CobblestoneWall,
    eTutorial_Hint_FlowerPot,
    eTutorial_Hint_Anvil,
    eTutorial_Hint_QuartzOre,
    eTutorial_Hint_QuartzBlock,
    eTutorial_Hint_Carpet,
    eTutorial_Hint_Potatoes,
    eTutorial_Hint_Carrots,
    eTutorial_Hint_CommandBlock,
    eTutorial_Hint_Beacon,
    eTutorial_Hint_ActivatorRail,
    eTutorial_Hint_RedstoneBlock,
    eTutorial_Hint_DaylightDetector,
    eTutorial_Hint_Dropper,
    eTutorial_Hint_Hopper,
    eTutorial_Hint_PoweredComparator,
    eTutorial_Hint_TrappedChest,
    eTutorial_Hint_HayBlock,
    eTutorial_Hint_HardenedClay,
    eTutorial_Hint_StainedHardenedClay,
    eTutorial_Hint_CoalBlock,
    eTutorial_Hint_Sponge,
    eTutorial_Hint_RedSandstone,
    eTutorial_Hint_Prismarine,
    eTutorial_Hint_SeaLantern,
    eTutorial_Hint_SlimeBlock,
    eTutorial_Hint_DoublePlant,
    eTutorial_Hint_EndBricks,
    eTutorial_Hint_RedNetherBrick,
    eTutorial_Hint_BoneBlock,
    eTutorial_Hint_GrassPath,
    eTutorial_Hint_NetherWartBlock,
    eTutorial_Hint_StandingBanner,
    eTutorial_Hint_BattleLobby = 321,
    eTutorial_Hint_BattleSelectMaps,
    eTutorial_Hint_BattleScoreboard,
    eTutorial_Hint_BattleGracePeriod,
    eTutorial_Hint_BattleExplore,
    eTutorial_Hint_BattleShowdown,
    eTutorial_Hint_BattleDied,
    eTutorial_Hint_BattleSpectateAsMob,
    eTutorial_Hint_BattleFollowMode,
    eTutorial_Hint_BattleHunger,
    eTutorial_Hint_BattleLowHunger,
    eTutorial_Hint_BattleSprint,
    eTutorial_Hint_BattleLastSurvivor,
    eTutorial_Hint_BattleChestRefill = 338,
    eTutorial_Hint_BattleWaitingForLoad = 340,
    eTutorial_Hint_BattleWaitingForLoad2,
    eTutorial_Hint_BattleWaitingForLoad3,
    eTutorial_Hint_TumbleLobby = 344,
    eTutorial_Hint_TumbleScoreboard,
    eTutorial_Hint_TumbleDied,
    eTutorial_Hint_TumbleSpectateAsMob,
    eTutorial_Hint_TumbleFollowMode,
    eTutorial_Hint_TumbleBreakBlocks,
    eTutorial_Hint_Concrete,
    eTutorial_Hint_ConcretePowder,
    eTutorial_Hint_WhiteGlazedTerracotta,
    eTutorial_Hint_WhiteShulkerBox,
    eTutorial_Hint_ZombieHorse,
    eTutorial_Hint_Llama,
    eTutorial_Hint_Parrot,
    eTutorial_Hint_Mule,
    eTutorial_Hint_Donkey,
    eTutorial_Hint_Vex,
    eTutorial_Hint_EvocationIllager,
    eTutorial_Hint_VindicationIllager,
    eTutorial_Hint_ElderGuardian,
    eTutorial_Hint_ZombieVillager,
    eTutorial_Hint_Observer,
    eTutorial_Hint_Turtle = 366,
    eTutorial_Hint_Dolphin,
    eTutorial_Hint_Fish,
    eTutorial_Hint_Drowned,
    eTutorial_Hint_Phantom,
    eTutorial_Hint_CoralBlock,
    eTutorial_Hint_DeadCoralBlock,
    eTutorial_Hint_Coral,
    eTutorial_Hint_CoralFan,
    eTutorial_Hint_CoralFanDead,
    eTutorial_Hint_Kelp,
    eTutorial_Hint_SeaGrass,
    eTutorial_Hint_SeaPickle,
    eTutorial_Hint_BlueIce,
    eTutorial_Hint_TurtleEgg,
    eTutorial_Hint_DriedKelpBlock,
    eTutorial_Hint_StrippedLogOak,
    eTutorial_Hint_Conduit,
    eTutorial_Hint_CarvedPumpkin
};

enum eTutorial_State
{
    eTutorial_State_Completed = -2,
    eTutorial_State_Invalid,
    eTutorial_State_Gameplay,
    eTutorial_State_InventoryMenu,
    eTutorial_State_CraftingMenu,
    eTutorial_State_CraftingTableMenu,
    eTutorial_State_FurnaceMenu,
    eTutorial_State_RidingMinecart,
    eTutorial_State_RidingBoat,
    eTutorial_State_Fishing,
    eTutorial_State_NameTag,
    eTutorial_State_Bed,
    eTutorial_State_ContainerMenu,
    eTutorial_State_DispenserMenu,
    eTutorial_State_Piston,
    eTutorial_State_Portal,
    eTutorial_State_CreativeInventoryMenu,
    eTutorial_State_FoodBar,
    eTutorial_State_CreativeMode,
    eTutorial_State_Brewing,
    eTutorial_State_BrewingMenu,
    eTutorial_State_Enchanting,
    eTutorial_State_EnchantingMenu,
    eTutorial_State_Farming,
    eTutorial_State_Breeding,
    eTutorial_State_Golem,
    eTutorial_State_Trading,
    eTutorial_State_TradingMenu,
    eTutorial_State_Anvil,
    eTutorial_State_AnvilMenu,
    eTutorial_State_EnderChest,
    eTutorial_State_Horse,
    eTutorial_State_HorseMenu,
    eTutorial_State_Hopper,
    eTutorial_State_HopperMenu,
    eTutorial_State_Beacon,
    eTutorial_State_BeaconMenu,
    eTutorial_State_Fireworks,
    eTutorial_State_FireworksMenu,
    eTutorial_State_SurvivalCrafting,
    eTutorial_State_Banner,
    eTutorial_State_Slime,
    eTutorial_State_Elytra,
    eTutorial_State_Trident = 42,
    eTutorial_State_Conduit,
    eTutorial_State_Max
};

enum eTutorial_GameMode
{
    eTutorial_GameMode_Lobby = 0,
    eTutorial_GameMode_RoundStarting = 1,
    eTutorial_GameMode_GracePeriod = 2,
    eTutorial_GameMode_RoundPlaying = 3,
    eTutorial_GameMode_ChestRefill = 4,
    eTutorial_GameMode_Showdown = 5,
    eTutorial_GameMode_SpectatingInvisible = 6,
    eTutorial_GameMode_SpectatingAsMob = 7,
    eTutorial_GameMode_Following = 8,
    eTutorial_GameMode_Scoreboard = 9,
    eTutorial_GameMode_Fireworks = 10
};

enum eTutorial_CompletionAction
{
    eTutorial_CompletionAction_None = 0,
    eTutorial_CompletionAction_CompleteState,
    eTutorial_CompletionAction_CompleteStateAndConstraints,
    eTutorial_CompletionAction_CompleteStateExceptLastTask
};

extern unsigned int gTutorialVisibleTime;

class Tutorial
{
public:
    class PopupMessageDetails
    {
    public:
        PopupMessageDetails();
        unsigned int isSameContent(PopupMessageDetails* details);

        int titleId;
        int messageId;
        union
        {
            int promptId;
            int itemDescriptionId;
        };
        std::wstring title;
        std::wstring message;
        std::wstring itemDescription;
        boost::shared_ptr<ItemInstance> itemInstance;
        union
        {
            int displayTime;
            int itemId;
        };
        union
        {
            int priority;
            int itemAuxValue;
        };
        bool repeat;
        bool unknown;
        bool displayed;
        bool allowFade;
        bool isItem;
        bool force;
        int state;
    };

    Tutorial(int playerId, GameModeConstraints* constraints, bool enabled, bool skipSetup);
    virtual ~Tutorial();

    void addMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount);
    bool getCompleted(int completionId);
    unsigned int isHintCompleted(eTutorial_Hint hint);
    void addHint(eTutorial_State state, TutorialHint* hint);
    void addHintIfIncomplete(eTutorial_State state, TutorialHint* hint);
    void AddHints();
    void addTask(eTutorial_State state, TutorialTask* task);
    unsigned int AddTasks(bool enabled);
    unsigned int setMessage(PopupMessageDetails* details);
    void setCompleted(int completionId);
    void setHintCompleted(eTutorial_Hint hint);
    void setHintCompleted(TutorialHint* hint);
    int setMessage(TutorialHint* hint, PopupMessageDetails* details);
    unsigned int isSelectedItemState();
    virtual void tick();
    virtual unsigned char isStateCompleted(eTutorial_State state);
    virtual void setStateCompleted(eTutorial_State state);
    virtual void changeTutorialState(eTutorial_State state, UIScene* scene);
    void setMessage(const std::wstring& message, int displayTime, int priority);
    void showTutorialPopup(bool show);
    void useItemOn(Level* level, not_null_ptr<class ItemInstance> item, const BlockPos& pos, bool cancelled);
    void useItemOn(not_null_ptr<class ItemInstance> item, bool cancelled);
    void completeUsingItem(not_null_ptr<class ItemInstance> item);
    void startDestroyBlock(not_null_ptr<class ItemInstance> item, const BlockState* state);
    void destroyBlock(Block* block);
    void attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity);
    void itemDamaged(not_null_ptr<class ItemInstance> item);
    void handleUIInput(int input);
    void createItemSelected(not_null_ptr<class ItemInstance> item, bool selected);
    void onCrafted(not_null_ptr<class ItemInstance> item);
    void onEquip(not_null_ptr<class ItemInstance> item);
    void onTakeFromAnvil(not_null_ptr<class ItemInstance> item);
    void onTake(not_null_ptr<class ItemInstance> item, unsigned int itemCount, unsigned int exactCount);
    unsigned int onSelectedItemChanged(not_null_ptr<class ItemInstance> item);
    void onLookAt(const BlockState* state);
    void onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state);
    void onLookAtEntity(boost::shared_ptr<Entity> entity);
    void onRideEntity(boost::shared_ptr<Entity> entity);
    void onEffectChanged(MobEffect* effect, bool added);
    void onGameModeChange(eTutorial_GameMode gameMode);
    static void staticCtor();

    std::vector<TutorialTask*>* getTasks();
    int getCurrentTaskIndex();
    static void debugResetPlayerSavedProgress(int playerId);

    eTutorial_State currentState;
    bool stateChanged;
    bool splitScreen;
    bool started;
    int startTime;
    GameModeConstraints* gameModeConstraints;
    boost::unordered_map<int, TutorialMessage*> messages;
    std::vector<TutorialTask*> tasks;
    std::vector<TutorialTask*> stateTasks[eTutorial_State_Max];
    std::vector<TutorialHint*> stateHints[eTutorial_State_Max];
    TutorialTask* currentTask[eTutorial_State_Max];
    std::vector<eTutorial_State> completedStates;
    bool overridesGameHostOptions;
    bool gameHostOptionsOverridden;
    eTutorial_GameMode gameMode;
    bool gameModeChanged;
    bool hintShown;
    int playerId;
    int visibleTime;
    int messageTime;
    PopupMessageDetails* currentMessage;
    int messageState;
    int messageRepeatCount;
    bool popupVisible;
    bool popupPending;
    bool uiControllerSet;
    bool tutorialHidden;
    UIScene* currentScene;
    bool tasksChecked;
    bool completed;
    bool enabled;
};

#endif
