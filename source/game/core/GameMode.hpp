#ifndef GAME_CORE_GAMEMODE_HPP
#define GAME_CORE_GAMEMODE_HPP

#include <string>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/GameType.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Recipe.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"

class Block;
class BlockItem;
class ClientboundChatPacket;
class CombatEntry;
class Direction;
class Entity;
class Item;
class ItemInstance;
class Inventory;
class Level;
class Minecraft;
class MiniGameDef;
class Player;

class GameMode
{
public:
    GameMode();
    virtual ~GameMode() {}

    virtual bool startDestroyBlock(const BlockPos& pos, const Direction* direction) = 0;
    virtual void tick() = 0;
    virtual InteractionResult::EInteractionResult useItem(boost::shared_ptr<Player> player, Level* level,
                                                          InteractionHand::EInteractionHand hand, bool simulate) = 0;
    virtual void SetGameType(const GameType* gameType) = 0;
    virtual const GameType* GetGameType() = 0;

    virtual bool CanRecordStatsAndAchievements();
    virtual bool MayDestroy(const BlockPos& pos);
    bool MayPlace(const BlockPos& pos, Block* block);
    virtual bool MayPlace(const BlockPos& pos, BlockItem* item);
    virtual bool MayUse(const BlockPos& pos, Item* item);
    virtual const MiniGameDef* GetMiniGame() = 0;
    virtual boost::shared_ptr<Player> GetPlayer() = 0;
    virtual Level* GetLevel() = 0;

    virtual void RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                              boost::shared_ptr<ClientboundChatPacket> packet);
    virtual void RecordDeath(boost::shared_ptr<Entity> entity);
    virtual void RecordItem(not_null_ptr<ItemInstance> item);
    virtual void RecordContainer(int type, int slot, int count, int data);
    virtual void RecordWalk(int distance, bool sprinting, bool sneaking);
    virtual void RecordSwim(int distance, bool sprinting);
    virtual void RecordClimb(int distance);
    virtual void RecordUseItem(not_null_ptr<ItemInstance> item);
    virtual void RecordJump();
    virtual void RecordGainPoints(int points);
    virtual void ResetTempPoints();
    virtual bool RecordSecondaryTargetReached(const std::wstring& name, int target,
                                              GlideRingGenerator::eGlideRingSize size);
    virtual bool RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third);
    virtual bool RecordLapCompleted();
    virtual void RecordThermalEntered();
    virtual bool RecordProgress(double first, double second, double third, double fourth);
    virtual void RecordSpawnedAtCheckpoint(int checkpoint);
    virtual void RecordBlockPlaced(const BlockPos& pos, int blockId);
    virtual void RecordBlockDestroyed(const BlockPos& pos, int blockId);
    virtual void RecordBlockPresent(const BlockPos& pos, int blockId);
    virtual void RecordLayerTime();
    virtual void RecordSnowballHit();
    virtual void RecordHitBySnowball();
    virtual void RecordLanding(const BlockPos& pos);
    virtual void OnMouseDown();

    virtual not_null_ptr<ItemInstance> handleCraftItem(int recipe, int category, boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<ItemInstance> handleStackedCraftItem(std::vector<std::pair<int, int> >& recipes,
                                                                   int category, boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<ItemInstance> handleCustomCraftItem(std::vector<int>& recipes,
                                                                  boost::shared_ptr<Player> player);

    bool isSurvival();
    bool isCreative();
    bool isAdventure();
    bool isSpectator();
    bool isLobby();
    bool AutoEquipEnabled();

    static void RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients, Inventory* inventory,
                                               not_null_ptr<ItemInstance> result, bool removeResult);
    static void RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients,
                                               boost::shared_ptr<Inventory> inventory,
                                               not_null_ptr<ItemInstance> result, bool removeResult);
    void UpdateCraftAchievements(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item, eAward award);
    static void RemoveCustomRecipeItemsFromInventory(int recipe, Inventory* inventory);
    static void RemoveCustomRecipeItemsFromInventory(int recipe, boost::shared_ptr<Inventory> inventory);

protected:
    Minecraft* mMinecraft;
    const GameType* mGameType;
};

#endif
