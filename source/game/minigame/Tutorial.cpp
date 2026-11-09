#include "game/minigame/Tutorial.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcInput.hpp"
#endif

#include <algorithm>
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ChoiceTask.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/InfoTask.hpp"
#include "game/core/ProcedureCompoundTask.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/HorseChoiceTask.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LookAtEntityHint.hpp"
#include "game/item/DiggerItemHint.hpp"
#include "game/item/Item.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/TutorialHint.hpp"
#include "game/minigame/TutorialMessage.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/minigame/TutorialConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/LookAtBlockHint.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "vendor/_TutorialPopupInfo.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern ConsoleUIController gConsoleUIController;

unsigned int gTutorialVisibleTime = 7000;

namespace
{
const unsigned int gTutorialStartDelay = 1500;
unsigned int gTutorialHintRepeatTime = 14000;
unsigned int gTutorialMinimumDisplayTime = 2000;
unsigned int gTutorialMessageRepeatDelay = 13000;
long long gTutorialDayTime = 8000;
unsigned int gTutorialMessageRepeatTime = gTutorialVisibleTime + gTutorialMessageRepeatDelay;
std::vector<int> gTutorialCompletionIds;
}

Tutorial::PopupMessageDetails::PopupMessageDetails()
    : titleId(-1),
      messageId(-1),
      promptId(-1),
      title(L""),
      message(L""),
      itemDescription(L""),
      itemInstance(ItemInstance::EMPTY),
      displayTime(-1),
      priority(0),
      repeat(true),
      unknown(false),
      displayed(false),
      allowFade(false),
      isItem(false),
      force(false),
      state(-1)
{
}

unsigned int Tutorial::PopupMessageDetails::isSameContent(PopupMessageDetails* details)
{
    if (details == NULL)
    {
        return 0;
    }
    return (titleId == details->titleId && messageId == details->messageId && promptId == details->promptId
            && title == details->title && message == details->message && itemDescription == details->itemDescription)
               ? 1U
               : 0U;
}

Tutorial::Tutorial(int playerIdValue, GameModeConstraints* constraints, bool enabledValue, bool skipSetup)
{
    playerId = playerIdValue;
    enabled = enabledValue;
    currentScene = NULL;
    gameMode = eTutorial_GameMode_ChestRefill;
    popupPending = false;
    currentMessage = NULL;
    startTime = 0;
    hintShown = false;
    started = false;
    popupVisible = true;
    uiControllerSet = false;
    currentState = eTutorial_State_Gameplay;
    completed = false;
    gameHostOptionsOverridden = false;
    gameModeChanged = false;
    stateChanged = false;
    messageRepeatCount = 0;
    tasksChecked = false;
    tutorialHidden = false;
    gameModeConstraints = constraints;
    overridesGameHostOptions = false;
    visibleTime = 0;
    messageState = 0;
    for (int i = 0; i < eTutorial_State_Max; ++i)
    {
        currentTask[i] = NULL;
    }
    if (!skipSetup)
    {
        AddHints();
        AddTasks(enabledValue);
    }
}

Tutorial::~Tutorial()
{
    boost::unordered_map<int, TutorialMessage*>::iterator message = messages.begin();
    while (message != messages.end())
    {
        delete message->second;
        ++message;
    }
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin(); it < stateTasks[state].end(); ++it)
        {
            delete *it;
        }
        for (std::vector<TutorialHint*>::iterator it = stateHints[state].begin(); it < stateHints[state].end(); ++it)
        {
            delete *it;
        }
        currentTask[state] = NULL;
    }
}

void Tutorial::addMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount)
{
    if (messageId == -1 || messages.find(messageId) != messages.end())
    {
        return;
    }
    messages[messageId] = new TutorialMessage(messageId, repeatMessage, maxDisplayCount);
}

bool Tutorial::getCompleted(int completionId)
{
    unsigned int index = static_cast<unsigned int>(-1);
    for (std::vector<int>::const_iterator it = gTutorialCompletionIds.begin(); it < gTutorialCompletionIds.end(); ++it)
    {
        ++index;
        if (*it == completionId)
            break;
    }
    if (index >= 0x200)
        return 0;
    return GameSettings::GetTutorialCompletionFlag(playerId, static_cast<int>(index));
}

unsigned int Tutorial::isHintCompleted(eTutorial_Hint hint)
{
    return getCompleted(hint);
}

void Tutorial::addHint(eTutorial_State state, TutorialHint* hint)
{
    stateHints[state].push_back(hint);
}

void Tutorial::addHintIfIncomplete(eTutorial_State state, TutorialHint* hint)
{
    if (!isHintCompleted(hint->hint))
    {
        addHint(state, hint);
        return;
    }
    delete hint;
}

void Tutorial::AddHints()
{
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new TutorialHint(eTutorial_Hint_HoldToMine, this, IDS_TUTORIAL_HINT_HOLD_TO_MINE,
                                         TutorialHint::HINT_START_DESTROY_BLOCK, true, true, -1));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new TutorialHint(eTutorial_Hint_ToolDamaged, this, IDS_TUTORIAL_HINT_TOOL_DAMAGED,
                                         TutorialHint::HINT_ITEM_DAMAGED, true, true, -1));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new TutorialHint(eTutorial_Hint_SwimUp, this, IDS_TUTORIAL_HINT_SWIM_UP,
                                         TutorialHint::HINT_WATER_MOVEMENT, true, true, -1));
    const BlockState* stoneStates[] = {Blocks::stone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Stone, this, stoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* cobblestoneStates[] = {Blocks::cobblestone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cobblestone, this,
                                                                      cobblestoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* planksStates[] = {Blocks::planks->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Planks, this, planksStates, 1, NULL, 0, -1, -1, false));
    const BlockState* saplingStates[] = {Blocks::sapling->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Sapling, this, saplingStates, 1, NULL, 0, -1, -1, false));
    const BlockState* bedrockStates[] = {Blocks::bedrock->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Bedrock, this, bedrockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* waterStates[] = {Blocks::water->defaultBlockState(), Blocks::flowing_water->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Water, this, waterStates, 2, NULL, 0, -1, -1, false));
    const BlockState* lavaStates[] = {Blocks::lava->defaultBlockState(), Blocks::flowing_lava->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Lava, this, lavaStates, 2, NULL, 0, -1, -1, false));
    const BlockState* magmaStates[] = {Blocks::magma->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Magma, this, magmaStates, 1, NULL, 0, -1, -1, false));
    const BlockState* sandStates[] = {Blocks::sand->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Sand, this, sandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* gravelStates[] = {Blocks::gravel->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Gravel, this, gravelStates, 1, NULL, 0, -1, -1, false));
    const BlockState* goldOreStates[] = {Blocks::gold_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_GoldOre, this, goldOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* ironOreStates[] = {Blocks::iron_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_IronOre, this, ironOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coalOreStates[] = {Blocks::coal_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_CoalOre, this, coalOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* logStates[] = {Blocks::log->defaultBlockState(), Blocks::log2->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Log, this, logStates, 2, NULL, 0, -1, -1, false));
    const BlockState* leavesStates[] = {Blocks::leaves->defaultBlockState(), Blocks::leaves2->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Leaves, this, leavesStates, 2, NULL, 0, -1, -1, false));
    const BlockState* glassStates[] = {Blocks::glass->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Glass, this, glassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* lapisOreStates[] = {Blocks::lapis_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_LapisOre, this, lapisOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* lapisBlockStates[] = {Blocks::lapis_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_LapisBlock, this, lapisBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* dispenserStates[] = {Blocks::dispenser->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Dispenser, this, dispenserStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* sandstoneStates[] = {Blocks::sandstone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Sandstone, this, sandstoneStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* noteblockStates[] = {Blocks::noteblock->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Noteblock, this, noteblockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* goldenRailStates[] = {Blocks::golden_rail->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GoldenRail, this, goldenRailStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* detectorRailStates[] = {Blocks::detector_rail->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DetectorRail, this,
                                                                      detectorRailStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tallgrassStates[] = {Blocks::tallgrass->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Tallgrass, this, tallgrassStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* woolStates[] = {Blocks::wool->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Wool, this, woolStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redFlowerStates[] = {Blocks::red_flower->defaultBlockState(),
                                           Blocks::yellow_flower->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedFlower, this, redFlowerStates,
                                                                      2, NULL, 0, -1, -1, false));
    const BlockState* redMushroomStates[] = {Blocks::red_mushroom->defaultBlockState(),
                                             Blocks::brown_mushroom->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedMushroom, this,
                                                                      redMushroomStates, 2, NULL, 0, -1, -1, false));
    const BlockState* goldBlockStates[] = {Blocks::gold_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GoldBlock, this, goldBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* ironBlockStates[] = {Blocks::iron_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronBlock, this, ironBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* stoneSlabStates[] = {
        Blocks::stone_slab->defaultBlockState(),        Blocks::stone_slab2->defaultBlockState(),
        Blocks::double_stone_slab->defaultBlockState(), Blocks::double_stone_slab2->defaultBlockState(),
        Blocks::purpur_slab->defaultBlockState(),       Blocks::prismarine_slab->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneSlab, this, stoneSlabStates,
                                                                      6, NULL, 0, -1, -1, false));
    const BlockState* woodenSlabStates[] = {Blocks::wooden_slab->defaultBlockState(),
                                            Blocks::double_wooden_slab->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneSlab, this, woodenSlabStates,
                                                                      2, NULL, 0, -1, -1, false));
    const BlockState* brickBlockStates[] = {Blocks::brick_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BrickBlock, this, brickBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* tntStates[] = {Blocks::tnt->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Tnt, this, tntStates, 1, NULL, 0, -1, -1, false));
    const BlockState* bookshelfStates[] = {Blocks::bookshelf->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Bookshelf, this, bookshelfStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* mossyCobblestoneStates[] = {Blocks::mossy_cobblestone->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_MossyCobblestone, this, mossyCobblestoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* obsidianStates[] = {Blocks::obsidian->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Obsidian, this, obsidianStates, 1, NULL, 0, -1, -1, false));
    const BlockState* torchStates[] = {Blocks::torch->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Torch, this, torchStates, 1, NULL, 0, -1, -1, false));
    const BlockState* mobSpawnerStates[] = {Blocks::mob_spawner->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MobSpawner, this, mobSpawnerStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* chestStates[] = {Blocks::chest->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Chest, this, chestStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redstoneWireStates[] = {Blocks::redstone_wire->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_RedstoneWire, this, redstoneWireStates, 1, NULL, 0,
                                            Item::getId(Items::redstone), -1, false));
    const BlockState* diamondOreStates[] = {Blocks::diamond_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DiamondOre, this, diamondOreStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* diamondBlockStates[] = {Blocks::diamond_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DiamondBlock, this,
                                                                      diamondBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* craftingTableStates[] = {Blocks::crafting_table->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CraftingTable, this,
                                                                      craftingTableStates, 1, NULL, 0, -1, -1, false));
    const BlockState* wheatStates[] = {Blocks::wheat->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Wheat, this, wheatStates, 1, NULL, 0, -1, 7, false));
    const BlockState* beetrootsStates[] = {Blocks::beetroots->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Beetroots, this, beetrootsStates, 1, NULL, 0, -1, 7, false));
    const BlockState* farmlandStates[] = {Blocks::farmland->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Farmland, this, farmlandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* furnaceStates[] = {Blocks::furnace->defaultBlockState(),
                                         Blocks::lit_furnace->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Furnace, this, furnaceStates, 2, NULL, 0, -1, -1, false));
    const BlockState* wallSignStates[] = {Blocks::wall_sign->defaultBlockState(),
                                          Blocks::standing_sign->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WallSign, this, wallSignStates, 2,
                                                                      NULL, 0, Item::getId(Items::sign), -1, false));
    const BlockState* woodenDoorStates[] = {
        Blocks::wooden_door->defaultBlockState(), Blocks::spruce_door->defaultBlockState(),
        Blocks::birch_door->defaultBlockState(),  Blocks::jungle_door->defaultBlockState(),
        Blocks::acacia_door->defaultBlockState(), Blocks::dark_oak_door->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WoodenDoor, this, woodenDoorStates,
                                                                      6, NULL, 0, -1, -1, true));
    const BlockState* ladderStates[] = {Blocks::ladder->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Ladder, this, ladderStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stoneStairsStates[] = {Blocks::stone_stairs->defaultBlockState(),
                                             Blocks::brick_stairs->defaultBlockState(),
                                             Blocks::stone_brick_stairs->defaultBlockState(),
                                             Blocks::nether_brick_stairs->defaultBlockState(),
                                             Blocks::red_sandstone_stairs->defaultBlockState(),
                                             Blocks::quartz_stairs->defaultBlockState(),
                                             Blocks::purpur_stairs->defaultBlockState(),
                                             Blocks::oak_stairs->defaultBlockState(),
                                             Blocks::spruce_stairs->defaultBlockState(),
                                             Blocks::birch_stairs->defaultBlockState(),
                                             Blocks::jungle_stairs->defaultBlockState(),
                                             Blocks::acacia_stairs->defaultBlockState(),
                                             Blocks::dark_oak_stairs->defaultBlockState(),
                                             Blocks::prismarine_stairs->defaultBlockState(),
                                             Blocks::prismarine_bricks_stairs->defaultBlockState(),
                                             Blocks::dark_prismarine_stairs->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneStairs, this,
                                                                      stoneStairsStates, 16, NULL, 0, -1, -1, false));
    const BlockState* railStates[] = {Blocks::rail->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Rail, this, railStates, 1, NULL, 0, -1, -1, false));
    const BlockState* leverStates[] = {Blocks::lever->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Lever, this, leverStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stonePressurePlateStates[] = {Blocks::stone_pressure_plate->defaultBlockState(),
                                                    Blocks::wooden_pressure_plate->defaultBlockState(),
                                                    Blocks::acacia_pressure_plate->defaultBlockState(),
                                                    Blocks::birch_pressure_plate->defaultBlockState(),
                                                    Blocks::dark_oak_pressure_plate->defaultBlockState(),
                                                    Blocks::jungle_pressure_plate->defaultBlockState(),
                                                    Blocks::spruce_pressure_plate->defaultBlockState(),
                                                    Blocks::light_weighted_pressure_plate->defaultBlockState(),
                                                    Blocks::heavy_weighted_pressure_plate->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_StonePressurePlate, this, stonePressurePlateStates, 9, NULL,
                                            0, -1, -1, false));
    const BlockState* ironDoorStates[] = {Blocks::iron_door->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_IronDoor, this, ironDoorStates, 1, NULL, 0,
                                            Item::getId(Items::ironDoor), -1, false));
    const BlockState* redstoneOreStates[] = {Blocks::redstone_ore->defaultBlockState(),
                                             Blocks::lit_redstone_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneOre, this,
                                                                      redstoneOreStates, 2, NULL, 0, -1, -1, false));
    const BlockState* redstoneTorchStates[] = {Blocks::redstone_torch->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneTorch, this,
                                                                      redstoneTorchStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stoneButtonStates[] = {
        Blocks::stone_button->defaultBlockState(),    Blocks::wooden_button->defaultBlockState(),
        Blocks::acacia_button->defaultBlockState(),   Blocks::birch_button->defaultBlockState(),
        Blocks::dark_oak_button->defaultBlockState(), Blocks::jungle_button->defaultBlockState(),
        Blocks::spruce_button->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneButton, this,
                                                                      stoneButtonStates, 7, NULL, 0, -1, -1, false));
    if (!isHintCompleted(eTutorial_Hint_Snow))
    {
        const BlockState* snowStates[] = {Blocks::snow->defaultBlockState()};
        addHintIfIncomplete(eTutorial_State_Gameplay,
                            new LookAtBlockHint(eTutorial_Hint_Snow, this, snowStates, 1, NULL, 0, -1, -1, false));
    }
    const BlockState* iceStates[] = {Blocks::ice->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Ice, this, iceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* cactusStates[] = {Blocks::cactus->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Cactus, this, cactusStates, 1, NULL, 0, -1, -1, false));
    const BlockState* clayStates[] = {Blocks::clay->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Clay, this, clayStates, 1, NULL, 0, -1, -1, false));
    const BlockState* reedsStates[] = {Blocks::reeds->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Reeds, this, reedsStates, 1, NULL, 0, -1, -1, false));
    const BlockState* jukeboxStates[] = {Blocks::jukebox->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Jukebox, this, jukeboxStates, 1, NULL, 0, -1, -1, false));
    const BlockState* pumpkinStates[] = {Blocks::pumpkin->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Pumpkin, this, pumpkinStates, 1, NULL, 0, -1, 0, false));
    const BlockState* netherrackStates[] = {Blocks::netherrack->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Netherrack, this, netherrackStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* soulSandStates[] = {Blocks::soul_sand->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_SoulSand, this, soulSandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* glowstoneStates[] = {Blocks::glowstone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Glowstone, this, glowstoneStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* portalStates[] = {Blocks::portal->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Portal, this, portalStates, 1, NULL, 0, -1, -1, false));
    const BlockState* litPumpkinStates[] = {Blocks::lit_pumpkin->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_LitPumpkin, this, litPumpkinStates,
                                                                      1, NULL, 0, -1, 0, false));
    const BlockState* cakeStates[] = {Blocks::cake->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Cake, this, cakeStates, 1, NULL, 0, -1, -1, false));
    const BlockState* poweredRepeaterStates[] = {Blocks::powered_repeater->defaultBlockState(),
                                                 Blocks::unpowered_repeater->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_PoweredRepeater, this, poweredRepeaterStates, 2, NULL, 0,
                                            Item::getId(Items::repeater), -1, false));
    const BlockState* trapdoorStates[] = {
        Blocks::trapdoor->defaultBlockState(),        Blocks::spruce_trapdoor->defaultBlockState(),
        Blocks::birch_trapdoor->defaultBlockState(),  Blocks::jungle_trapdoor->defaultBlockState(),
        Blocks::acacia_trapdoor->defaultBlockState(), Blocks::dark_oak_trapdoor->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Trapdoor, this, trapdoorStates, 6, NULL, 0, -1, -1, false));
    const BlockState* pistonStates[] = {Blocks::piston->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Piston, this, pistonStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stickyPistonStates[] = {Blocks::sticky_piston->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StickyPiston, this,
                                                                      stickyPistonStates, 1, NULL, 0, -1, -1, false));
    const BlockState* monsterEggStates[] = {Blocks::monster_egg->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MonsterEgg, this, monsterEggStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* stonebrickStates[] = {Blocks::stonebrick->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Stonebrick, this, stonebrickStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* brownMushroomBlockStates[] = {Blocks::brown_mushroom_block->defaultBlockState(),
                                                    Blocks::red_mushroom_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_BrownMushroomBlock, this, brownMushroomBlockStates, 2, NULL,
                                            0, -1, -1, false));
    const BlockState* ironBarsStates[] = {Blocks::iron_bars->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_IronBars, this, ironBarsStates, 1, NULL, 0, -1, -1, false));
    const BlockState* glassPaneStates[] = {Blocks::glass_pane->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GlassPane, this, glassPaneStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* melonBlockStates[] = {Blocks::melon_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MelonBlock, this, melonBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* vineStates[] = {Blocks::vine->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Vine, this, vineStates, 1, NULL, 0, -1, -1, false));
    const BlockState* fenceGateStates[] = {
        Blocks::fence_gate->defaultBlockState(),        Blocks::spruce_fence_gate->defaultBlockState(),
        Blocks::birch_fence_gate->defaultBlockState(),  Blocks::jungle_fence_gate->defaultBlockState(),
        Blocks::acacia_fence_gate->defaultBlockState(), Blocks::dark_oak_fence_gate->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_FenceGate, this, fenceGateStates,
                                                                      6, NULL, 0, -1, -1, false));
    const BlockState* myceliumStates[] = {Blocks::mycelium->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Mycelium, this, myceliumStates, 1, NULL, 0, -1, -1, false));
    const BlockState* waterlilyStates[] = {Blocks::waterlily->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Waterlily, this, waterlilyStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* netherBrickStates[] = {Blocks::nether_brick->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherBrick, this,
                                                                      netherBrickStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherBrickFenceStates[] = {Blocks::nether_brick_fence->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_NetherBrickFence, this, netherBrickFenceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherWartStates[] = {Blocks::nether_wart->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherWart, this, netherWartStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* enchantingTableStates[] = {Blocks::enchanting_table->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_EnchantingTable, this, enchantingTableStates, 1, NULL, 0, -1, -1, false));
    const BlockState* brewingStandStates[] = {Blocks::brewing_stand->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_BrewingStand, this, brewingStandStates, 1, NULL, 0,
                                            Item::getId(Items::brewingStand), -1, false));
    const BlockState* cauldronStates[] = {Blocks::cauldron->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Cauldron, this, cauldronStates, 1, NULL, 0,
                                            Item::getId(Items::cauldron), -1, false));
    const BlockState* endPortalStates[] = {Blocks::end_portal->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndPortal, this, endPortalStates,
                                                                      1, NULL, 0, -2, -1, false));
    const BlockState* endPortalFrameStates[] = {Blocks::end_portal_frame->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndPortalFrame, this,
                                                                      endPortalFrameStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endStoneStates[] = {Blocks::end_stone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_EndStone, this, endStoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dragonEggStates[] = {Blocks::dragon_egg->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DragonEgg, this, dragonEggStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* redstoneLampStates[] = {Blocks::redstone_lamp->defaultBlockState(),
                                              Blocks::lit_redstone_lamp->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneLamp, this,
                                                                      redstoneLampStates, 2, NULL, 0, -1, -1, false));
    const BlockState* cocoaStates[] = {Blocks::cocoa->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Cocoa, this, cocoaStates, 1, NULL, 0,
                                            Item::getId(Items::dye), DyeColor::BROWN->getItemData(), false));
    const BlockState* emeraldOreStates[] = {Blocks::emerald_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EmeraldOre, this, emeraldOreStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* emeraldBlockStates[] = {Blocks::emerald_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EmeraldBlock, this,
                                                                      emeraldBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* enderChestStates[] = {Blocks::ender_chest->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EnderChest, this, enderChestStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* tripwireHookStates[] = {Blocks::tripwire_hook->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TripwireHook, this,
                                                                      tripwireHookStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tripwireStates[] = {Blocks::tripwire->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Tripwire, this, tripwireStates, 1,
                                                                      NULL, 0, Item::getId(Items::string), -1, false));
    const BlockState* cobblestoneWallStates[] = {Blocks::cobblestone_wall->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_CobblestoneWall, this, cobblestoneWallStates, 1, NULL, 0, -1, -1, false));
    const BlockState* flowerPotStates[] = {Blocks::flower_pot->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_FlowerPot, this, flowerPotStates, 1, NULL, 0,
                                            Item::getId(Items::flowerPot), -1, false));
    const BlockState* anvilStates[] = {Blocks::anvil->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Anvil, this, anvilStates, 1, NULL, 0, -1, -1, false));
    const BlockState* quartzOreStates[] = {Blocks::quartz_ore->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_QuartzOre, this, quartzOreStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* quartzBlockStates[] = {Blocks::quartz_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_QuartzBlock, this,
                                                                      quartzBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* carpetStates[] = {Blocks::carpet->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Carpet, this, carpetStates, 1, NULL, 0, -1, -1, false));
    const BlockState* potatoesStates[] = {Blocks::potatoes->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Potatoes, this, potatoesStates, 1, NULL, 0, -1, 7, false));
    const BlockState* carrotsStates[] = {Blocks::carrots->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Carrots, this, carrotsStates, 1, NULL, 0, -1, 7, false));
    const BlockState* commandBlockStates[] = {Blocks::command_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CommandBlock, this,
                                                                      commandBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* beaconStates[] = {Blocks::beacon->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Beacon, this, beaconStates, 1, NULL, 0, -1, -1, false));
    const BlockState* activatorRailStates[] = {Blocks::activator_rail->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ActivatorRail, this,
                                                                      activatorRailStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redstoneBlockStates[] = {Blocks::redstone_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneBlock, this,
                                                                      redstoneBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* daylightDetectorStates[] = {Blocks::daylight_detector->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_DaylightDetector, this, daylightDetectorStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dropperStates[] = {Blocks::dropper->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Dropper, this, dropperStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hopperStates[] = {Blocks::hopper->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Hopper, this, hopperStates, 1, NULL, 0, -1, -1, false));
    const BlockState* poweredComparatorStates[] = {Blocks::powered_comparator->defaultBlockState(),
                                                   Blocks::unpowered_comparator->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_PoweredComparator, this, poweredComparatorStates, 2, NULL, 0,
                                            Item::getId(Items::comparator), -1, false));
    const BlockState* trappedChestStates[] = {Blocks::trapped_chest->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TrappedChest, this,
                                                                      trappedChestStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hayBlockStates[] = {Blocks::hay_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_HayBlock, this, hayBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hardenedClayStates[] = {Blocks::hardened_clay->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_HardenedClay, this,
                                                                      hardenedClayStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stainedHardenedClayStates[] = {Blocks::stained_hardened_clay->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_StainedHardenedClay, this, stainedHardenedClayStates, 1,
                                            NULL, 0, -1, -1, false));
    const BlockState* coalBlockStates[] = {Blocks::coal_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoalBlock, this, coalBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* spongeStates[] = {Blocks::sponge->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Sponge, this, spongeStates, 1, NULL, 0, -1, -1, false));
    const BlockState* prismarineStates[] = {Blocks::prismarine->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Prismarine, this, prismarineStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* seaLanternStates[] = {Blocks::sea_lantern->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SeaLantern, this, seaLanternStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* slimeStates[] = {Blocks::slime->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_SlimeBlock, this, slimeStates, 1, NULL, 0, -1, -1, false));
    const BlockState* doublePlantStates[] = {
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                            DoublePlantBlock::Variant::SUNFLOWER),
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                            DoublePlantBlock::Variant::SYRINGA),
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                            DoublePlantBlock::Variant::GRASS),
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::FERN),
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::ROSE),
        Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                            DoublePlantBlock::Variant::PAEONIA)};
    const Property* doublePlantProperties[] = {DoublePlantBlock::VARIANT};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_DoublePlant, this, doublePlantStates, 6,
                                            doublePlantProperties, 1, -1, -1, true));
    const BlockState* redSandstoneStates[] = {Blocks::red_sandstone->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedSandstone, this,
                                                                      redSandstoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endBricksStates[] = {Blocks::end_bricks->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndBricks, this, endBricksStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* redNetherBrickStates[] = {Blocks::red_nether_brick->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedNetherBrick, this,
                                                                      redNetherBrickStates, 1, NULL, 0, -1, -1, false));
    const BlockState* boneBlockStates[] = {Blocks::bone_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BoneBlock, this, boneBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* grassPathStates[] = {Blocks::grass_path->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GrassPath, this, grassPathStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* netherWartBlockStates[] = {Blocks::nether_wart_block->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_NetherWartBlock, this, netherWartBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* standingBannerStates[] = {Blocks::standing_banner->defaultBlockState(),
                                                Blocks::wall_banner->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StandingBanner, this,
                                                                      standingBannerStates, 2, NULL, 0, -1, -1, false));
    const BlockState* chorusFlowerStates[] = {Blocks::chorus_flower->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ChorusFlower, this,
                                                                      chorusFlowerStates, 1, NULL, 0, -1, -1, false));
    const BlockState* chorusPlantStates[] = {Blocks::chorus_plant->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ChorusPlant, this,
                                                                      chorusPlantStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endGatewayStates[] = {Blocks::end_gateway->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndGateway, this, endGatewayStates,
                                                                      1, NULL, 0, -2, -1, false));
    const BlockState* endRodStates[] = {Blocks::end_rod->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_EndRod, this, endRodStates, 1, NULL, 0, -1,
                                            Direction::UP->get3DDataValue(), false));
    const BlockState* purpurBlockStates[] = {Blocks::purpur_block->defaultBlockState(),
                                             Blocks::purpur_pillar->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PurpurBlock, this,
                                                                      purpurBlockStates, 2, NULL, 0, -1, -1, false));
    const BlockState* grassStates[] = {Blocks::grass->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Grass, this, grassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dirtStates[] = {
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT)};
    const Property* dirtProperties[] = {DirtBlock::VARIANT};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Dirt, this, dirtStates, 1, dirtProperties, 1, -1, -1, true));
    const BlockState* dirtStates2[] = {
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::COARSE_DIRT)};
    const Property* dirtProperties2[] = {DirtBlock::VARIANT};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoarseDirt, this, dirtStates2, 1,
                                                                      dirtProperties2, 1, -1, -1, true));
    const BlockState* dirtStates3[] = {
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::PODZOL)};
    const Property* dirtProperties3[] = {DirtBlock::VARIANT};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Podzol, this, dirtStates3, 1,
                                                                      dirtProperties3, 1, -1, -1, true));
    const BlockState* fenceStates[] = {
        Blocks::fence->defaultBlockState(),        Blocks::spruce_fence->defaultBlockState(),
        Blocks::birch_fence->defaultBlockState(),  Blocks::jungle_fence->defaultBlockState(),
        Blocks::acacia_fence->defaultBlockState(), Blocks::dark_oak_fence->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Fence, this, fenceStates, 6, NULL, 0, -1, -1, false));
    const BlockState* packedIceStates[] = {Blocks::packed_ice->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PackedIce, this, packedIceStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* ironTrapdoorStates[] = {Blocks::iron_trapdoor->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronTrapdoor, this,
                                                                      ironTrapdoorStates, 1, NULL, 0, -1, -1, false));
    const BlockState* skullStates[] = {Blocks::skull->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Skull, this, skullStates, 1, NULL, 0, -1, -1, false));
    const BlockState* deadbushStates[] = {Blocks::deadbush->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Deadbush, this, deadbushStates, 1, NULL, 0, -1, -1, false));
    const BlockState* webStates[] = {Blocks::web->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Web, this, webStates, 1, NULL, 0, -1, -1, false));
    const BlockState* concreteStates[] = {Blocks::concrete->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Concrete, this, concreteStates, 1, NULL, 0, -1, -1, false));
    const BlockState* concretePowderStates[] = {Blocks::concrete_powder->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ConcretePowder, this,
                                                                      concretePowderStates, 1, NULL, 0, -1, -1, false));
    const BlockState* whiteGlazedTerracottaStates[] = {Blocks::white_glazed_terracotta->defaultBlockState(),
                                                       Blocks::orange_glazed_terracotta->defaultBlockState(),
                                                       Blocks::magenta_glazed_terracotta->defaultBlockState(),
                                                       Blocks::light_blue_glazed_terracotta->defaultBlockState(),
                                                       Blocks::yellow_glazed_terracotta->defaultBlockState(),
                                                       Blocks::lime_glazed_terracotta->defaultBlockState(),
                                                       Blocks::pink_glazed_terracotta->defaultBlockState(),
                                                       Blocks::gray_glazed_terracotta->defaultBlockState(),
                                                       Blocks::silver_glazed_terracotta->defaultBlockState(),
                                                       Blocks::cyan_glazed_terracotta->defaultBlockState(),
                                                       Blocks::purple_glazed_terracotta->defaultBlockState(),
                                                       Blocks::blue_glazed_terracotta->defaultBlockState(),
                                                       Blocks::brown_glazed_terracotta->defaultBlockState(),
                                                       Blocks::green_glazed_terracotta->defaultBlockState(),
                                                       Blocks::red_glazed_terracotta->defaultBlockState(),
                                                       Blocks::black_glazed_terracotta->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_WhiteGlazedTerracotta, this, whiteGlazedTerracottaStates, 16,
                                            NULL, 0, -1, -1, false));
    const BlockState* whiteShulkerBoxStates[] = {
        Blocks::white_shulker_box->defaultBlockState(),   Blocks::orange_shulker_box->defaultBlockState(),
        Blocks::magenta_shulker_box->defaultBlockState(), Blocks::light_blue_shulker_box->defaultBlockState(),
        Blocks::yellow_shulker_box->defaultBlockState(),  Blocks::lime_shulker_box->defaultBlockState(),
        Blocks::pink_shulker_box->defaultBlockState(),    Blocks::gray_shulker_box->defaultBlockState(),
        Blocks::silver_shulker_box->defaultBlockState(),  Blocks::cyan_shulker_box->defaultBlockState(),
        Blocks::purple_shulker_box->defaultBlockState(),  Blocks::blue_shulker_box->defaultBlockState(),
        Blocks::brown_shulker_box->defaultBlockState(),   Blocks::green_shulker_box->defaultBlockState(),
        Blocks::red_shulker_box->defaultBlockState(),     Blocks::black_shulker_box->defaultBlockState()};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtBlockHint(eTutorial_Hint_WhiteShulkerBox, this, whiteShulkerBoxStates, 16, NULL, 0, -1, -1, false));
    const BlockState* observerStates[] = {Blocks::observer->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Observer, this, observerStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coralBlockStates[] = {Blocks::coral_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoralBlock, this, coralBlockStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* coralBlockStates2[] = {
        Blocks::coral_block->defaultBlockState()->setValue(CoralBlock::DEAD, true)};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DeadCoralBlock, this,
                                                                      coralBlockStates2, 1, NULL, 0, -1, -1, false));
    const BlockState* coralStates[] = {Blocks::coral->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Coral, this, coralStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coralFanStates[] = {
        Blocks::coral_fan->defaultBlockState(), Blocks::coral_fan_hang->defaultBlockState(),
        Blocks::coral_fan_hang2->defaultBlockState(), Blocks::coral_fan_hang3->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_CoralFan, this, coralFanStates, 4, NULL, 0, -1, -1, false));
    const BlockState* coralFanDeadStates[] = {
        Blocks::coral_fan_dead->defaultBlockState(),
        Blocks::coral_fan_hang->defaultBlockState()->setValue(CoralFanHang::DEAD, true),
        Blocks::coral_fan_hang2->defaultBlockState()->setValue(CoralFanHang::DEAD, true),
        Blocks::coral_fan_hang3->defaultBlockState()->setValue(CoralFanHang::DEAD, true)};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoralFanDead, this,
                                                                      coralFanDeadStates, 4, NULL, 0, -1, -1, false));
    const BlockState* kelpStates[] = {Blocks::kelp->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Kelp, this, kelpStates, 1, NULL, 0, -1, -1, false));
    const BlockState* seaGrassStates[] = {Blocks::sea_grass->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_SeaGrass, this, seaGrassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* seaPickleStates[] = {Blocks::sea_pickle->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SeaPickle, this, seaPickleStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* blueIceStates[] = {Blocks::blue_ice->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_BlueIce, this, blueIceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* turtleEggStates[] = {Blocks::turtle_egg->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TurtleEgg, this, turtleEggStates,
                                                                      1, NULL, 0, -1, -1, false));
    const BlockState* driedKelpBlockStates[] = {Blocks::dried_kelp_block->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DriedKelpBlock, this,
                                                                      driedKelpBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* strippedLogOakStates[] = {
        Blocks::stripped_log_oak->defaultBlockState(),    Blocks::stripped_log_spruce->defaultBlockState(),
        Blocks::stripped_log_birch->defaultBlockState(),  Blocks::stripped_log_jungle->defaultBlockState(),
        Blocks::stripped_log_acacia->defaultBlockState(), Blocks::stripped_log_dark_oak->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StrippedLogOak, this,
                                                                      strippedLogOakStates, 6, NULL, 0, -1, -1, false));
    const BlockState* conduitStates[] = {Blocks::conduit->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtBlockHint(eTutorial_Hint_Conduit, this, conduitStates, 1, NULL, 0, -1, -1, false));
    const BlockState* carvedPumpkinStates[] = {Blocks::carved_pumpkin->defaultBlockState()};
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CarvedPumpkin, this,
                                                                      carvedPumpkinStates, 1, NULL, 0, -1, -1, false));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Squid, this, IDS_DESC_SQUID, IDS_SQUID, EntityType::SQUID));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Cow, this, IDS_DESC_COW, IDS_COW, EntityType::COW));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Sheep, this, IDS_DESC_SHEEP, IDS_SHEEP, EntityType::SHEEP));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Chicken, this, IDS_DESC_CHICKEN,
                                                                       IDS_CHICKEN, EntityType::CHICKEN));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Pig, this, IDS_DESC_PIG, IDS_PIG, EntityType::PIG));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Wolf, this, IDS_DESC_WOLF, IDS_WOLF, EntityType::WOLF));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Creeper, this, IDS_DESC_CREEPER,
                                                                       IDS_CREEPER, EntityType::CREEPER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Skeleton, this, IDS_DESC_SKELETON,
                                                                       IDS_SKELETON, EntityType::SKELETON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Spider, this, IDS_DESC_SPIDER,
                                                                       IDS_SPIDER, EntityType::SPIDER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Zombie, this, IDS_DESC_ZOMBIE,
                                                                       IDS_ZOMBIE, EntityType::ZOMBIE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_PigZombie, this, IDS_DESC_PIGZOMBIE, IDS_PIGZOMBIE,
                                             EntityType::PIG_ZOMBIE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Ghast, this, IDS_DESC_GHAST, IDS_GHAST, EntityType::GHAST));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Slime, this, IDS_DESC_SLIME, IDS_SLIME, EntityType::SLIME));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_EnderMan, this, IDS_DESC_ENDERMAN,
                                                                       IDS_ENDERMAN, EntityType::ENDER_MAN));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Silverfish, this, IDS_DESC_SILVERFISH, IDS_SILVERFISH,
                                             EntityType::SILVERFISH));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_CaveSpider, this, IDS_DESC_CAVE_SPIDER, IDS_CAVE_SPIDER,
                                             EntityType::CAVE_SPIDER));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MushroomCow, this, IDS_DESC_MUSHROOM_COW, IDS_MUSHROOM_COW,
                                             EntityType::MUSHROOM_COW));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_SnowMan, this, IDS_DESC_SNOWMAN,
                                                                       IDS_SNOWMAN, EntityType::SNOW_MAN));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_VillagerGolem, this, IDS_DESC_IRONGOLEM, IDS_IRONGOLEM,
                                             EntityType::VILLAGER_GOLEM));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_EnderDragon, this, IDS_DESC_ENDERDRAGON, IDS_ENDERDRAGON,
                                             EntityType::ENDER_DRAGON));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Blaze, this, IDS_DESC_BLAZE, IDS_BLAZE, EntityType::BLAZE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MagmaCube, this, 0xA0138101,
                                                                       0x9E8D900F, EntityType::MAGMA_CUBE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Ocelot, this, 0x4C0C6E55, 0xAA44AD8B, EntityType::OCELOT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Villager, this, IDS_DESC_VILLAGER,
                                                                       IDS_VILLAGER, EntityType::VILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_WitherBoss, this, IDS_DESC_WITHER,
                                                                       IDS_WITHER, EntityType::WITHER_BOSS));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_WitherSkeleton, this, IDS_DESC_WITHER_SKELETON,
                                             IDS_WITHER_SKELETON, EntityType::WITHER_SKELETON));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Witch, this, 0xE6AF0FB, IDS_WITCH, EntityType::WITCH));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Bat, this, IDS_DESC_BAT, IDS_BAT, EntityType::BAT));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Horse, this, IDS_DESC_HORSE, IDS_HORSE, EntityType::HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Guardian, this, IDS_DESC_GUARDIAN,
                                                                       IDS_GUARDIAN, EntityType::GUARDIAN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Rabbit, this, IDS_DESC_RABBIT,
                                                                       IDS_RABBIT, EntityType::RABBIT));
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new LookAtEntityHint(eTutorial_Hint_Endermite, this, IDS_DESC_ENDERMITE, IDS_ENDERMITE, EntityType::ENDERMITE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_PolarBear, this, IDS_DESC_POLARBEAR, IDS_POLARBEAR,
                                             EntityType::POLAR_BEAR));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Shulker, this, IDS_DESC_SHULKER,
                                                                       IDS_SHULKER, EntityType::SHULKER));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Stray, this, IDS_DESC_STRAY_SKELETON, IDS_STRAY_SKELETON,
                                             EntityType::STRAY));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Husk, this, 0xC1C90A7, IDS_HUSK, EntityType::HUSK));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_SkeletonHorse, this, IDS_DESC_SKELETON_HORSE,
                                             IDS_SKELETON_HORSE, EntityType::SKELETON_HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_ZombieHorse, this, IDS_DESC_ZOMBIE_HORSE, IDS_ZOMBIE_HORSE,
                                             EntityType::ZOMBIE_HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Painting, this, IDS_DESC_PICTURE,
                                                                       IDS_ITEM_PAINTING, EntityType::PAINTING));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_ItemFrame, this, IDS_DESC_ITEMFRAME, IDS_ITEM_ITEMFRAME,
                                             EntityType::ITEM_FRAME));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MinecartRideable, this, IDS_DESC_MINECART,
                                             IDS_ITEM_MINECART, EntityType::MINECART_RIDEABLE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MinecartChest, this, IDS_DESC_MINECARTWITHCHEST,
                                             IDS_ITEM_MINECART_CHEST, EntityType::MINECART_CHEST));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MinecartFurnace, this, IDS_DESC_MINECARTWITHFURNACE,
                                             IDS_ITEM_MINECART_FURNACE, EntityType::MINECART_FURNACE));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MinecartHopper, this, IDS_DESC_MINECART_HOPPER,
                                             IDS_ITEM_MINECART_HOPPER, EntityType::MINECART_HOPPER));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_MinecartTnt, this, IDS_DESC_MINECART_TNT,
                                             IDS_ITEM_MINECART_TNT, EntityType::MINECART_TNT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Boat, this, IDS_DESC_BOAT,
                                                                       IDS_ITEM_BOAT_OAK, EntityType::BOAT));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_ArmorStand, this, IDS_DESC_ARMORSTAND, IDS_ARMORSTAND,
                                             EntityType::ARMOR_STAND));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_LeashFenceKnotEntity, this, IDS_DESC_LEAD, IDS_ITEM_LEAD,
                                             EntityType::LEASH_FENCE_KNOT_ENTITY));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_EnderCrystal, this, IDS_DESC_ITEM_END_CRYSTAL,
                                             IDS_ITEM_END_CRYSTAL, EntityType::ENDER_CRYSTAL));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Llama, this, IDS_DESC_LLAMA, IDS_LLAMA, EntityType::LLAMA));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Parrot, this, IDS_DESC_PARROT,
                                                                       IDS_PARROT, EntityType::PARROT));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Mule, this, IDS_DESC_MULE, IDS_MULE, EntityType::MULE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Donkey, this, IDS_DESC_DONKEY,
                                                                       IDS_DONKEY, EntityType::DONKEY));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Vex, this, IDS_DESC_VEX, IDS_VEX, EntityType::VEX));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_EvocationIllager, this, IDS_DESC_EVOCATION_ILLAGER,
                                             IDS_EVOCATION_ILLAGER, EntityType::EVOCATION_ILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_VindicationIllager, this, IDS_DESC_VINDICATION_ILLAGER,
                                             IDS_VINDICATION_ILLAGER, EntityType::VINDICATION_ILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_ElderGuardian, this, IDS_DESC_ELDER_GUARDIAN,
                                             IDS_ELDER_GUARDIAN, EntityType::ELDER_GUARDIAN));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_ZombieVillager, this, IDS_DESC_ZOMBIE_VILLAGER,
                                             IDS_ZOMBIE_VILLAGER, EntityType::ZOMBIE_VILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Turtle, this, IDS_DESC_TURTLE,
                                                                       IDS_TURTLE, EntityType::TURTLE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Dolphin, this, IDS_DESC_DOLPHIN,
                                                                       IDS_DOLPHIN, EntityType::DOLPHIN));
    addHintIfIncomplete(eTutorial_State_Gameplay,
                        new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_COD, IDS_COD, EntityType::FISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_SALMON,
                                                                       IDS_SALMON, EntityType::SALMON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_PUFFERFISH,
                                                                       IDS_PUFFERFISH, EntityType::PUFFERFISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_TROPICALFISH,
                                                                       IDS_TROPICALFISH, EntityType::TROPICAL_FISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Drowned, this, IDS_DESC_DROWNED,
                                                                       IDS_DROWNED, EntityType::DROWNED));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Phantom, this, 0x1D435D3,
                                                                       IDS_PHANTOM, EntityType::PHANTOM));
    Item* shovelItems[] = {Items::woodenShovel, Items::stoneShovel, Items::ironShovel, Items::goldenShovel,
                           Items::diamondShovel};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new DiggerItemHint(eTutorial_Hint_Shovel, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_SHOVEL, shovelItems, 5));
    Item* axeItems[] = {Items::woodenAxe, Items::stoneAxe, Items::ironAxe, Items::goldenAxe, Items::diamondAxe};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new DiggerItemHint(eTutorial_Hint_Axe, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_HATCHET, axeItems, 5));
    Item* pickaxeItems[] = {Items::woodenPickaxe, Items::stonePickaxe, Items::ironPickaxe, Items::goldenPickaxe,
                            Items::diamondPickaxe};
    addHintIfIncomplete(
        eTutorial_State_Gameplay,
        new DiggerItemHint(eTutorial_Hint_Pickaxe, this, IDS_TUTORIAL_HINT_DIGGER_ITEM_PICKAXE, pickaxeItems, 5));
    addHint(eTutorial_State_CraftingMenu, new TutorialHint(eTutorial_Hint_General, this, 0xDBD723F6,
                                                           TutorialHint::HINT_CREATE_ITEM_SELECTED, true, true, -1));
    addHint(eTutorial_State_CraftingTableMenu,
            new TutorialHint(eTutorial_Hint_General, this, 0xDBD723F6, TutorialHint::HINT_CREATE_ITEM_SELECTED, true,
                             true, -1));
}

void Tutorial::addTask(eTutorial_State state, TutorialTask* task)
{
    if (state == eTutorial_State_Gameplay)
    {
        tasks.push_back(task);
    }
    stateTasks[state].push_back(task);
}

unsigned int Tutorial::AddTasks(bool enabledValue)
{
    ProcedureCompoundTask* compound_0;
    ProcedureCompoundTask* compound_1;
    ProcedureCompoundTask* compound_2;
    ProcedureCompoundTask* compound_3;
    ProcedureCompoundTask* compound_4;
    ProcedureCompoundTask* compound_5;
    ProcedureCompoundTask* compound_6;
    ProcedureCompoundTask* compound_7;
    ProcedureCompoundTask* compound_9;
    ProcedureCompoundTask* compound_10;
    ProcedureCompoundTask* compound_11;
    if (!enabledValue && isStateCompleted(eTutorial_State_InventoryMenu))
        goto check_state_0xE;
create_state_1:
    compound_0 = new ProcedureCompoundTask(this);
    compound_0->AddTask(new ChoiceTask(this, 0xB77FC340, 0x9E1E8969, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x4)));
    compound_0->AddTask(new InfoTask(this, 0xC18416E0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0x99B85F72, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0x1A28A6CA, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0xEC1649D5, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_InventoryMenu, compound_0);
    if (!enabledValue)
        goto check_state_0xE;
create_state_0xE:
    compound_1 = new ProcedureCompoundTask(this);
    compound_1->AddTask(new ChoiceTask(this, 0x25598D4, 0x46859B85, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x14)));
    compound_1->AddTask(new InfoTask(this, 0x11BE3D54, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x5F3FE8A6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x94985DDE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x3F6BE86, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x893A28E1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_CreativeInventoryMenu, compound_1);
    if (!enabledValue)
        goto check_state_2;
create_state_2:
    compound_2 = new ProcedureCompoundTask(this);
    compound_2->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_CRAFT_OVERVIEW, 0x7F0C21BA, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x5)));
    compound_2->AddTask(new InfoTask(this, 0x27A2B969, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_CREATE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_CRAFT_TABLE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_INVENTORY, 0xA0D0169C, false, 2, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0xB135E36, 0xFC8730E4, false, 2, eTelemetryChallenges_None));
    compound_2->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_CRAFT_INGREDIENTS, 0xAF244F78, false, 2, eTelemetryChallenges_None));
    addTask(eTutorial_State_CraftingMenu, compound_2);
    if (!enabledValue)
        goto check_state_4;
create_state_4:
    compound_3 = new ProcedureCompoundTask(this);
    compound_3->AddTask(new ChoiceTask(this, 0x6CE6D19, 0x4C0CF470, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x6)));
    compound_3->AddTask(new InfoTask(this, 0x9BB6558D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_3->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_FUELS, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_3->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_FURNACE_INGREDIENTS, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FurnaceMenu, compound_3);
    if (!enabledValue)
        goto check_state_0x12;
create_state_0x12:
    compound_4 = new ProcedureCompoundTask(this);
    compound_4->AddTask(new ChoiceTask(this, 0x9253A77, 0x9E500506, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0xF)));
    compound_4->AddTask(new InfoTask(this, 0x96BEBCB3, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xAED3450F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xC02F0030, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xFB48ACEA, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xFCFE0427, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_BrewingMenu, compound_4);
    if (!enabledValue)
        goto check_state_0x14;
create_state_0x14:
    compound_5 = new ProcedureCompoundTask(this);
    compound_5->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_OVERVIEW, 0x690D3FD7, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x15)));
    compound_5->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_ENCHANTING_MENU_START, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0xCA85FE57, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x244CF0FE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x731167E6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x697EA620, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_EnchantingMenu, compound_5);
    if (!enabledValue)
        goto check_state_0x1B;
create_state_0x1B:
    compound_6 = new ProcedureCompoundTask(this);
    compound_6->AddTask(new ChoiceTask(this, 0xD381DD9, 0x645B6F8, true, 0, 1, eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x2E)));
    compound_6->AddTask(new InfoTask(this, 0xC9DE21A, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x4F577027, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x5E84B4A1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_ANVIL_MENU_ENCHANT, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x190AE839, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x67CD9B1F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x51ED81D1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_AnvilMenu, compound_6);
    if (!enabledValue)
        goto check_state_0x19;
create_state_0x19:
    compound_7 = new ProcedureCompoundTask(this);
    compound_7->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_OVERVIEW, 0x789B40B7, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x30)));
    compound_7->AddTask(new InfoTask(this, 0x54091CCD, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0xB7242ECF, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0x7C03162F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0x7B8B7B8F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(
        new InfoTask(this, IDS_TUTORIAL_TASK_TRADING_MENU_TRADE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_TradingMenu, compound_7);
    if (!enabledValue)
        goto check_state_0x1D;
create_state_0x1D:
    addTask(eTutorial_State_Horse,
            new HorseChoiceTask(this, IDS_TUTORIAL_TASK_HORSE_OVERVIEW, IDS_TUTORIAL_TASK_DONKEY_OVERVIEW,
                                IDS_TUTORIAL_TASK_MULE_OVERVIEW, IDS_TUTORIAL_TASK_LLAMA_OVERVIEW, 0x7595B01B, true, 0,
                                1, eTutorial_CompletionAction_CompleteStateAndConstraints,
                                static_cast<ETelemetryChallenges>(0x32)));
    addTask(eTutorial_State_Horse,
            new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_INTRO, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xCE6D12A7, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xFB9D6DB1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x86C858BB, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x32AEB3B, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse,
            new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_SADDLES, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse,
            new InfoTask(this, IDS_TUTORIAL_TASK_HORSE_SADDLEBAGS, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x3ED848DD, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_0x1E;
create_state_0x1E:
    compound_9 = new ProcedureCompoundTask(this);
    compound_9->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_HORSE_MENU_OVERVIEW, 0xEB9A80F7, true, 0, 1,
                                       eTutorial_CompletionAction_CompleteState,
                                       static_cast<ETelemetryChallenges>(0x33)));
    compound_9->AddTask(new InfoTask(this, 0xE4582E6F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_9->AddTask(new InfoTask(this, 0xAC55BC2F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_9->AddTask(new InfoTask(this, 0xAD1032A9, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_HorseMenu, compound_9);
    if (!enabledValue)
        goto check_state_0x24;
create_state_0x24:
    compound_10 = new ProcedureCompoundTask(this);
    compound_10->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_FIREWORK_MENU_OVERVIEW, 0xB0ED8A95, true, 0, 1,
                                        eTutorial_CompletionAction_CompleteState,
                                        static_cast<ETelemetryChallenges>(0x35)));
    compound_10->AddTask(new InfoTask(this, 0x8C159652, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xFE1D058D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x22299E7D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xE44F42D0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x13FD0693, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x859377B3, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xD0C84AA4, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x36518124, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x306CF96D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FireworksMenu, compound_10);
    if (!enabledValue)
        goto check_state_0x22;
create_state_0x22:
    compound_11 = new ProcedureCompoundTask(this);
    compound_11->AddTask(new ChoiceTask(this, IDS_TUTORIAL_TASK_BEACON_MENU_OVERVIEW, 0xBA245C8, true, 0, 1,
                                        eTutorial_CompletionAction_CompleteState,
                                        static_cast<ETelemetryChallenges>(0x37)));
    compound_11->AddTask(new InfoTask(this, 0x934C3015, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_11->AddTask(new InfoTask(this, 0xF87CF280, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_11->AddTask(new InfoTask(this, 0xC6447326, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_BeaconMenu, compound_11);
    if (!enabledValue)
        goto check_state_5;
create_state_5:
    addTask(eTutorial_State_RidingMinecart, new ChoiceTask(this, IDS_TUTORIAL_TASK_MINECART_OVERVIEW, 0x16DFAE83, true,
                                                           0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints,
                                                           static_cast<ETelemetryChallenges>(0x8)));
    addTask(eTutorial_State_RidingMinecart,
            new InfoTask(this, IDS_TUTORIAL_TASK_MINECART_RAILS, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_RidingMinecart, new InfoTask(this, IDS_TUTORIAL_TASK_MINECART_POWERED_RAILS, 0x3F9463F,
                                                         true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_RidingMinecart,
            new InfoTask(this, 0x5C8E319F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_6;
create_state_6:
    addTask(eTutorial_State_RidingBoat, new ChoiceTask(this, IDS_TUTORIAL_TASK_BOAT_OVERVIEW, 0x811A4288, true, 0, 1,
                                                       eTutorial_CompletionAction_CompleteStateAndConstraints,
                                                       static_cast<ETelemetryChallenges>(0x9)));
    addTask(eTutorial_State_RidingBoat, new InfoTask(this, 0xE6A88FAF, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_7;
create_state_7:
    addTask(eTutorial_State_Fishing, new ChoiceTask(this, IDS_TUTORIAL_TASK_FISHING_OVERVIEW, 0x1B284F34, true, 0, 1,
                                                    eTutorial_CompletionAction_CompleteStateAndConstraints,
                                                    static_cast<ETelemetryChallenges>(0x7)));
    addTask(eTutorial_State_Fishing,
            new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_CAST, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Fishing,
            new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_FISH, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Fishing,
            new InfoTask(this, IDS_TUTORIAL_TASK_FISHING_USES, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_8;
create_state_8:
    addTask(eTutorial_State_NameTag, new InfoTask(this, IDS_TUTORIAL_TASK_NAMETAG_OVERVIEW, 0x3F9463F, true, 0,
                                                  eTelemetryChallenges_Challenge58));
    if (!enabledValue)
        goto check_state_9;
    goto add_state_9;
check_state_0xE:
    if (!isStateCompleted(eTutorial_State_CreativeInventoryMenu))
        goto create_state_0xE;
check_state_2:
    if (!isStateCompleted(eTutorial_State_CraftingMenu))
        goto create_state_2;
check_state_4:
    if (!isStateCompleted(eTutorial_State_FurnaceMenu))
        goto create_state_4;
check_state_0x12:
    if (!isStateCompleted(eTutorial_State_BrewingMenu))
        goto create_state_0x12;
check_state_0x14:
    if (!isStateCompleted(eTutorial_State_EnchantingMenu))
        goto create_state_0x14;
check_state_0x1B:
    if (!isStateCompleted(eTutorial_State_AnvilMenu))
        goto create_state_0x1B;
check_state_0x19:
    if (!isStateCompleted(eTutorial_State_TradingMenu))
        goto create_state_0x19;
check_state_0x1D:
    if (!isStateCompleted(eTutorial_State_Horse))
        goto create_state_0x1D;
check_state_0x1E:
    if (!isStateCompleted(eTutorial_State_HorseMenu))
        goto create_state_0x1E;
check_state_0x24:
    if (!isStateCompleted(eTutorial_State_FireworksMenu))
        goto create_state_0x24;
check_state_0x22:
    if (!isStateCompleted(eTutorial_State_BeaconMenu))
        goto create_state_0x22;
check_state_5:
    if (!isStateCompleted(eTutorial_State_RidingMinecart))
        goto create_state_5;
check_state_6:
    if (!isStateCompleted(eTutorial_State_RidingBoat))
        goto create_state_6;
check_state_7:
    if (!isStateCompleted(eTutorial_State_Fishing))
        goto create_state_7;
check_state_8:
    if (!isStateCompleted(eTutorial_State_NameTag))
        goto create_state_8;
check_state_9:
    if (!isStateCompleted(eTutorial_State_Bed))
        goto add_state_9;
    goto add_state_f;
add_state_9:
    addTask(eTutorial_State_Bed, new ChoiceTask(this, IDS_TUTORIAL_TASK_BED_OVERVIEW, 0x2C631D03, true, 0, 1,
                                                eTutorial_CompletionAction_CompleteStateAndConstraints,
                                                static_cast<ETelemetryChallenges>(0xA)));
    addTask(eTutorial_State_Bed, new InfoTask(this, 0x54D286F0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Bed, new InfoTask(this, 0xA62DA153, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (enabledValue)
        return 0;
add_state_f:
    if (!isStateCompleted(eTutorial_State_FoodBar))
    {
        addTask(eTutorial_State_FoodBar, new ChoiceTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_OVERVIEW, 0x2B6F086C, true, 0,
                                                        1, eTutorial_CompletionAction_CompleteStateAndConstraints,
                                                        static_cast<ETelemetryChallenges>(0xD)));
        addTask(eTutorial_State_FoodBar,
                new InfoTask(this, IDS_TUTORIAL_TASK_FOOD_BAR_DEPLETE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_FoodBar, new InfoTask(this, 0xD756C6A6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
        addTask(eTutorial_State_FoodBar, new InfoTask(this, 0x642C1BB4, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    }
    return 0;
}


unsigned int Tutorial::setMessage(PopupMessageDetails* details)
{
    if (details != NULL)
    {
        if (!details->allowFade && messageState == currentState && details->isSameContent(currentMessage) != 0
            && (!details->unknown
                || (static_cast<unsigned int>(visibleTime) + gTutorialMessageRepeatTime > GetTickCount()
                    && details->unknown)))
        {
            delete details;
            return 0;
        }
    }

    if (details == NULL || ((details->titleId == -1 || details->titleId == -2) && details->title.empty()))
    {
        if (currentMessage != NULL && currentMessage->titleId != -1 && currentMessage->titleId != -2)
        {
            _TutorialPopupInfo info;
            info.scene = currentScene;
            info.tutorial = this;
            gConsoleUIController.SetTutorialDescription(playerId, &info);
        }
    }
    else
    {
        messageState = currentState;
        if (!details->displayed)
            visibleTime = static_cast<int>(GetTickCount());

        std::wstring title;
        if (details->title.empty())
        {
            boost::unordered_map<int, TutorialMessage*>::iterator found = messages.find(details->titleId);
            if (details->force || found == messages.end() || found->second == NULL)
                title = std::wstring(L10N::GetString(details->titleId));
            else
                title = std::wstring(found->second->getMessageForDisplay());
        }
        else
            title = details->title;
#ifdef MINECRAFT_PC
        if (details->titleId == static_cast<int>(IDS_TUTORIAL_TASK_MOVE) && PcInputGlyphSet() == ePcGlyphSet_Keyboard)
        {
            std::wstring::size_type end = title.find_last_not_of(L" \t\r\n");
            if (end != std::wstring::npos && title[end] == L'.')
                title.insert(end, L" (use all four keys)");
            else
                title += L" (use all four keys)";
        }
#endif

        if (!details->message.empty())
            title += details->message;
        else if (details->messageId != -1)
        {
            boost::unordered_map<int, TutorialMessage*>::iterator found = messages.find(details->messageId);
            if (found != messages.end() && found->second != NULL)
                title += found->second->getMessageForDisplay();
        }

        std::wstring description;
        if (!details->itemDescription.empty())
            description = details->itemDescription;
        else if (details->promptId != -1 && details->promptId != -2)
            description = std::wstring(L10N::GetString(details->promptId));

        _TutorialPopupInfo info;
        info.scene = currentScene;
        info.title = title.c_str();
        info.description = description.c_str();
        info.icon = details->itemInstance;
        info.iconItemId = details->itemId;
        info.iconAuxValue = details->itemAuxValue;
        info.showTitle = details->repeat;
        info.showExitScreenshot = details->unknown;
        info.displayTime = details->state;
        info.tutorial = this;
        gConsoleUIController.SetTutorialDescription(playerId, &info);
    }

    delete currentMessage;
    currentMessage = details;
    return 1;
}

void Tutorial::setCompleted(int completionId)
{
    unsigned int index = static_cast<unsigned int>(-1);
    for (std::vector<int>::const_iterator it = gTutorialCompletionIds.begin(); it < gTutorialCompletionIds.end(); ++it)
    {
        ++index;
        if (*it == completionId)
            break;
    }
    if (index < 0x200)
    {
        GameSettings::SetTutorialCompletionFlag(playerId, static_cast<int>(index));
    }
}

void Tutorial::setHintCompleted(eTutorial_Hint hint)
{
    setCompleted((int)hint);
}

void Tutorial::setHintCompleted(TutorialHint* hint)
{
    if (hint->hint != eTutorial_Hint_General)
    {
        setHintCompleted(hint->hint);
        std::vector<TutorialHint*>& hints = stateHints[currentState];
        hints.erase(std::find(hints.begin(), hints.end(), hint));
        delete hint;
    }
}

int Tutorial::setMessage(TutorialHint* hint, PopupMessageDetails* details)
{
    bool eligible = true;
    if (!enabled
        && (GameSettings::GetGameSettings(playerId, eGameSetting_Hints) == 0
            || GameSettings::GetGameSettings(playerId, eGameSetting_DisplayHUD) == 0))
    {
        eligible = false;
    }
    unsigned int now = GetTickCount();
    int result = 0;
    if (details != NULL)
    {
        if (details->allowFade || eligible)
        {
            bool delayed = false;
            if (details->isItem)
            {
                if (!popupPending)
                {
                    if (now - static_cast<unsigned int>(visibleTime) <= gTutorialMinimumDisplayTime)
                    {
                        delayed = true;
                    }
                }
                else if (now - static_cast<unsigned int>(messageTime) <= gTutorialHintRepeatTime)
                {
                    delayed = true;
                }
            }
            if (!delayed)
            {
                result = static_cast<int>(setMessage(details));
                if (result != 0)
                {
                    messageTime = static_cast<int>(now);
                    popupPending = true;
                    if (hint != NULL)
                    {
                        setHintCompleted(hint);
                    }
                }
            }
        }
    }
    return result;
}

unsigned int Tutorial::isSelectedItemState()
{
    switch (currentState)
    {
    case eTutorial_State_Gameplay:
    case eTutorial_State_Fishing:
    case eTutorial_State_NameTag:
        return 1;
    default:
        return 0;
    }
}

void Tutorial::tick()
{
    if (!started)
    {
        unsigned int now = GetTickCount();
        if (startTime == 0)
        {
            startTime = static_cast<int>(now);
            return;
        }
        if (now - static_cast<unsigned int>(startTime) <= gTutorialStartDelay)
            return;
        started = true;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (overridesGameHostOptions)
    {
        if (gameHostOptionsOverridden)
        {
            if (completed)
            {
                MinecraftServer::s_setDayTimeAtEndOfTick = true;
                MinecraftServer::s_setDayTime = gTutorialDayTime;
                minecraft->mLevel->setDayTime(gTutorialDayTime);
                gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);
                gameHostOptionsOverridden = false;
            }
        }
        else if (!completed)
        {
            MinecraftServer::s_setDayTimeAtEndOfTick = true;
            MinecraftServer::s_setDayTime = gTutorialDayTime;
            minecraft->mLevel->setDayTime(gTutorialDayTime);
            gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 0);
            gameHostOptionsOverridden = true;
        }
    }

    if (popupVisible)
    {
        bool taskChanged = false;
        if (!uiControllerSet)
        {
            gConsoleUIController.SetTutorial(playerId, this);
            uiControllerSet = true;
        }
        else if (splitScreen != (gApp.GetLocalPlayerCount() > 1))
        {
            gConsoleUIController.SetTutorial(playerId, this);
        }

        if (!gConsoleUIController.IsPauseMenuDisplayed(playerId))
        {
            if (tutorialHidden)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState) && currentTask[currentState] != NULL)
                {
                    gConsoleUIController.SetTutorialVisible(playerId, true);
                    visibleTime = static_cast<int>(GetTickCount());
                }
                tutorialHidden = false;
            }

            if (!popupPending)
            {
                const bool hintsEnabled = enabled || GameSettings::GetGameSettings(playerId, eGameSetting_Hints) != 0;
                if ((gameModeChanged && gameMode == eTutorial_GameMode_Fireworks) || hintsEnabled)
                {
                    std::vector<TutorialHint*>& hints = stateHints[currentState];
                    for (std::vector<TutorialHint*>::iterator it = hints.begin(); it != hints.end(); ++it)
                    {
                        TutorialHint* hint = *it;
                        int hintId = hint->tick();
                        if (gameModeChanged && !gConsoleUIController.GetMenuDisplayed(playerId))
                            hintId = static_cast<int>(hint->onGameModeChange(gameMode));
                        if (hintId != -1)
                        {
                            hintShown = true;
                            gameModeChanged = false;
                            PopupMessageDetails* details = new PopupMessageDetails();
                            details->titleId = hintId;
                            details->repeat = hint->allowFade() != 0;
                            details->allowFade = true;
                            details->state = static_cast<int>(hint->getMinimumDisplayTime());
                            setMessage(hint, details);
                            break;
                        }
                    }
                }
            }

            if (!tasksChecked)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState))
                {
                    std::vector<TutorialTask*>& tasks = stateTasks[currentState];
                    bool checkTask = true;
                    std::vector<TutorialTask*>::iterator it = tasks.begin();
                    while (!tasks.empty() && it != tasks.end())
                    {
                        TutorialTask* task = *it;
                        if (!checkTask && !task->isPreCompletionEnabled())
                        {
                            ++it;
                            continue;
                        }
                        checkTask = false;
                        if ((!task->ShowMinimumTime()
                             || (task->hasBeenActivated()
                                 && static_cast<unsigned int>(visibleTime) + gTutorialMinimumDisplayTime
                                        < GetTickCount()))
                            && task->isCompleted())
                        {
                            const eTutorial_CompletionAction action = task->getCompletionAction();
                            it = tasks.erase(it);
                            delete task;
                            if (!tasks.empty()
                                && (action == eTutorial_CompletionAction_CompleteState
                                    || action == eTutorial_CompletionAction_CompleteStateAndConstraints))
                            {
                                if (action == eTutorial_CompletionAction_CompleteStateAndConstraints)
                                    gameModeConstraints->MoveDelayedConstraints(currentState, 0);
                                for (std::vector<TutorialTask*>::iterator rest = tasks.begin(); rest != tasks.end();
                                     ++rest)
                                {
                                    delete *rest;
                                }
                                tasks.clear();
                                it = tasks.end();
                                if (currentState == eTutorial_State_Gameplay)
                                    gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            }
                            else if (!tasks.empty() && action == eTutorial_CompletionAction_CompleteStateExceptLastTask)
                            {
                                TutorialTask* lastTask = tasks.at(tasks.size() - 1);
                                tasks.pop_back();
                                for (std::vector<TutorialTask*>::iterator rest = tasks.begin(); rest != tasks.end();
                                     ++rest)
                                {
                                    delete *rest;
                                }
                                tasks.clear();
                                tasks.push_back(lastTask);
                                it = tasks.begin();
                                if (currentState == eTutorial_State_Gameplay)
                                    gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            }

                            taskChanged = true;
                            if (tasks.empty())
                            {
                                setStateCompleted(currentState);
                                currentTask[currentState] = NULL;
                            }
                            else
                            {
                                currentTask[currentState] = tasks.front();
                                currentTask[currentState]->setAsCurrentTask(true);
                                if (currentTask[currentState]->isPreCompletionEnabled())
                                    checkTask = true;
                            }
                        }
                        else
                        {
                            ++it;
                            if (task->ShowMinimumTime() && task->hasBeenActivated()
                                && static_cast<unsigned int>(visibleTime) + gTutorialMinimumDisplayTime
                                       < GetTickCount())
                            {
                                task->setShownForMinimumTime();
                                if (!popupPending)
                                {
                                    PopupMessageDetails* details = new PopupMessageDetails();
                                    details->titleId = task->getDescriptionId();
                                    details->messageId = task->getPromptId();
                                    details->repeat = task->AllowFade();
                                    details->displayed = true;
                                    setMessage(details);
                                }
                            }
                        }
                    }

                    if (currentTask[currentState] == NULL && !tasks.empty())
                    {
                        currentTask[currentState] = tasks.front();
                        currentTask[currentState]->setAsCurrentTask(true);
                        taskChanged = true;
                    }
                }

                if (!tasksChecked && (taskChanged || stateChanged))
                {
                    bool allTasksDone = true;
                    for (int state = 0; state < eTutorial_State_Max; ++state)
                    {
                        if (!stateTasks[state].empty())
                        {
                            allTasksDone = false;
                            break;
                        }
                        if (state == eTutorial_State_Gameplay)
                        {
                            completed = true;
                            gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            Minecraft::GetInstance()->playerLeftTutorial(playerId);
                        }
                    }
                    if (allTasksDone)
                        tasksChecked = true;
                }
            }

            bool showTaskMessage = gameModeConstraints->HasFailedConstraint(currentState) || taskChanged
                                   || stateChanged;
            if (!showTaskMessage && !popupPending && !gameModeConstraints->HasFailedConstraint(currentState)
                && currentTask[currentState] != NULL
                && (currentMessage == NULL || currentTask[currentState]->getDescriptionId() != currentMessage->titleId))
            {
                showTaskMessage = true;
            }

            if (showTaskMessage)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState))
                {
                    TutorialTask* task = currentTask[currentState];
                    if (task == NULL)
                    {
                        setMessage(static_cast<PopupMessageDetails*>(NULL));
                    }
                    else
                    {
                        PopupMessageDetails* details = new PopupMessageDetails();
                        details->titleId = task->getDescriptionId();
                        details->messageId = task->getPromptId();
                        details->repeat = task->AllowFade();
                        setMessage(details);
                        messageRepeatCount = currentTask[currentState]->repeatMessage;
                    }
                }
                else
                {
                    PopupMessageDetails* details = new PopupMessageDetails();
                    details->titleId = gameModeConstraints->GetFailedConstraint(currentState)->messageId;
                    details->repeat = false;
                    setMessage(details);
                }
            }

            if (popupPending && static_cast<unsigned int>(visibleTime) + gTutorialVisibleTime < GetTickCount())
                popupPending = false;

            if (!gameModeConstraints->HasFailedConstraint(currentState))
            {
                TutorialTask* task = currentTask[currentState];
                if (task != NULL && messageRepeatCount != 0
                    && static_cast<unsigned int>(visibleTime) + messageRepeatCount * gTutorialMessageRepeatTime
                           < GetTickCount())
                {
                    PopupMessageDetails* details = new PopupMessageDetails();
                    details->titleId = task->getDescriptionId();
                    details->messageId = task->getPromptId();
                    details->repeat = task->AllowFade();
                    details->unknown = true;
                    setMessage(details);
                    ++messageRepeatCount;
                    if (static_cast<unsigned int>(messageRepeatCount) > 1)
                        messageRepeatCount = 1;
                }
            }
            stateChanged = false;

            if (currentState != eTutorial_State_Gameplay && stateTasks[currentState].empty()
                && (isSelectedItemState() || !gConsoleUIController.GetMenuDisplayed(playerId)))
            {
                completedStates.push_back(currentState);
                changeTutorialState(eTutorial_State_Gameplay, NULL);
            }
            return;
        }
    }

    TutorialTask* task = currentTask[currentState];
    if (task != NULL
        && (!task->AllowFade() || static_cast<unsigned int>(visibleTime) + gTutorialVisibleTime > GetTickCount()))
    {
        tutorialHidden = true;
    }
    gConsoleUIController.SetTutorialVisible(playerId, false);
}

unsigned char Tutorial::isStateCompleted(eTutorial_State state)
{
    return getCompleted(state);
}

void Tutorial::setStateCompleted(eTutorial_State state)
{
    setCompleted((int)state);
}

void Tutorial::changeTutorialState(eTutorial_State state, UIScene* scene)
{
    if (state == currentState)
    {
        if (scene == NULL)
            gConsoleUIController.RemoveInteractSceneReference(playerId, currentScene);
        currentScene = scene;
        return;
    }

    if (!enabled && GameSettings::GetGameSettings(playerId, eGameSetting_Hints) == 0)
        return;

    if (state != eTutorial_State_Gameplay && stateTasks[state].empty()
        && !gConsoleUIController.GetMenuDisplayed(playerId))
        return;

    if (currentTask[currentState] != NULL)
    {
        if (currentTask[currentState]->isCompleted())
        {
            std::vector<TutorialTask*>& tasks = stateTasks[currentState];
            tasks.erase(std::find(tasks.begin(), tasks.end(), currentTask[currentState]));
            if (stateTasks[currentState].empty())
            {
                currentTask[currentState] = NULL;
            }
            else
            {
                currentTask[currentState] = stateTasks[currentState].front();
                currentTask[currentState]->setAsCurrentTask(true);
            }
        }
        if (currentTask[currentState] != NULL)
            currentTask[currentState]->onStateChange(state);
    }
    setMessage((PopupMessageDetails*)NULL);

    if (scene == NULL)
        gConsoleUIController.RemoveInteractSceneReference(playerId, currentScene);
    currentScene = scene;

    if (currentState != state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin(); it < stateTasks[state].end(); ++it)
            (*it)->onStateChange(state);
        currentState = state;
        gameModeConstraints->SetCurrentState(state);
        popupPending = false;
        stateChanged = true;
    }
}

void Tutorial::setMessage(const std::wstring& messageValue, int displayTimeValue, int priorityValue)
{
    PopupMessageDetails* details = new PopupMessageDetails();
    details->title.assign(messageValue);
    details->displayTime = displayTimeValue;
    details->priority = priorityValue;
    details->allowFade = true;
    setMessage(details);
}

void Tutorial::showTutorialPopup(bool show)
{
    popupVisible = show;
    if (!show)
    {
        TutorialTask* task = currentTask[currentState];
        if (task != NULL && (!task->AllowFade() || (unsigned int)visibleTime + gTutorialVisibleTime > GetTickCount()))
        {
            tutorialHidden = true;
        }
        gConsoleUIController.SetTutorialVisible(playerId, false);
    }
}

void Tutorial::useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos, bool cancelled)
{
    std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
    while (it < stateTasks[currentState].end())
    {
        (*it)->useItemOn(level, item, pos, cancelled);
        ++it;
    }
}

void Tutorial::useItemOn(not_null_ptr<ItemInstance> item, bool cancelled)
{
    std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
    while (it < stateTasks[currentState].end())
    {
        (*it)->useItem(item, cancelled);
        ++it;
    }
}

void Tutorial::completeUsingItem(not_null_ptr<ItemInstance> item)
{
    std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
    while (it < stateTasks[currentState].end())
    {
        (*it)->completeUsingItem(item);
        ++it;
    }
    if (currentState != eTutorial_State_Gameplay)
    {
        std::vector<TutorialTask*>::iterator it = stateTasks[eTutorial_State_Gameplay].begin();
        while (it < stateTasks[eTutorial_State_Gameplay].end())
        {
            (*it)->completeUsingItem(item);
            ++it;
        }
    }
}

void Tutorial::startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state)
{
    int result;
    std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
    while (it < stateHints[currentState].end())
    {
        TutorialHint* hint = *it;
        result = hint->startDestroyBlock(item, state);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
        ++it;
    }
}

void Tutorial::destroyBlock(Block* block)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it < stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        int result = static_cast<int>(hint->destroyBlock(block));
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
    }
}

void Tutorial::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it < stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        int result = hint->attack(player->mInventory->getSelected(), entity);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
    }
}

void Tutorial::itemDamaged(not_null_ptr<ItemInstance> item)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it < stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        int result = hint->itemDamaged(item);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
    }
}

void Tutorial::handleUIInput(int input)
{
    if (popupPending || currentTask[currentState] == NULL)
        return;
    currentTask[currentState]->handleUIInput(input);
}

void Tutorial::createItemSelected(not_null_ptr<ItemInstance> item, bool selected)
{
    std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
    while (it < stateHints[currentState].end())
    {
        TutorialHint* hint = *it;
        int result = hint->createItemSelected(item, selected);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
        ++it;
    }
}

void Tutorial::onCrafted(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
        while (it < stateTasks[state].end())
        {
            (*it)->onCrafted(item);
            ++it;
        }
    }
}

void Tutorial::onEquip(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
        while (it < stateTasks[state].end())
        {
            (*it)->onEquip(item);
            ++it;
        }
    }
}

void Tutorial::onTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
        while (it < stateTasks[state].end())
        {
            (*it)->onTakeFromAnvil(item);
            ++it;
        }
    }
}

void Tutorial::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount)
{
    if (!popupPending)
    {
        std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
        while (it < stateHints[currentState].end())
        {
            if ((*it)->onTake(item) != 0)
                break;
            ++it;
        }
    }
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
        while (it < stateTasks[state].end())
        {
            (*it)->onTake(item, itemCount, exactCount);
            ++it;
        }
    }
}

unsigned int Tutorial::onSelectedItemChanged(not_null_ptr<ItemInstance> item)
{
    if (isSelectedItemState() == 0)
        return 0;
    if (item->isEmpty())
    {
        changeTutorialState(eTutorial_State_Gameplay, NULL);
        return 0;
    }
    if (item->getItem() == Items::fishingRod)
    {
        changeTutorialState(eTutorial_State_Fishing, NULL);
        return 0;
    }
    if (item->getItem() == Items::nameTag)
    {
        changeTutorialState(eTutorial_State_NameTag, NULL);
        return 0;
    }
    changeTutorialState(eTutorial_State_Gameplay, NULL);
    return 0;
}

void Tutorial::onLookAt(const BlockState* state)
{
    if (!popupPending)
    {
        for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
             it < stateHints[currentState].end(); ++it)
        {
            if ((*it)->onLookAt(state) != 0)
            {
                break;
            }
        }
        if (currentState == eTutorial_State_Gameplay && state->getBlock() == Blocks::bed)
        {
            changeTutorialState(eTutorial_State_Bed, NULL);
        }
    }
}

void Tutorial::onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!popupPending && state->getBlock() == Blocks::skull)
    {
        for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
             it < stateHints[currentState].end(); ++it)
        {
            if ((*it)->onLookAtSkull(level, pos, state) != 0)
                break;
        }
    }
}

void Tutorial::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (!popupPending)
    {
        std::vector<TutorialHint*>::iterator hintIt = stateHints[currentState].begin();
        while (hintIt < stateHints[currentState].end())
        {
            TutorialHint* hint = *hintIt;
            if (hint->onLookAtEntity(entity) != 0)
                break;
            ++hintIt;
        }
        if (currentState == eTutorial_State_Gameplay)
        {
            if (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::ZOMBIE_HORSE, entity->GetType()) && (!enabled || completed))
            {
                if (!boost::static_pointer_cast<AbstractHorse>(entity)->isBaby())
                    changeTutorialState(eTutorial_State_Horse, NULL);
            }
        }
        for (std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
             it != stateTasks[currentState].end(); ++it)
        {
            (*it)->onLookAtEntity(entity);
        }
    }
}

void Tutorial::onRideEntity(boost::shared_ptr<Entity> entity)
{
    if (currentState == eTutorial_State_Gameplay)
    {
        unsigned long long type = entity->GetType();
        if (type == EntityType::BOAT)
        {
            changeTutorialState(eTutorial_State_RidingBoat, NULL);
        }
        else if (type == EntityType::MINECART_RIDEABLE)
        {
            changeTutorialState(eTutorial_State_RidingMinecart, NULL);
        }
    }
    for (std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
         it != stateTasks[currentState].end(); ++it)
    {
        (*it)->onRideEntity(entity);
    }
}

void Tutorial::onEffectChanged(MobEffect* effect, bool added)
{
    for (std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
         it < stateTasks[currentState].end(); ++it)
    {
        (*it)->onEffectChanged(effect, added);
    }
}

void Tutorial::onGameModeChange(eTutorial_GameMode newGameMode)
{
    if (gameMode == newGameMode)
    {
        return;
    }
    if (gameMode == eTutorial_GameMode_Fireworks && !hintShown)
    {
        return;
    }
    gameMode = newGameMode;
    gameModeChanged = true;
    if (newGameMode != eTutorial_GameMode_Fireworks)
    {
        return;
    }
    hintShown = false;
}

void Tutorial::staticCtor()
{
    gTutorialCompletionIds.push_back(1);
    gTutorialCompletionIds.push_back(2);
    gTutorialCompletionIds.push_back(3);
    gTutorialCompletionIds.push_back(4);
    gTutorialCompletionIds.push_back(5);
    gTutorialCompletionIds.push_back(6);
    gTutorialCompletionIds.push_back(7);
    gTutorialCompletionIds.push_back(9);
    gTutorialCompletionIds.push_back(10);
    gTutorialCompletionIds.push_back(11);
    gTutorialCompletionIds.push_back(12);
    gTutorialCompletionIds.push_back(13);
    gTutorialCompletionIds.push_back(14);
    gTutorialCompletionIds.push_back(15);
    gTutorialCompletionIds.push_back(16);
    gTutorialCompletionIds.push_back(17);
    gTutorialCompletionIds.push_back(18);
    gTutorialCompletionIds.push_back(19);
    gTutorialCompletionIds.push_back(45);
    gTutorialCompletionIds.push_back(46);
    gTutorialCompletionIds.push_back(47);
    gTutorialCompletionIds.push_back(48);
    gTutorialCompletionIds.push_back(49);
    gTutorialCompletionIds.push_back(50);
    gTutorialCompletionIds.push_back(51);
    gTutorialCompletionIds.push_back(52);
    gTutorialCompletionIds.push_back(53);
    gTutorialCompletionIds.push_back(54);
    gTutorialCompletionIds.push_back(55);
    gTutorialCompletionIds.push_back(56);
    gTutorialCompletionIds.push_back(57);
    gTutorialCompletionIds.push_back(58);
    gTutorialCompletionIds.push_back(59);
    gTutorialCompletionIds.push_back(60);
    gTutorialCompletionIds.push_back(61);
    gTutorialCompletionIds.push_back(62);
    gTutorialCompletionIds.push_back(63);
    gTutorialCompletionIds.push_back(65);
    gTutorialCompletionIds.push_back(66);
    gTutorialCompletionIds.push_back(67);
    gTutorialCompletionIds.push_back(68);
    gTutorialCompletionIds.push_back(69);
    gTutorialCompletionIds.push_back(70);
    gTutorialCompletionIds.push_back(72);
    gTutorialCompletionIds.push_back(71);
    gTutorialCompletionIds.push_back(73);
    gTutorialCompletionIds.push_back(74);
    gTutorialCompletionIds.push_back(75);
    gTutorialCompletionIds.push_back(76);
    gTutorialCompletionIds.push_back(77);
    gTutorialCompletionIds.push_back(78);
    gTutorialCompletionIds.push_back(79);
    gTutorialCompletionIds.push_back(80);
    gTutorialCompletionIds.push_back(81);
    gTutorialCompletionIds.push_back(82);
    gTutorialCompletionIds.push_back(83);
    gTutorialCompletionIds.push_back(84);
    gTutorialCompletionIds.push_back(85);
    gTutorialCompletionIds.push_back(86);
    gTutorialCompletionIds.push_back(87);
    gTutorialCompletionIds.push_back(88);
    gTutorialCompletionIds.push_back(89);
    gTutorialCompletionIds.push_back(90);
    gTutorialCompletionIds.push_back(91);
    gTutorialCompletionIds.push_back(92);
    gTutorialCompletionIds.push_back(93);
    gTutorialCompletionIds.push_back(94);
    gTutorialCompletionIds.push_back(95);
    gTutorialCompletionIds.push_back(96);
    gTutorialCompletionIds.push_back(97);
    gTutorialCompletionIds.push_back(98);
    gTutorialCompletionIds.push_back(99);
    gTutorialCompletionIds.push_back(101);
    gTutorialCompletionIds.push_back(102);
    gTutorialCompletionIds.push_back(103);
    gTutorialCompletionIds.push_back(104);
    gTutorialCompletionIds.push_back(105);
    gTutorialCompletionIds.push_back(107);
    gTutorialCompletionIds.push_back(106);
    gTutorialCompletionIds.push_back(108);
    gTutorialCompletionIds.push_back(109);
    gTutorialCompletionIds.push_back(110);
    gTutorialCompletionIds.push_back(111);
    gTutorialCompletionIds.push_back(112);
    gTutorialCompletionIds.push_back(113);
    gTutorialCompletionIds.push_back(114);
    gTutorialCompletionIds.push_back(115);
    gTutorialCompletionIds.push_back(116);
    gTutorialCompletionIds.push_back(117);
    gTutorialCompletionIds.push_back(118);
    gTutorialCompletionIds.push_back(119);
    gTutorialCompletionIds.push_back(120);
    gTutorialCompletionIds.push_back(121);
    gTutorialCompletionIds.push_back(122);
    gTutorialCompletionIds.push_back(123);
    gTutorialCompletionIds.push_back(124);
    gTutorialCompletionIds.push_back(125);
    gTutorialCompletionIds.push_back(126);
    gTutorialCompletionIds.push_back(127);
    gTutorialCompletionIds.push_back(128);
    gTutorialCompletionIds.push_back(129);
    gTutorialCompletionIds.push_back(130);
    gTutorialCompletionIds.push_back(131);
    gTutorialCompletionIds.push_back(132);
    gTutorialCompletionIds.push_back(133);
    gTutorialCompletionIds.push_back(134);
    gTutorialCompletionIds.push_back(135);
    gTutorialCompletionIds.push_back(136);
    gTutorialCompletionIds.push_back(137);
    gTutorialCompletionIds.push_back(138);
    gTutorialCompletionIds.push_back(139);
    gTutorialCompletionIds.push_back(140);
    gTutorialCompletionIds.push_back(141);
    gTutorialCompletionIds.push_back(142);
    gTutorialCompletionIds.push_back(143);
    gTutorialCompletionIds.push_back(144);
    gTutorialCompletionIds.push_back(145);
    gTutorialCompletionIds.push_back(146);
    gTutorialCompletionIds.push_back(147);
    gTutorialCompletionIds.push_back(148);
    gTutorialCompletionIds.push_back(164);
    gTutorialCompletionIds.push_back(165);
    gTutorialCompletionIds.push_back(166);
    gTutorialCompletionIds.push_back(167);
    gTutorialCompletionIds.push_back(168);
    gTutorialCompletionIds.push_back(169);
    gTutorialCompletionIds.push_back(170);
    gTutorialCompletionIds.push_back(171);
    gTutorialCompletionIds.push_back(172);
    gTutorialCompletionIds.push_back(173);
    gTutorialCompletionIds.push_back(174);
    gTutorialCompletionIds.push_back(175);
    gTutorialCompletionIds.push_back(176);
    gTutorialCompletionIds.push_back(177);
    gTutorialCompletionIds.push_back(178);
    gTutorialCompletionIds.push_back(179);
    gTutorialCompletionIds.push_back(180);
    gTutorialCompletionIds.push_back(181);
    gTutorialCompletionIds.push_back(182);
    gTutorialCompletionIds.push_back(183);
    gTutorialCompletionIds.push_back(184);
    gTutorialCompletionIds.push_back(185);
    gTutorialCompletionIds.push_back(186);
    gTutorialCompletionIds.push_back(187);
    gTutorialCompletionIds.push_back(212);
    gTutorialCompletionIds.push_back(213);
    gTutorialCompletionIds.push_back(214);
    gTutorialCompletionIds.push_back(215);
    gTutorialCompletionIds.push_back(216);
    gTutorialCompletionIds.push_back(217);
    gTutorialCompletionIds.push_back(218);
    gTutorialCompletionIds.push_back(219);
    gTutorialCompletionIds.push_back(220);
    gTutorialCompletionIds.push_back(221);
    gTutorialCompletionIds.push_back(222);
    gTutorialCompletionIds.push_back(223);
    gTutorialCompletionIds.push_back(224);
    gTutorialCompletionIds.push_back(225);
    gTutorialCompletionIds.push_back(226);
    gTutorialCompletionIds.push_back(227);
    gTutorialCompletionIds.push_back(228);
    gTutorialCompletionIds.push_back(229);
    gTutorialCompletionIds.push_back(230);
    gTutorialCompletionIds.push_back(231);
    gTutorialCompletionIds.push_back(232);
    gTutorialCompletionIds.push_back(233);
    gTutorialCompletionIds.push_back(234);
    gTutorialCompletionIds.push_back(235);
    gTutorialCompletionIds.push_back(236);
    gTutorialCompletionIds.push_back(237);
    gTutorialCompletionIds.push_back(238);
    gTutorialCompletionIds.push_back(239);
    gTutorialCompletionIds.push_back(240);
    gTutorialCompletionIds.push_back(241);
    gTutorialCompletionIds.push_back(242);
    gTutorialCompletionIds.push_back(243);
    gTutorialCompletionIds.push_back(244);
    gTutorialCompletionIds.push_back(245);
    gTutorialCompletionIds.push_back(246);
    gTutorialCompletionIds.push_back(247);
    gTutorialCompletionIds.push_back(248);
    gTutorialCompletionIds.push_back(249);
    gTutorialCompletionIds.push_back(250);
    gTutorialCompletionIds.push_back(251);
    gTutorialCompletionIds.push_back(252);
    gTutorialCompletionIds.push_back(253);
    gTutorialCompletionIds.push_back(254);
    gTutorialCompletionIds.push_back(255);
    gTutorialCompletionIds.push_back(256);
    gTutorialCompletionIds.push_back(257);
    gTutorialCompletionIds.push_back(258);
    gTutorialCompletionIds.push_back(259);
    gTutorialCompletionIds.push_back(260);
    gTutorialCompletionIds.push_back(261);
    gTutorialCompletionIds.push_back(262);
    gTutorialCompletionIds.push_back(263);
    gTutorialCompletionIds.push_back(264);
    gTutorialCompletionIds.push_back(265);
    gTutorialCompletionIds.push_back(266);
    gTutorialCompletionIds.push_back(267);
    gTutorialCompletionIds.push_back(268);
    gTutorialCompletionIds.push_back(269);
    gTutorialCompletionIds.push_back(270);
    gTutorialCompletionIds.push_back(271);
    gTutorialCompletionIds.push_back(272);
    gTutorialCompletionIds.push_back(273);
    gTutorialCompletionIds.push_back(274);
    gTutorialCompletionIds.push_back(275);
    gTutorialCompletionIds.push_back(276);
    gTutorialCompletionIds.push_back(277);
    gTutorialCompletionIds.push_back(278);
    gTutorialCompletionIds.push_back(279);
    gTutorialCompletionIds.push_back(280);
    gTutorialCompletionIds.push_back(281);
    gTutorialCompletionIds.push_back(282);
    gTutorialCompletionIds.push_back(283);
    gTutorialCompletionIds.push_back(284);
    gTutorialCompletionIds.push_back(285);
    gTutorialCompletionIds.push_back(286);
    gTutorialCompletionIds.push_back(287);
    gTutorialCompletionIds.push_back(288);
    gTutorialCompletionIds.push_back(289);
    gTutorialCompletionIds.push_back(290);
    gTutorialCompletionIds.push_back(291);
    gTutorialCompletionIds.push_back(292);
    gTutorialCompletionIds.push_back(293);
    gTutorialCompletionIds.push_back(294);
    gTutorialCompletionIds.push_back(295);
    gTutorialCompletionIds.push_back(296);
    gTutorialCompletionIds.push_back(297);
    gTutorialCompletionIds.push_back(386);
    gTutorialCompletionIds.push_back(387);
    gTutorialCompletionIds.push_back(388);
    gTutorialCompletionIds.push_back(389);
    gTutorialCompletionIds.push_back(390);
    gTutorialCompletionIds.push_back(391);
    gTutorialCompletionIds.push_back(392);
    gTutorialCompletionIds.push_back(393);
    gTutorialCompletionIds.push_back(394);
    gTutorialCompletionIds.push_back(395);
    gTutorialCompletionIds.push_back(396);
    gTutorialCompletionIds.push_back(397);
    gTutorialCompletionIds.push_back(398);
    gTutorialCompletionIds.push_back(20);
    gTutorialCompletionIds.push_back(21);
    gTutorialCompletionIds.push_back(22);
    gTutorialCompletionIds.push_back(23);
    gTutorialCompletionIds.push_back(24);
    gTutorialCompletionIds.push_back(25);
    gTutorialCompletionIds.push_back(26);
    gTutorialCompletionIds.push_back(27);
    gTutorialCompletionIds.push_back(28);
    gTutorialCompletionIds.push_back(30);
    gTutorialCompletionIds.push_back(32);
    gTutorialCompletionIds.push_back(188);
    gTutorialCompletionIds.push_back(190);
    gTutorialCompletionIds.push_back(191);
    gTutorialCompletionIds.push_back(192);
    gTutorialCompletionIds.push_back(298);
    gTutorialCompletionIds.push_back(299);
    gTutorialCompletionIds.push_back(300);
    gTutorialCompletionIds.push_back(301);
    gTutorialCompletionIds.push_back(302);
    gTutorialCompletionIds.push_back(303);
    gTutorialCompletionIds.push_back(304);
    gTutorialCompletionIds.push_back(305);
    gTutorialCompletionIds.push_back(306);
    gTutorialCompletionIds.push_back(307);
    gTutorialCompletionIds.push_back(34);
    gTutorialCompletionIds.push_back(36);
    gTutorialCompletionIds.push_back(29);
    gTutorialCompletionIds.push_back(31);
    gTutorialCompletionIds.push_back(33);
    gTutorialCompletionIds.push_back(35);
    gTutorialCompletionIds.push_back(189);
    gTutorialCompletionIds.push_back(8);
    gTutorialCompletionIds.push_back(193);
    gTutorialCompletionIds.push_back(194);
    gTutorialCompletionIds.push_back(195);
    gTutorialCompletionIds.push_back(309);
    gTutorialCompletionIds.push_back(310);
    gTutorialCompletionIds.push_back(311);
    gTutorialCompletionIds.push_back(312);
    gTutorialCompletionIds.push_back(313);
    gTutorialCompletionIds.push_back(308);
    {
        gTutorialCompletionIds.push_back(320);
        gTutorialCompletionIds.push_back(321);
        gTutorialCompletionIds.push_back(322);
        gTutorialCompletionIds.push_back(323);
        gTutorialCompletionIds.push_back(324);
        gTutorialCompletionIds.push_back(325);
        gTutorialCompletionIds.push_back(326);
        gTutorialCompletionIds.push_back(327);
        gTutorialCompletionIds.push_back(330);
        gTutorialCompletionIds.push_back(333);
        gTutorialCompletionIds.push_back(334);
        gTutorialCompletionIds.push_back(335);
        gTutorialCompletionIds.push_back(336);
        gTutorialCompletionIds.push_back(337);
        gTutorialCompletionIds.push_back(338);
        gTutorialCompletionIds.push_back(339);
        gTutorialCompletionIds.push_back(340);
        gTutorialCompletionIds.push_back(341);
        gTutorialCompletionIds.push_back(342);
    }
    gTutorialCompletionIds.push_back(196);
    gTutorialCompletionIds.push_back(100);
    gTutorialCompletionIds.push_back(64);
    gTutorialCompletionIds.push_back(314);
    gTutorialCompletionIds.push_back(315);
    gTutorialCompletionIds.push_back(316);
    gTutorialCompletionIds.push_back(317);
    gTutorialCompletionIds.push_back(318);
    gTutorialCompletionIds.push_back(319);
    gTutorialCompletionIds.push_back(149);
    gTutorialCompletionIds.push_back(150);
    gTutorialCompletionIds.push_back(151);
    gTutorialCompletionIds.push_back(152);
    gTutorialCompletionIds.push_back(153);
    gTutorialCompletionIds.push_back(197);
    gTutorialCompletionIds.push_back(198);
    gTutorialCompletionIds.push_back(199);
    gTutorialCompletionIds.push_back(200);
    gTutorialCompletionIds.push_back(38);
    gTutorialCompletionIds.push_back(39);
    gTutorialCompletionIds.push_back(154);
    gTutorialCompletionIds.push_back(155);
    gTutorialCompletionIds.push_back(156);
    gTutorialCompletionIds.push_back(157);
    gTutorialCompletionIds.push_back(158);
    gTutorialCompletionIds.push_back(159);
    gTutorialCompletionIds.push_back(160);
    gTutorialCompletionIds.push_back(161);
    gTutorialCompletionIds.push_back(162);
    gTutorialCompletionIds.push_back(163);
    gTutorialCompletionIds.push_back(201);
    gTutorialCompletionIds.push_back(202);
    gTutorialCompletionIds.push_back(203);
    gTutorialCompletionIds.push_back(204);
    gTutorialCompletionIds.push_back(205);
    gTutorialCompletionIds.push_back(206);
    gTutorialCompletionIds.push_back(207);
    gTutorialCompletionIds.push_back(208);
    gTutorialCompletionIds.push_back(209);
    gTutorialCompletionIds.push_back(210);
    gTutorialCompletionIds.push_back(211);
    gTutorialCompletionIds.push_back(350);
    gTutorialCompletionIds.push_back(351);
    gTutorialCompletionIds.push_back(352);
    gTutorialCompletionIds.push_back(353);
    gTutorialCompletionIds.push_back(354);
    gTutorialCompletionIds.push_back(355);
    gTutorialCompletionIds.push_back(356);
    gTutorialCompletionIds.push_back(357);
    gTutorialCompletionIds.push_back(358);
    gTutorialCompletionIds.push_back(359);
    gTutorialCompletionIds.push_back(360);
    gTutorialCompletionIds.push_back(361);
    gTutorialCompletionIds.push_back(362);
    gTutorialCompletionIds.push_back(363);
    gTutorialCompletionIds.push_back(364);
    gTutorialCompletionIds.push_back(365);
    gTutorialCompletionIds.push_back(366);
    gTutorialCompletionIds.push_back(367);
    gTutorialCompletionIds.push_back(368);
    gTutorialCompletionIds.push_back(369);
    gTutorialCompletionIds.push_back(370);
    gTutorialCompletionIds.push_back(371);
    gTutorialCompletionIds.push_back(372);
    gTutorialCompletionIds.push_back(373);
    gTutorialCompletionIds.push_back(374);
    gTutorialCompletionIds.push_back(375);
    gTutorialCompletionIds.push_back(376);
    gTutorialCompletionIds.push_back(377);
    gTutorialCompletionIds.push_back(378);
    gTutorialCompletionIds.push_back(379);
    gTutorialCompletionIds.push_back(380);
    gTutorialCompletionIds.push_back(381);
    gTutorialCompletionIds.push_back(383);
    gTutorialCompletionIds.push_back(384);
}
