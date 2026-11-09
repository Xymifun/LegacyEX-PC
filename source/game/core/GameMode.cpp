#include "game/core/GameMode.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/BannerRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/TridentItem.hpp"
#include "game/minigame/GrantPermissionsRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

GameMode::GameMode() : mGameType(NULL)
{
    mMinecraft = Minecraft::GetInstance();
}

bool GameMode::isSurvival()
{
    return mGameType->isSurvival();
}

bool GameMode::isCreative()
{
    return mGameType->isCreative()
           || (Minecraft::InMiniGame(eMiniGameId_Any, true) && GetPlayer()->mAbilities.mInstabuild);
}

bool GameMode::isAdventure()
{
    return mGameType == GameType::ADVENTURE;
}

bool GameMode::isSpectator()
{
    return mGameType == GameType::SPECTATOR;
}

bool GameMode::isLobby()
{
    return mGameType == GameType::LOBBY;
}

bool GameMode::CanRecordStatsAndAchievements()
{
    return mGameType != GameType::CREATIVE && !isSpectator();
}

bool GameMode::MayDestroy(const BlockPos& pos)
{
    if (isSpectator() && !Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
        return false;

    if (isCreative())
    {
        if (dynamic_cast<SwordItem*>(GetPlayer()->getMainHandItem()->getItem()) != NULL
            || dynamic_cast<TridentItem*>(GetPlayer()->getMainHandItem()->getItem()) != NULL)
            return false;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        LevelRuleset* ruleset;
        GrantPermissionsRuleDefinition* permissions;
        const MiniGameDef* miniGame;
        if (options != NULL && (ruleset = options->getRequiredGameRules()) != NULL)
        {
            permissions = static_cast<GrantPermissionsRuleDefinition*>(
                ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_GrantPermissions));
            miniGame = GetMiniGame();
            if (permissions != NULL)
            {
                if (permissions->canDestroy(GetPlayer(), GetLevel(),
                                            gApp.getLevelGenerationOptions()->getRequiredGameRules(), pos))
                    return !miniGame->EditActionsDisabled();
            }
            return miniGame->MayDestroy(Block::getId(GetLevel()->getBlock(pos)));
        }
        miniGame = GetMiniGame();
        return miniGame->MayDestroy(Block::getId(GetLevel()->getBlock(pos)));
    }

    boost::shared_ptr<Player> player = GetPlayer();
    if (!mGameType->isBlockPlacingRestricted())
        return true;
    if (mGameType == GameType::SPECTATOR)
        return false;
    if (player->mayBuild())
        return true;

    Block* block = GetLevel()->getBlock(pos);
    not_null_ptr<ItemInstance> selected = player->getMainHandItem();
    if (!selected->isEmpty())
    {
        if (selected->hasAdventureModeBreakTagForBlock(block))
            return true;
    }
    else if (!isAdventure() || (block != Blocks::chorus_flower && block != Blocks::chorus_plant))
    {
        return block->mMaterial->isDestroyedByHand();
    }
    return false;
}

bool GameMode::MayPlace(const BlockPos& pos, Block* block)
{
    BlockItem item(block);
    return MayPlace(pos, &item);
}

bool GameMode::MayPlace(const BlockPos& pos, BlockItem* item)
{
    if (isSpectator())
        return false;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        LevelRuleset* ruleset;
        GrantPermissionsRuleDefinition* permissions;
        bool canPlace = false;
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (options != NULL && (ruleset = options->getRequiredGameRules()) != NULL
            && (permissions = static_cast<GrantPermissionsRuleDefinition*>(
                    ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_GrantPermissions)))
                   != NULL)
        {
            canPlace = permissions->canPlace(GetPlayer(), GetLevel(),
                                             gApp.getLevelGenerationOptions()->getRequiredGameRules(), pos);
        }
        if (!canPlace)
        {
            const MiniGameDef* miniGame = GetMiniGame();
            return miniGame->MayPlace(Block::getId(item->getBlock()));
        }
    }
    return true;
}

bool GameMode::MayUse(const BlockPos& pos, Item* item)
{
    if (isSpectator())
        return false;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        LevelRuleset* ruleset;
        GrantPermissionsRuleDefinition* permissions;
        bool canUse = false;
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (options != NULL && (ruleset = options->getRequiredGameRules()) != NULL
            && (permissions = static_cast<GrantPermissionsRuleDefinition*>(
                    ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_GrantPermissions)))
                   != NULL)
        {
            canUse = permissions->canUse(GetPlayer(), GetLevel(),
                                         gApp.getLevelGenerationOptions()->getRequiredGameRules(), pos);
        }
        if (!canUse)
        {
            const MiniGameDef* miniGame = GetMiniGame();
            return miniGame->MayUse(Item::getId(item));
        }
    }
    return true;
}

bool GameMode::AutoEquipEnabled()
{
    return Minecraft::InMiniGame(eMiniGameId_Any, true);
}

void GameMode::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                            boost::shared_ptr<ClientboundChatPacket> packet)
{
}

void GameMode::RecordDeath(boost::shared_ptr<Entity> entity)
{
}

void GameMode::RecordItem(not_null_ptr<ItemInstance> item)
{
}

void GameMode::RecordContainer(int type, int slot, int count, int data)
{
}

void GameMode::RecordWalk(int distance, bool sprinting, bool sneaking)
{
}

void GameMode::RecordSwim(int distance, bool sprinting)
{
}

void GameMode::RecordClimb(int distance)
{
}

void GameMode::RecordUseItem(not_null_ptr<ItemInstance> item)
{
}

void GameMode::RecordJump()
{
}

void GameMode::RecordGainPoints(int points)
{
}

void GameMode::ResetTempPoints()
{
}

bool GameMode::RecordSecondaryTargetReached(const std::wstring& name, int target,
                                            GlideRingGenerator::eGlideRingSize size)
{
    return false;
}

bool GameMode::RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third)
{
    return false;
}

bool GameMode::RecordLapCompleted()
{
    return false;
}

void GameMode::RecordThermalEntered()
{
}

bool GameMode::RecordProgress(double first, double second, double third, double fourth)
{
    return false;
}

void GameMode::RecordSpawnedAtCheckpoint(int checkpoint)
{
}

void GameMode::RecordBlockPlaced(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordBlockDestroyed(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordBlockPresent(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordLayerTime()
{
}

void GameMode::RecordSnowballHit()
{
}

void GameMode::RecordHitBySnowball()
{
}

void GameMode::RecordLanding(const BlockPos& pos)
{
}

void GameMode::OnMouseDown()
{
}

void GameMode::RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients, Inventory* inventory,
                                              not_null_ptr<ItemInstance> result, bool removeResult)
{
    for (int ingredient = 0; ingredient < ingredients.numIngredients; ++ingredient)
    {
        for (int count = 0; count < ingredients.itemCounts[ingredient]; ++count)
        {
            not_null_ptr<ItemInstance> resource = ItemInstance::EMPTY;
            Item* item = Item::byId(ingredients.itemIds[ingredient]);
            int auxValue = ingredients.auxValues[ingredient];
            if (auxValue == 0 && item->getMaxDamage() > 0)
                auxValue = -1;

            CompoundTag* tag = ingredients.itemTags[ingredient];
            Item* consumedItem = NULL;
            if (!result->isEmpty() && result->getItem() == item)
            {
                if (!removeResult)
                {
                    if (inventory->removeResourcesWithTags(result) != 1)
                    {
                        if (auxValue == -1)
                            inventory->removeResource(item, false);
                        else
                            inventory->removeResource(item, auxValue, false);
                    }
                }
                removeResult = false;
            }
            else
            {
                if (tag == NULL)
                {
                    if (auxValue == -1)
                    {
                        resource = inventory->getResourceItem(item);
                        consumedItem = resource->getItem();
                        inventory->removeResource(item, true);
                    }
                    else
                    {
                        resource = inventory->getResourceItem(item, auxValue);
                        consumedItem = resource->getItem();
                        inventory->removeResource(item, auxValue, true);
                    }
                }
                else
                {
                    resource = inventory->getResourceItem(item, auxValue, tag);
                    consumedItem = resource->getItem();
                    inventory->removeResourcesWithTags(resource);
                }
            }

            if (resource->isEmpty() && consumedItem != NULL && consumedItem->hasCraftingRemainingItem())
                inventory->add(not_null_ptr<ItemInstance>(new ItemInstance(consumedItem->getCraftingRemainingItem())));
        }
    }
}

void GameMode::RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients,
                                              boost::shared_ptr<Inventory> inventory, not_null_ptr<ItemInstance> result,
                                              bool removeResult)
{
    RemoveRecipeItemsFromInventory(ingredients, inventory.get(), result, removeResult);
}

void GameMode::UpdateCraftAchievements(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item, eAward award)
{
    switch (Item::VALIDgetId(item->getItem()))
    {
    case 0x3a:
        player->awardStat(GenericStats::buildWorkbench(), GenericStats::param_buildWorkbench());
        break;
    case 0x10e:
        player->awardStat(GenericStats::buildPickaxe(), GenericStats::param_buildPickaxe());
        break;
    case 0x3d:
        player->awardStat(GenericStats::buildFurnace(), GenericStats::param_buildFurnace());
        break;
    case 0x122:
    case 0x123:
    case 0x124:
    case 0x125:
    case 0x126:
        player->awardStat(GenericStats::buildHoe(), GenericStats::param_buildHoe());
        break;
    case 0x129:
        player->awardStat(GenericStats::makeBread(), GenericStats::param_makeBread());
        break;
    case 0x162:
        player->awardStat(GenericStats::bakeCake(), GenericStats::param_bakeCake());
        break;
    case 0x112:
        player->awardStat(GenericStats::buildBetterPickaxe(), GenericStats::param_buildBetterPickaxe());
        break;
    case 0x10c:
        player->awardStat(GenericStats::buildSword(), GenericStats::param_buildSword());
        break;
    case 0x17:
        player->awardStat(GenericStats::dispenseWithThis(), GenericStats::param_dispenseWithThis());
        break;
    case 0x74:
        player->awardStat(GenericStats::enchantments(), GenericStats::param_enchantments());
        break;
    case 0x2f:
        player->awardStat(GenericStats::bookcase(), GenericStats::param_bookcase());
        break;
    }

    if (award != eAward_ChangeOfSheets)
        return;

    player->awardStat(GenericStats::changeOfSheets(), GenericStats::param_changeOfSheets());
}

not_null_ptr<ItemInstance> GameMode::handleCraftItem(int recipe, int category, boost::shared_ptr<Player> player)
{
    Recipes* recipes = Recipes::getInstance();
    Recipe::INGREDIENTS_REQUIRED& ingredients = recipes->getRecipeIngredientsArray(
        static_cast<Recipes::ERecipeCategory>(category))[recipe];
    not_null_ptr<ItemInstance> result = ingredients.recipe->assemble(boost::shared_ptr<CraftingContainer>());
    bool creativeDebug = gApp.DebugSettingsOn() && (player->mDebugOptions & 0x4000) != 0;
    result->onCraftedBy(player->mLevel, player, result->getCount());

    if (!creativeDebug)
        RemoveRecipeItemsFromInventory(ingredients, player->mInventory, ItemInstance::EMPTY, false);
    if (!player->mInventory->add(result))
        player->drop(result, false);
    UpdateCraftAchievements(player, result, eAward_None);
    return result;
}

boost::shared_ptr<ItemInstance> GameMode::handleStackedCraftItem(std::vector<std::pair<int, int> >& recipeStack,
                                                                 int category, boost::shared_ptr<Player> player)
{
    Recipe::INGREDIENTS_REQUIRED* ingredients = Recipes::getInstance()->getRecipeIngredientsArray(
        static_cast<Recipes::ERecipeCategory>(category));
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    bool firstRecipe = true;

    for (std::vector<std::pair<int, int> >::iterator it = recipeStack.begin(); it != recipeStack.end(); ++it)
    {
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[it->first];
        not_null_ptr<ItemInstance> source = ItemInstance::EMPTY;
        if (firstRecipe && it->second >= 0)
        {
            source = player->mInventory->getItem(it->second)->copy();
            source->setCount(1);
        }
        else
        {
            source = result;
        }

        bool preserveSource = true;
        if (!source->isEmpty())
        {
            boost::shared_ptr<CraftingContainer> crafting = required.recipe->generateExpectedCraftSlots();
            bool substituted = false;
            for (unsigned int row = 0; row < 3 && !substituted; ++row)
            {
                for (unsigned int column = 0; column < 3; ++column)
                {
                    if (!crafting->getItem(row, column)->isEmpty()
                        && crafting->getItem(row, column)->getItem() == source->getItem())
                    {
                        crafting->setItem(row + column * 3, source);
                        substituted = true;
                        break;
                    }
                }
            }

            result = required.recipe->assemble(crafting);
            if (result->getItem() == Items::banner)
            {
                result = BannerRecipes::mAddPatternRecipe->assemble(crafting);
                if (result->isEmpty())
                {
                    result = BannerRecipes::mDuplicateRecipe->assemble(crafting);
                    const DyeColor* color = BannerBlockEntity::getBaseColor(source);
                    result = not_null_ptr<ItemInstance>(BannerItem::makeBanner(color, NULL));
                    BannerBlockEntity::addTags(result, color->getItemData(), NULL);
                    preserveSource = false;
                }
            }
        }
        else
        {
            result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        }

        bool creativeDebug = gApp.DebugSettingsOn() && (player->mDebugOptions & 0x4000) != 0;
        result->onCraftedBy(player->mLevel, player, result->getCount());
        if (!creativeDebug)
            RemoveRecipeItemsFromInventory(required, player->mInventory, firstRecipe ? source : result,
                                           !firstRecipe || !preserveSource);
        UpdateCraftAchievements(player, result, eAward_None);
        firstRecipe = false;
    }

    if (!player->mInventory->add(result))
        player->drop(result, false);
    return result;
}

void GameMode::RemoveCustomRecipeItemsFromInventory(int slot, Inventory* inventory)
{
    if (slot >= 0)
        inventory->removeItem(slot, 1);
}

void GameMode::RemoveCustomRecipeItemsFromInventory(int slot, boost::shared_ptr<Inventory> inventory)
{
    RemoveCustomRecipeItemsFromInventory(slot, inventory.get());
}

boost::shared_ptr<ItemInstance> GameMode::handleCustomCraftItem(std::vector<int>& slots,
                                                                boost::shared_ptr<Player> player)
{
    boost::shared_ptr<CraftingContainer> crafting(new CraftingContainer(NULL, 3, 3));
    for (unsigned int slot = 0; slot < slots.size(); ++slot)
        crafting->setItem(slot, player->mInventory->getItem(slots[slot]));

    not_null_ptr<ItemInstance> result = Recipes::getInstance()->getItemFor(crafting, player->mLevel, NULL,
                                                                           Recipes::eRecipeCategory_Crafting);
    bool creativeDebug = gApp.DebugSettingsOn() && (player->mDebugOptions & 0x4000) != 0;
    result->onCraftedBy(player->mLevel, player, result->getCount());
    eAward award = Recipes::getInstance()->hasAchievementAttachedToRecipe(crafting, player->mLevel, NULL,
                                                                          Recipes::eRecipeCategory_Crafting);

    if (!creativeDebug)
    {
        for (unsigned int slot = 0; slot < slots.size(); ++slot)
            RemoveCustomRecipeItemsFromInventory(slots[slot], player->mInventory);
    }

    if (!player->mInventory->add(result))
        player->drop(result, false);
    UpdateCraftAchievements(player, result, award);
    return result;
}
