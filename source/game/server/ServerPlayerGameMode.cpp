#include "game/server/ServerPlayerGameMode.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/GameRulesInstance.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/TridentItem.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/item/Container.hpp"
#include "game/item/LockableContainer.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/DimensionType.hpp"
#ifdef MINECRAFT_PC
#include "game/world/block/Blocks.hpp"
#include "platform/pc/PcScripts.hpp"
#endif

ServerPlayerGameMode::ServerPlayerGameMode(Level* level, MasterGameMode* masterGameMode)
{
    mDestroying = false;
    mDestroyStartTick = 0;
    mDestroyPos = BlockPos::ZERO;
    mGameModeTicks = 0;
    mIsDestroying = false;
    mPreviousDestroyPos = BlockPos::ZERO;
    mLastSentProgress = -1;
    mLevel = level;
    mGameRules = NULL;
    mGameType = GameType::NOT_SET;
    mSecondaryDestroyStartTick = 0;
    mLayerTimeMs = 0;
    mDeaths = 0;
    mLastCheckpoint = 0;
    mMasterGameMode = masterGameMode;
}

ServerPlayerGameMode::~ServerPlayerGameMode()
{
    delete mGameRules;
}

void ServerPlayerGameMode::setLevel(ServerLevel* level)
{
    mLevel = level;
}
void ServerPlayerGameMode::updateGameMode(const GameType* gameType)
{
    if (mGameType == GameType::NOT_SET)
        mGameType = gameType;

    SetGameType(gameType);
}
void ServerPlayerGameMode::setGameRules(GameRulesInstance* gameRules)
{
    delete mGameRules;
    mGameRules = gameRules;
}
void ServerPlayerGameMode::nullifyGameRules()
{
    mGameRules = NULL;
}
void ServerPlayerGameMode::setLastCheckpoint(int checkpoint)
{
    mLastCheckpoint = checkpoint;
}
unsigned int ServerPlayerGameMode::getLastCheckpoint()
{
    return mLastCheckpoint;
}
void ServerPlayerGameMode::RestoreFromOld(ServerPlayerGameMode* oldMode)
{
    SetDeaths(oldMode->GetDeaths());
    setLastCheckpoint(oldMode->getLastCheckpoint());
}
boost::shared_ptr<ServerPlayer> ServerPlayerGameMode::getServerPlayer() const
{
    return boost::static_pointer_cast<ServerPlayer>(mPlayer->shared_from_this());
}
int ServerPlayerGameMode::superDestroyBlock(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    int result = state->getBlock()->playerWillDestroy(mLevel, pos, state, getServerPlayer()) ? 1 : 0;
    if (result == 0)
    {
        result = mLevel->removeBlock(pos) ? 1 : 0;
        if (result != 0)
            state->getBlock()->destroy(mLevel, pos, state);
    }
    return result;
}
int ServerPlayerGameMode::destroyBlock(const BlockPos& pos)
{
    if (isCreative())
    {
        if (dynamic_cast<SwordItem*>(mPlayer->getMainHandItem()->getItem()) != NULL)
            return 0;
    }

    if (!MayDestroy(pos))
        return 0;

    const BlockState* state = mLevel->getBlockState(pos);
    boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(pos);
    Block* block = state->getBlock();

    if (GetGameType()->isBlockPlacingRestricted())
    {
        if (GetGameType() != GameType::SPECTATOR
            || !Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
        {
            if (GetGameType() == GameType::SPECTATOR)
                return 0;
            if (!mPlayer->mayBuild())
            {
                not_null_ptr<ItemInstance> item = mPlayer->getMainHandItem();
                if (item->isEmpty())
                {
                    if (!block->mMaterial->isDestroyedByHand())
                        return 0;
                }
                else if (!item->hasAdventureModeBreakTagForBlock(block))
                    return 0;
            }
        }
    }

#ifdef MINECRAFT_PC
    if (!PcScriptsHostBlockBreak(mPlayer, static_cast<ServerLevel*>(mLevel), pos))
    {
        mPlayer->mConnection->send(
            boost::shared_ptr<ClientboundBlockUpdatePacket>(new ClientboundBlockUpdatePacket(mLevel, pos)));
        return 0;
    }
#endif

    boost::shared_ptr<ServerPlayer> serverPlayer = getServerPlayer();
    mLevel->levelEvent(serverPlayer, LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));

    int result = superDestroyBlock(pos);

    int dimensionId = mLevel->mDimension->getType()->getId();
    gMinecraftServer->getPlayers()->prioritiseBlockChanges(pos, dimensionId);

    RecordBlockDestroyed(pos, Block::getId(state->getBlock()));

    if (isCreative())
    {
        boost::shared_ptr<ClientboundBlockUpdatePacket> packet(new ClientboundBlockUpdatePacket(mLevel, pos));
        mPlayer->mConnection->send(packet);
    }
    else
    {
        not_null_ptr<ItemInstance> item = mPlayer->getMainHandItem();
        not_null_ptr<ItemInstance> toolCopy = item->isEmpty() ? ItemInstance::EMPTY : item->copy();

        bool canDestroy = mPlayer->canDestroy(state);
        if (!item->isEmpty())
            item->mineBlock(mLevel, state, pos, serverPlayer);

        if (result != 0 && canDestroy)
            block->playerDestroy(mLevel, serverPlayer, pos, state, blockEntity, toolCopy);
    }

    return result;
}
void ServerPlayerGameMode::stopDestroyBlock(const BlockPos& pos)
{
    if (pos.equals(mDestroyPos))
    {
        const BlockState* state = mLevel->getBlockState(pos);
        if (state->getMaterial() != Material::AIR)
        {
            mDestroying = false;
            mLevel->destroyBlockProgress(mPlayer->getId(), pos, -1);
            destroyBlock(pos);
        }
    }
}
void ServerPlayerGameMode::abortDestroyBlock()
{
    mDestroying = false;
    mLevel->destroyBlockProgress(mPlayer->getId(), mDestroyPos, -1);
}
InteractionResult::EInteractionResult ServerPlayerGameMode::useItemOn(boost::shared_ptr<Player> player, Level* level,
                                                                      not_null_ptr<ItemInstance> item,
                                                                      InteractionHand::EInteractionHand hand,
                                                                      const BlockPos& pos, const Direction* direction,
                                                                      float hitX, float hitY, float hitZ, bool simulate,
                                                                      bool* result)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();

    if (GetGameType() == GameType::SPECTATOR)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        if (blockEntity)
        {
            if (eTYPE_DERIVED_FROM(BlockEntityType::LOCKABLE_CONTAINER, blockEntity->GetType()))
            {
                boost::shared_ptr<LockableContainer> container = boost::dynamic_pointer_cast<LockableContainer>(
                    blockEntity);
                if (eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType())
                    && dynamic_cast<ChestBlock*>(block) != NULL)
                    container = static_cast<ChestBlock*>(block)->getContainer(level, pos);
                if (container)
                    player->openContainer(container);
                return InteractionResult::eInteractionResult_Success;
            }
            if (eTYPE_DERIVED_FROM(BlockEntityType::CONTAINER, blockEntity->GetType()))
            {
                player->openContainer(boost::dynamic_pointer_cast<Container>(blockEntity));
                return InteractionResult::eInteractionResult_Success;
            }
        }
        return InteractionResult::eInteractionResult_Pass;
    }

    bool skipBlockUse = false;
    if (player->isSneaking())
    {
        not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
        if (!mainHandItem->isEmpty())
        {
            skipBlockUse = true;
        }
        else
        {
            not_null_ptr<ItemInstance> offhandItem = player->getOffhandItem();
            if (!offhandItem->isEmpty())
                skipBlockUse = true;
        }
    }

    if (!skipBlockUse && player->isAllowedToUse(block))
    {
        if (!simulate)
        {
            if (block->use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false))
            {
                if (mGameRules != NULL)
                    mGameRules->onUseBlock(Block::getId(block), pos.x, pos.y, pos.z);
                return InteractionResult::eInteractionResult_Success;
            }
        }
        else if (block->TestUse())
        {
            return InteractionResult::eInteractionResult_Success;
        }
    }

    if (item->isEmpty() || !player->isAllowedToUse(item))
        return InteractionResult::eInteractionResult_Pass;

    BlockItem* blockItem = dynamic_cast<BlockItem*>(item->getItem());
    if (blockItem != NULL && dynamic_cast<CommandBlock*>(blockItem->getBlock()) != NULL
        && !player->canUseCommandBlocks())
        return InteractionResult::eInteractionResult_Fail;

#ifdef MINECRAFT_PC
    if (!simulate && blockItem != NULL)
    {
        const bool replaces = block == Blocks::snow_layer || block == Blocks::tallgrass || block == Blocks::sea_grass
                              || block->mayReplaceWithPlace(level, pos);
        const BlockPos placePos = replaces ? pos : pos.relative(direction);
        if (!PcScriptsHostBlockPlace(getServerPlayer().get(), static_cast<ServerLevel*>(level), placePos,
                                     blockItem->getBlock(), item->getAuxValue()))
            return InteractionResult::eInteractionResult_Fail;
    }
#endif
    const bool restoreCreativeItem = isCreative() && !isLobby();
    const int savedAuxValue = restoreCreativeItem ? item->getAuxValue() : 0;
    const int savedCount = restoreCreativeItem ? item->getCount() : 0;
    const InteractionResult::EInteractionResult itemResult = item->useOn(player, level, pos, hand, direction, hitX,
                                                                         hitY, hitZ, simulate);

    if (itemResult == InteractionResult::eInteractionResult_Success)
    {
        BlockItem* placedItem = dynamic_cast<BlockItem*>(item->getItem());
        if (placedItem != NULL)
            RecordBlockPlaced(pos, Block::getId(placedItem->getBlock()));
    }

    if (restoreCreativeItem)
    {
        const int dimensionId = level->mDimension->getType()->getId();
        gMinecraftServer->getPlayers()->prioritiseBlockChanges(pos, dimensionId);
        item->setAuxValue(savedAuxValue);
        item->setCount(savedCount);
    }

    return itemResult;
}
bool ServerPlayerGameMode::startDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    if (!mPlayer->isAllowedToMine() || !MayDestroy(pos))
    {
        if (Minecraft::InMiniGame(eMiniGameId_Lobby, true))
        {
            const BlockState* state = mLevel->getBlockState(pos);
            if (state->getMaterial() != Material::AIR)
            {
                boost::shared_ptr<ServerPlayer> player = getServerPlayer();
                state->getBlock()->attack(mLevel, pos, player);
            }
        }
        return false;
    }

    if (!isCreative())
    {
        const BlockState* state = mLevel->getBlockState(pos);
        Block* block = state->getBlock();

        bool spectatorParticipation = false;
        if (GetGameType() == GameType::SPECTATOR)
        {
            if (Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
                spectatorParticipation = true;
        }

        if (GetGameType()->isBlockPlacingRestricted() && !spectatorParticipation)
        {
            if (GetGameType() == GameType::SPECTATOR)
                return false;
            if (!mPlayer->mayBuild())
            {
                not_null_ptr<ItemInstance> heldItem = mPlayer->getMainHandItem();
                if (!heldItem->isEmpty())
                {
                    if (!heldItem->hasAdventureModeBreakTagForBlock(block))
                        return false;
                }
                else if (!state->getMaterial()->isDestroyedByHand())
                    return false;
            }
        }

        mLevel->extinguishFire(boost::shared_ptr<Player>(), pos, direction);
        mDestroyStartTick = mGameModeTicks;

        float destroyProgress = 1.0f;
        if (state->getMaterial() != Material::AIR)
        {
            boost::shared_ptr<ServerPlayer> player = getServerPlayer();
            block->attack(mLevel, pos, player);
            destroyProgress = state->getDestroyProgress(player, mPlayer->mLevel, pos);
        }
        if (spectatorParticipation)
            destroyProgress = 0.0f;
        if (state->getMaterial() == Material::AIR || destroyProgress < 1.0f)
        {
            mDestroying = true;
            mDestroyPos = pos;
            const int sentProgress = static_cast<int>(destroyProgress * 10.0f);
            mLevel->destroyBlockProgress(mPlayer->getId(), pos, sentProgress);
            mLastSentProgress = sentProgress;
        }
        else
        {
            destroyBlock(pos);
        }
    }
    else
    {
        if (isLobby())
        {
            const BlockState* state = mLevel->getBlockState(pos);
            if (state->getMaterial() != Material::AIR)
            {
                boost::shared_ptr<ServerPlayer> player = getServerPlayer();
                state->getBlock()->attack(mLevel, pos, player);
            }
        }
        if (!mLevel->extinguishFire(boost::shared_ptr<Player>(), pos, direction))
            destroyBlock(pos);
    }

    return true;
}
InteractionResult::EInteractionResult ServerPlayerGameMode::useItem(boost::shared_ptr<Player> player, Level* level,
                                                                    InteractionHand::EInteractionHand hand,
                                                                    bool simulate)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        if (!mMinecraft->GetMiniGame()->ArePowerupsActive())
            return InteractionResult::eInteractionResult_Fail;

        PowerupItems::UsePowerupItem(player, Item::getId(item->getItem()));
        item->shrink(1);
        if (item->isEmpty())
        {
            player->setItemInHand(hand, ItemInstance::EMPTY);
            player->mInventory->OnRemove(player->mInventory->m_selected);
        }
        return InteractionResult::eInteractionResult_Success;
    }

    if (!player->isAllowedToUse(item))
        return InteractionResult::eInteractionResult_Pass;

    if (isSpectator() || GetGameType() == GameType::SPECTATOR)
        return InteractionResult::eInteractionResult_Pass;

    int savedCount = item->getCount();
    int savedAux = item->getAuxValue();

    InteractionResultHolder holder = item->use(level, player, hand);
    InteractionResult::EInteractionResult result = holder.getResult();
    not_null_ptr<ItemInstance> resultItem = holder.getObject();

    if (result == InteractionResult::eInteractionResult_Success)
        RecordUseItem(resultItem);

    if (resultItem == item && item->getCount() == savedCount && item->getUseDuration() < 1
        && item->getAuxValue() == savedAux)
        return result;

    if (result == InteractionResult::eInteractionResult_Fail && item->getUseDuration() > 0 && !player->isUsingItem())
        return result;

    player->setItemInHand(hand, resultItem);

    if (isCreative() && !resultItem->getItem()->IsArmor() && !isLobby())
    {
        resultItem->setCount(savedCount);
        if (resultItem->isDamageableItem())
            resultItem->setAuxValue(savedAux);
    }

    if (resultItem->isEmpty())
    {
        player->setItemInHand(hand, ItemInstance::EMPTY);
        player->mInventory->OnRemove(player->mInventory->m_selected);
    }

    if (!player->isUsingItem())
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
        serverPlayer->refreshContainer(serverPlayer->mInventoryMenu);
    }

    return result;
}
void ServerPlayerGameMode::SetGameType(const GameType* gameType)
{
    mGameType = gameType;

    gameType->updatePlayerAbilities(&mPlayer->mAbilities);
    mPlayer->onUpdateAbilities();
    mPlayer->UpdateSize();

    const int playerIndex = mPlayer->mPlayerIndex;
    PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;

    players->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
        ClientboundGameEventPacket::CHANGE_GAME_MODE, static_cast<float>(mGameType->getId()), playerIndex)));

    mLevel->updateSleepingPlayerList();
}
const GameType* ServerPlayerGameMode::GetGameType()
{
    return mGameType;
}
bool ServerPlayerGameMode::IsNearCentreOf(const AABB* box, ServerPlayer* player, double x, double y, double z)
{
    if (player != NULL && box != NULL)
    {
        AABB* scaled = box->scale(x, y, z);
        Vec3* position = Vec3::newTemp(player->mX, player->mY, player->mZ);
        if (scaled->insideXZ(position))
            return true;
    }
    return false;
}
void ServerPlayerGameMode::tick()
{
    ++mGameModeTicks;

    if (!isSpectator() && Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        RecordLayerTime();

    if (mIsDestroying)
    {
        const int elapsedTicks = mGameModeTicks - mSecondaryDestroyStartTick;
        const BlockState* state = mLevel->getBlockState(mPreviousDestroyPos);
        if (state->getMaterial() == Material::AIR)
        {
            mIsDestroying = false;
            return;
        }

        const float fractionalProgress = state->getDestroyProgress(getServerPlayer(), mPlayer->mLevel,
                                                                   mPreviousDestroyPos)
                                         * static_cast<float>(elapsedTicks + 1);
        const int sentProgress = static_cast<int>(fractionalProgress * 10.0f);
        if (sentProgress != mLastSentProgress)
        {
            mLevel->destroyBlockProgress(mPlayer->getId(), mPreviousDestroyPos, sentProgress);
            mLastSentProgress = sentProgress;
        }

        if (fractionalProgress >= 1.0f)
        {
            mIsDestroying = false;
            destroyBlock(mPreviousDestroyPos);
        }
    }
    else if (mDestroying)
    {
        const BlockState* state = mLevel->getBlockState(mDestroyPos);
        if (state->getMaterial() == Material::AIR)
        {
            mLevel->destroyBlockProgress(mPlayer->getId(), mDestroyPos, -1);
            mDestroying = false;
            mLastSentProgress = -1;
            return;
        }

        const int elapsedTicks = mGameModeTicks - mDestroyStartTick;
        const float fractionalProgress = state->getDestroyProgress(getServerPlayer(), mPlayer->mLevel,
                                                                   mPreviousDestroyPos)
                                         * static_cast<float>(elapsedTicks + 1);
        const int sentProgress = static_cast<int>(fractionalProgress * 10.0f);
        if (sentProgress != mLastSentProgress)
        {
            mLevel->destroyBlockProgress(mPlayer->getId(), mDestroyPos, sentProgress);
            mLastSentProgress = sentProgress;
        }
    }
}
void ServerPlayerGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player)
{
    ++mDeaths;
    if (mMasterGameMode != NULL)
        mMasterGameMode->OnDeath(player);
}
unsigned int ServerPlayerGameMode::GetDeaths()
{
    return mDeaths;
}
void ServerPlayerGameMode::SetDeaths(int deaths)
{
    mDeaths = deaths;
}
unsigned int ServerPlayerGameMode::GetKills()
{
    return 0;
}
void ServerPlayerGameMode::SetKills(int)
{
}

void ServerPlayerGameMode::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                                        boost::shared_ptr<ClientboundChatPacket> packet)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordDamage(entity, entry, packet);
}
void ServerPlayerGameMode::RecordDeath(boost::shared_ptr<Entity> entity)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    masterGameMode->mGameStats.RecordDeath(entity);
}
void ServerPlayerGameMode::RecordItem(not_null_ptr<ItemInstance> item)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordItem(getServerPlayer()->getSUID(), item);
}
void ServerPlayerGameMode::RecordContainer(int type, int slot, int count, int data)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    masterGameMode->mGameStats.RecordContainer(player->getSUID(), type, slot, count, data);
}
void ServerPlayerGameMode::RecordWalk(int distance, bool sprinting, bool sneaking)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    masterGameMode->mGameStats.RecordWalk(player->getSUID(), distance, sprinting, sneaking);
}
void ServerPlayerGameMode::RecordSwim(int distance, bool sprinting)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordSwim(getServerPlayer()->getSUID(), distance, sprinting);
}
void ServerPlayerGameMode::RecordClimb(int distance)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordClimb(getServerPlayer()->getSUID(), distance);
}
void ServerPlayerGameMode::RecordUseItem(not_null_ptr<ItemInstance> item)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordUseItem(getServerPlayer()->getSUID(), item);
}
void ServerPlayerGameMode::RecordJump()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordJump(getServerPlayer()->getSUID());
}
void ServerPlayerGameMode::RecordGainPoints(int points)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordGainPoints(getServerPlayer()->getSUID(), points);
}
void ServerPlayerGameMode::ResetTempPoints()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.ResetTempPoints(getServerPlayer()->getSUID());
}
bool ServerPlayerGameMode::RecordSecondaryTargetReached(const std::wstring& name, int target,
                                                        GlideRingGenerator::eGlideRingSize size)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        return masterGameMode->mGameStats.RecordSecondaryTargetReached(getServerPlayer()->getSUID(), name, target,
                                                                       size);
    return false;
}
bool ServerPlayerGameMode::RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
    {
        if (third || masterGameMode->mGameStats.GetLastCheckpointSequenceID(getServerPlayer()->getSUID()) == lap - 1)
        {
            if (second)
                setLastCheckpoint(checkpoint);

            bool result = masterGameMode->mGameStats.RecordLastCheckpoint(getServerPlayer()->getSUID(), checkpoint, lap,
                                                                          first);

            if (result && second && !third)
                mPlayer->sendMessage(ClientboundChatPacket::eChatPacketMessage_Checkpoint, NULL, 0, NULL, 0);

            return result;
        }
    }
    return false;
}
bool ServerPlayerGameMode::RecordLapCompleted()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
    {
        masterGameMode->mGameStats.RecordLapCompleted(getServerPlayer()->getSUID());
        return true;
    }
    return false;
}
void ServerPlayerGameMode::RecordThermalEntered()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordThermalEntered(getServerPlayer()->getSUID());
}
bool ServerPlayerGameMode::RecordProgress(double first, double second, double third, double fourth)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        return masterGameMode->mGameStats.RecordProgress(getServerPlayer()->getSUID(), first, second, third, fourth);
    return false;
}
void ServerPlayerGameMode::RecordSpawnedAtCheckpoint(int checkpoint)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordSpawnedAtCheckpoint(getServerPlayer()->getSUID(), checkpoint);
}
void ServerPlayerGameMode::RecordBlockPlaced(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordBlockPlaced(getServerPlayer()->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordBlockDestroyed(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordBlockDestroyed(getServerPlayer()->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordBlockPresent(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordBlockPresent(getServerPlayer()->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordLayerTime()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (isLobby())
        return;

    const long long elapsedMs = static_cast<long long>(System::processTimeInMilliSecs() - mLayerTimeMs);
    const bool waitedLongEnough = static_cast<unsigned int>(std::floor(static_cast<double>(elapsedMs)) / 1000.0) > 1;

    const int layer = masterGameMode->GetLayer(static_cast<int>(mPlayer->mY));
    if (masterGameMode != NULL && waitedLongEnough && layer != -1)
    {
        const AABB* layerBox = masterGameMode->GetLayerAABB(layer);
        const bool nearCentre = IsNearCentreOf(layerBox, mPlayer, 0.5, 1.0, 0.5);
        mLayerTimeMs = System::processTimeInMilliSecs();
        masterGameMode->mGameStats.RecordTimeOnLayer(getServerPlayer()->getSUID(), layer, nearCentre);
    }
}
void ServerPlayerGameMode::RecordSnowballHit()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordSnowballDirectHit(getServerPlayer()->getSUID());
}
void ServerPlayerGameMode::RecordHitBySnowball()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordHitBySnowball(getServerPlayer()->getSUID());
}
void ServerPlayerGameMode::RecordLanding(const BlockPos& pos)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
    {
        int layer = masterGameMode->GetLayer(pos.y);
        if (layer == gMinecraftServer->mMasterGameMode->GetLayerCount() - 1
            && masterGameMode->IsOnGeneratedLayer(layer, pos.y))
        {
            masterGameMode->mGameStats.RecordLanding(mPlayer->getSUID());
        }
    }
}
const MiniGameDef* ServerPlayerGameMode::GetMiniGame()
{
    return CommonMasterGameMode::GetMiniGame();
}
boost::shared_ptr<Player> ServerPlayerGameMode::GetPlayer()
{
    return boost::static_pointer_cast<Player>(mPlayer->shared_from_this());
}
Level* ServerPlayerGameMode::GetLevel()
{
    return mLevel;
}
