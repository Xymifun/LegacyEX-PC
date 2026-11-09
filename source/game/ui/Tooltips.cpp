#include "game/ui/Tooltips.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#ifdef MINECRAFT_PC
#include "game/core/L10N.hpp"
#endif
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
const int kTooltipsPerPlayer = 25;
const unsigned int kTooltipMapVote = 0x1a66548e;
const unsigned int kTooltipVote = 0x44c3150d;
const unsigned int kTooltipLaunch = 0x568a07be;
int gTooltips[4 * kTooltipsPerPlayer];
unsigned char gTooltipsDirty[4];
}

void Tooltips::staticCtor()
{
    int playerId;
    int action;

    for (playerId = 0; playerId < 4; ++playerId)
    {
        gTooltipsDirty[playerId] = 1;
        for (action = 0; action < 25; ++action)
            gTooltips[playerId * kTooltipsPerPlayer + action] = -1;
    }
}

void Tooltips::OverwriteTooltip(int playerId, EControllerActions action, int tooltip)
{
    if (gTooltips[playerId * kTooltipsPerPlayer + action - EControllerActions_Jump] == tooltip)
        return;

    gTooltipsDirty[playerId] = 1;
    gTooltips[playerId * kTooltipsPerPlayer + action - EControllerActions_Jump] = tooltip;
}

void Tooltips::ResetTooltip(int playerId, EControllerActions action)
{
    OverwriteTooltip(playerId, action, -1);
}

void Tooltips::SetInteractTooltips(boost::shared_ptr<MultiplayerLocalPlayer> player, MultiPlayerLevel* level,
                                   const HitResult* hitResult, InteractionResult::EInteractionResult interactionResult)
{
    static EControllerActions extraTooltipActions[5] = {
        EControllerActions_ConfirmReady, EControllerActions_Vote, EControllerActions_LookBehind,
        EControllerActions_RestartSection, EControllerActions_RestartRace};
    const int playerId = player->mUserIndex;

    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        ResetTooltip(playerId, EControllerActions_Crafting);
        ResetTooltip(playerId, EControllerActions_PickBlock);
    }
    else
    {
        OverwriteTooltip(playerId, EControllerActions_Crafting,
                         player->mAbilities.mInstabuild ? IDS_TOOLTIPS_CREATIVE : IDS_CONTROLS_CRAFTING);
        if (player->pickBlock(true) != 0)
            OverwriteTooltip(playerId, EControllerActions_PickBlock, IDS_CONTROLS_PICKBLOCK);
        else
            ResetTooltip(playerId, EControllerActions_PickBlock);
    }

    if (player->isUnderLiquid(Material::WATER) && !player->isPassenger())
        OverwriteTooltip(playerId, EControllerActions_Jump, static_cast<int>(IDS_TOOLTIPS_SWIMUP));
    else
        ResetTooltip(playerId, EControllerActions_Jump);

    if (player->isSleeping() && level != NULL)
    {
        ResetTooltip(playerId, EControllerActions_Action);
        OverwriteTooltip(playerId, EControllerActions_Use, IDS_TOOLTIPS_WAKEUP);
        return;
    }

    if (player->isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = player->getVehicle();
        const bool isMinecart = eTYPE_DERIVED_FROM(EntityType::MINECART, vehicle->GetType());
        if (isMinecart)
        {
            OverwriteTooltip(playerId, EControllerActions_Sneak, static_cast<int>(IDS_TOOLTIPS_EXIT));
        }
        else
        {
            const bool isBoat = eTYPE_DERIVED_FROM(EntityType::BOAT, vehicle->GetType());
            if (isBoat)
                OverwriteTooltip(playerId, EControllerActions_Sneak, static_cast<int>(IDS_TOOLTIPS_EXIT));
            else
                OverwriteTooltip(playerId, EControllerActions_Sneak, static_cast<int>(IDS_TOOLTIPS_DISMOUNT));
        }
    }
    else
    {
        ResetTooltip(playerId, EControllerActions_Sneak);
    }

    not_null_ptr<ItemInstance> item = player->getMainHandItem();
    if (hitResult != NULL)
    {
        switch (hitResult->mType)
        {
        case HitResult::BLOCK:
        {
            BlockPos pos = hitResult->getBlockPos();
            const Direction* face = hitResult->mDirection;
            const BlockState* state = level->getBlockState(pos);
            Block* block = state->getBlock();

            const bool useBlock = !player->isSneaking() && player->isAllowedToUse(block);
            const bool useItemOnBlock = player->isSneaking() && player->isAllowedToUse(block) && item->isEmpty();
            const bool canPlace = pos.y < level->getMaxBuildHeight() - 1
                                  || (face != Direction::UP && pos.y < level->getMaxBuildHeight());
            const bool canDestroy = Minecraft::GetInstance()->mGameModes[playerId]->MayDestroy(pos)
                                    && player->isAllowedToMine();

            BlockTooltipDataHolder blockData(player, level, item, state, pos, interactionResult, useBlock,
                                             useItemOnBlock, canDestroy, canPlace);
            int interactTooltip = static_cast<int>(block->GetInteractTooltip(blockData));
            if (interactTooltip == -1 && !item->isEmpty())
            {
                ItemTooltipDataHolder itemData(player, item, level, interactionResult);
                interactTooltip = static_cast<int>(item->getItem()->GetUseTooltip(itemData));
            }
            OverwriteTooltip(playerId, EControllerActions_Use, interactTooltip);
            OverwriteTooltip(playerId, EControllerActions_Action, static_cast<int>(block->GetHitTooltip(blockData)));
            break;
        }
        case HitResult::ENTITY:
        {
            EntityTooltipDataHolder entityData(player, item, player->isAllowedToHurtEntity(hitResult->mEntity),
                                               player->isAllowedToInteract(hitResult->mEntity));
            int interactTooltip = static_cast<int>(hitResult->mEntity->GetInteractTooltip(entityData));
            if (interactTooltip == -1 && !item->isEmpty() && !hitResult->mEntity->HideUseTooltips())
            {
                ItemTooltipDataHolder itemData(player, item, level, interactionResult);
                interactTooltip = static_cast<int>(item->getItem()->GetUseTooltip(itemData));
            }
            OverwriteTooltip(playerId, EControllerActions_Use, interactTooltip);
            OverwriteTooltip(playerId, EControllerActions_Action,
                             static_cast<int>(hitResult->mEntity->GetHitTooltip(entityData)));
            break;
        }
        case HitResult::MISS:
            if (item->isEmpty())
            {
                ResetTooltip(playerId, EControllerActions_Use);
                ResetTooltip(playerId, EControllerActions_Action);
            }
            else
            {
                ItemTooltipDataHolder itemData(player, item, level, interactionResult);
                OverwriteTooltip(playerId, EControllerActions_Use, item->getItem()->GetUseTooltip(itemData));
                ResetTooltip(playerId, EControllerActions_Action);
            }
            break;
        }
    }
    else
    {
        if (item->isEmpty())
        {
            ResetTooltip(playerId, EControllerActions_Use);
            ResetTooltip(playerId, EControllerActions_Action);
        }
        else
        {
            uint32_t allowed = player->isAllowedToUse(item);
            if (allowed != 0)
            {
                {
                    ItemTooltipDataHolder itemData(player, item, level, interactionResult);
                    OverwriteTooltip(playerId, EControllerActions_Use, item->getItem()->GetUseTooltip(itemData));
                }
                ResetTooltip(playerId, EControllerActions_Action);
            }
            else
            {
                ResetTooltip(playerId, EControllerActions_Use);
                ResetTooltip(playerId, EControllerActions_Action);
            }
        }
    }

    boost::unordered_map<int, int> extraTooltips(0xb);
#ifdef MINECRAFT_PC
    bool pcToggleGhost = false;
#endif
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        const bool isPrimaryPad = gProfile->GetPrimaryPad() == playerId;
        Minecraft* minecraft = Minecraft::GetInstance();
        ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
        if (player->GetGameType() == GameType::LOBBY)
        {
            extraTooltips[0x34] = gameMode->IsPlayerReady(player->mPlayerIndex)
                                      ? static_cast<int>(IDS_TOOLTIP_NOT_READY)
                                      : static_cast<int>(IDS_TOOLTIP_READY);

            std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
            gameMode->GetUniquePlaylist(playlist);
            if (playlist.size() > 1)
            {
                if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                {
                    extraTooltips[0x35] = kTooltipMapVote;
                }
                else if (isPrimaryPad && CGameNetworkManager::sInstance.IsHost() && minecraft != NULL
                         && minecraft->mLocalPlayers[playerId]->GetGameType() == GameType::LOBBY)
                {
                    extraTooltips[0x35] = IDS_MG_SELECT_MAP;
                }
            }
            else if (!Minecraft::InMiniGame(eMiniGameId_Battle, true)
                     && minecraft->GetMiniGame()->HasMultipleVariants())
            {
                if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                {
                    if (minecraft->GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                        extraTooltips[0x35] = kTooltipVote;
                }
                else if (isPrimaryPad && CGameNetworkManager::sInstance.IsHost() && minecraft != NULL
                         && minecraft->mLocalPlayers[playerId]->GetGameType() == GameType::LOBBY
                         && minecraft->GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                {
                    extraTooltips[0x35] = static_cast<int>(IDS_MG_SELECT_VARIANT);
                }
            }
        }

        if (Minecraft::InMiniGame(eMiniGameId_Tumble, false))
        {
            if (item->getItem() == Items::fireworks)
                OverwriteTooltip(playerId, EControllerActions_Use, kTooltipLaunch);
        }
        else if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            if (player->mThirdPersonView != 2)
                extraTooltips[0x38] = IDS_TOOLTIP_LOOK_BEHIND;

            int activePlayers = 0;
            ClientScoreboard* scoreboard = player->GetLevel()->GetClientScoreboard();
            for (int i = 0; i < 4; ++i)
            {
                if (minecraft->mLocalPlayers[i] != NULL
                    && (!minecraft->mLocalPlayers[i]->isSpectator()
                        || scoreboard->GetEndZoneEntered(minecraft->mLocalPlayers[i]->getSUID(), -1)))
                    ++activePlayers;
            }
            if (activePlayers == 1 && !player->isSpectator())
            {
#ifdef MINECRAFT_PC
                if (CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                    pcToggleGhost = true;
#endif
                ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
                if (CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0
                    && masterGameMode->IsRoundPlaying() && !scoreboard->GetEndZoneEntered(player->getSUID(), -1))
                {
                    extraTooltips[0x36] = IDS_TOOLTIP_RESTART_SECTION;
                    extraTooltips[0x37] = static_cast<int>(IDS_TOOLTIP_RESTART_RACE);
                }
            }
        }
    }

#ifdef MINECRAFT_PC
    if (pcToggleGhost)
        OverwriteTooltip(playerId, EControllerActions_CycleHeldItemRight, ePcString_ToggleGhost);
    else
        ResetTooltip(playerId, EControllerActions_CycleHeldItemRight);
#endif
    for (int i = 0; i < 5; ++i)
    {
        boost::unordered_map<int, int>::iterator it = extraTooltips.find(extraTooltipActions[i]);
        if (it != extraTooltips.end())
            OverwriteTooltip(playerId, extraTooltipActions[i], it->second);
        else
            ResetTooltip(playerId, extraTooltipActions[i]);
    }

    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    bool tamedOrNotHorse = true;
    if (vehicle != NULL)
    {
        const bool isHorse = eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, vehicle->GetType());
        if (isHorse)
            tamedOrNotHorse = boost::static_pointer_cast<AbstractHorse>(vehicle)->isTamed();
    }
    if (!tamedOrNotHorse)
    {
        ResetTooltip(playerId, EControllerActions_Inventory);
    }
    else if (Minecraft::InMiniGame(eMiniGameId_Any, true) && Minecraft::GetInstance()->GetMiniGame()->LockedInventory())
    {
        ResetTooltip(playerId, EControllerActions_Inventory);
    }
    else
    {
        OverwriteTooltip(playerId, EControllerActions_Inventory, static_cast<int>(IDS_CONTROLS_INVENTORY));
    }
}

void Tooltips::ResetAllTooltips(int playerId)
{
    int action;
    gTooltipsDirty[playerId] = 1;
    for (action = 0; action < 25; ++action)
        gTooltips[playerId * kTooltipsPerPlayer + action] = -1;
}

void Tooltips::ForceTooltipReload(int playerId)
{
    if (playerId == 0xff)
    {
        gTooltipsDirty[0] = 1;
        gTooltipsDirty[1] = 1;
        gTooltipsDirty[2] = 1;
        gTooltipsDirty[3] = 1;
        return;
    }

    gTooltipsDirty[playerId] = 1;
}

void Tooltips::ForceAllPlayerTooltipReset()
{
    ResetAllTooltips(0);
    ResetAllTooltips(1);
    ResetAllTooltips(2);
    ResetAllTooltips(3);
}

void Tooltips::DisplayTooltips(int playerId)
{
    if (gTooltipsDirty[playerId] != 0)
    {
        if (!gConsoleUIController.GetMenuDisplayed(playerId)
            && !gConsoleUIController.IsSceneInStack(playerId, EUIScene_ConnectingProgress))
        {
            gConsoleUIController.SetTooltips((unsigned int)playerId, &gTooltips[playerId * kTooltipsPerPlayer], false,
                                             false);
            gTooltipsDirty[playerId] = 0;
        }
    }
}
